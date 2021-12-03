/**
 * This microbenchmark is a very simple add operation to a shared variable
 * Creating MCS Time-Published lock (variant of MCS queue lock)
 * 
 * This lock has two additional states TIMED_OUT and REMOVED when compared to *original MCS Queue lock.
 * 
 * Use this command to compile:
 * clang -std=c11 -lpthread -o mcs_tp mb1_mcs_tp.c
 * Then to run:
 * ./mcs_tp
 * 
 * Authors: Alexandra Poltorak, Kiran Kumar Rajan Babu
 * Contact: apoltora@andrew.cmu.edu, krajanba@andrew.cmu.edu
 *
 */

#include <pthread.h>
#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdatomic.h>
#include <time.h>

#define NUM_THREADS 20

// Limit on the number of nodes the lock holder scan before releasing the lock 
#define MAX_NODES_TO_SCAN 20

#define CACHE_LINE_SIZE 64 

// How long should a thread keep spinning while waiting in the queue ? 
// Also known as PATIENCE INTERVAL...
// After this timeout has reached the thread can perform a yield or execute an alternate execution path... In our code we perform yield...
#define PATIENCE 5 // in seconds // time of 4 critical sections

// maximum time taken to execute the critical section...lock holder is preempted if it is holding the lock more than this time
// yield now to help the lock holder to get rescheduled
// measure the time of critical section and then use that time here
#define MAX_CS_TIME 1.25 // in seconds



// Lock Holder checks this value while handing the lock to other thread
// If the current system time is more than the (thread's latest published time + UPD_DELAY) by this value then we can assume the thread has been preempted.
// no need to handover the lock to this thread in that case

// the approx time it takes for a thread to see a timestamp from another thread
// keep this value slightly more than the exact upd_delay to provide some tolerance and to avoid false-positive in detecting preempted threads
#define UPD_DELAY 0.0001 // 100 microsecs // 0.00005 - 50 microseconds


// useful performance counters for observations...

volatile int preempted_perf_counter = 0;

volatile int not_preempted_lock_release_success = 0;

volatile int release_by_last_node_in_queue = 0;

volatile int force_lock_release_second_while_loop = 0;

volatile int timed_out_nodes_removed_by_lock_holder = 0;

volatile int timeout_yield_count = 0;

volatile int lock_holder_progress_yield_count = 0;

volatile int total_timeout_occurrences = 0;

volatile int qnodes_rejoined_after_timeout_success = 0;



typedef enum { AVAILABLE, WAITING, TIMED_OUT, REMOVED } qnode_state;


// A structure to represent a particular queue node in the queue lock
typedef struct q_node {

    volatile qnode_state _Atomic state;
    char padding1[CACHE_LINE_SIZE - sizeof(qnode_state)]; //padding
    volatile double published_time;
    char padding2[CACHE_LINE_SIZE - sizeof(double)]; //padding
    struct q_node* volatile next;

} qnode_t;


// A structure to represent the global lock node
typedef struct q_lock {

    // start time of the critical section
    volatile double crit_sec_start_time;
    char padding3[CACHE_LINE_SIZE - sizeof(double)]; //padding
    qnode_t* volatile _Atomic glock;

} q_lock_t;


// global lock variable
q_lock_t lock;


int x;

//function to return current time
double get_time_func()
{
    struct timespec t0;
    double time_in_sec;

    if(timespec_get(&t0, TIME_UTC) != TIME_UTC) {
        printf("Error in calling timespec_get\n");
        exit(EXIT_FAILURE);
    }

  //  time_in_sec = ((double) t0.tv_sec);
time_in_sec = ( ((double) t0.tv_sec) + ( ((double) t0.tv_nsec)/1000000000L ));
    
    return time_in_sec; // time_in_seconds
}


int AcquireQLock(qnode_t *mlock) {

    double acquire_start_time = get_time_func();

    qnode_state temp_state = TIMED_OUT;
    // If current status is timeout...then make it to waiting
    // possible states are TIMED_OUT or REMOVED
    if(atomic_compare_exchange_weak(&(mlock->state), &temp_state, WAITING))
    { 
        // atomic state swap success....rejoined the queue
        qnodes_rejoined_after_timeout_success++;
    }
    else // state is REMOVED or starting first try here // start a new try
    {
        // no need atomic modification here... 
        // you are the only one changing it here
        mlock->next = NULL;
        mlock->state = WAITING;

        qnode_t *prev_glock;

        // add mlock to the tail of global_lock
        while(1)
        {
            prev_glock = lock.glock;

            // parameters are destination, expected value, desired value
            if(atomic_compare_exchange_weak(&(lock.glock), &prev_glock, mlock))
                break;

        }

        // no thread in the queue lock yet...lock acquired...
        if (prev_glock == NULL)
        {
            lock.crit_sec_start_time = get_time_func();
            return 1;
        }
        else
        {
            prev_glock->next = mlock;
        }

    }


    while(1)
    {   
        // waiting in the queue
        while(mlock->state == WAITING)
        {
            // publish your time...to indicate that the thread is not preempted.
            mlock->published_time = get_time_func();

            // keep looping in this while loop wait until patience time
            if(!(get_time_func() > (acquire_start_time + PATIENCE)))
                continue;


            /** END OF PATIENCE **/

            temp_state = WAITING;
            if(!atomic_compare_exchange_weak(&(mlock->state), &temp_state, TIMED_OUT))
            {
                // mlock state has been changed by lock holder
                // breaks from this immediate while loop and loop in the outer while loop
                break;
            }

            /* State has already been changed to TIMED_OUT at this point by the previous atomic_compare_exchange_weak */

            total_timeout_occurrences++;
            timeout_yield_count++;

            // As we have timed out... perform a yield...
            sched_yield();
            

            // lock acquire failed because the thread TIMED_OUT
            return -2;

        }


        // lock holder released and handed the lock to the waiting node
        if(mlock->state == AVAILABLE)
        {
            //lock acquired
            lock.crit_sec_start_time = get_time_func();
            return 1;
        }


        // lock holder has removed the node
        else if(mlock->state == REMOVED)
        {
            // check whether lock holder is preempted... perform a yield...
            // help lock holder to make progress
              //  printf("I am in REMOVED \n");
            //    assert(0);

            if(get_time_func() > (lock.crit_sec_start_time + MAX_CS_TIME))
            {
                lock_holder_progress_yield_count++;
                sched_yield();
            }
            //mlock->last_lock = lock.glock;   

            // lock acquire failed because the thread was preempted/timed_out when the lock head tried to handover the lock 
            return -1;
        }

    }
    
}


void ReleaseQLock(qnode_t *mlock) {

    qnode_t *curr_node;
    qnode_t *next_node;
    qnode_t *scanned_qhead = NULL;
    int scanned_nodes = 0;
    int flag = 1;

    curr_node = mlock;
    curr_node->state = REMOVED; // giving up the AVAILABLE state

    qnode_state temp_state;

    /* we need to bound the number of runs of this outer while loop to ensure this thread completes release */
    while(scanned_nodes < MAX_NODES_TO_SCAN)
    {

        next_node = curr_node->next;

        // no successor... last in the queue... so make the global lock as NULL
        if(next_node == NULL)
        {

            qnode_t *curr_node_temp = curr_node;

            // parameters are destination, expected value, desired value
            if(atomic_compare_exchange_weak(&(lock.glock), &curr_node_temp, NULL))
            {
                //free(curr_node);
               // curr_node->state = REMOVED; 
               // we have moved this before while loop
                release_by_last_node_in_queue++;
                return;
            }

            /* The atomic_compare_exchange_weak above failed...so a new successor got enqueued...get that successor and leave this while loop */
            while (next_node == NULL)
            {
                next_node = curr_node->next;   
            }

        }


        /* no node is in the queue after the next_node...so break and just handover the lock to the next_node */
        if(next_node->next == NULL)
        {   
            flag = 0;
            break;
        }

        /* If there are nodes after next_node then execute the below code */
        if(next_node->state == WAITING)
        {
            temp_state = WAITING;
            double next_node_published_time;
            next_node_published_time = next_node->published_time;

            // check if successor is not preempted... if so handover the lock...

            if((get_time_func() <= (next_node_published_time + UPD_DELAY)))
            {

                if(atomic_compare_exchange_weak(&(next_node->state), &temp_state, AVAILABLE))
                {
                    // lock transfer successful to next node
                    not_preempted_lock_release_success++;
                    return;
                }

                // atomic_compare_exchange_weak failed.. Thread has TIMED_OUT 
                //as a lock holder's responsibility, change the state to REMOVED
                else
                {   
                    /* possible state here is TIMED_OUT..or sometimes WAITING if the thread immediately rejoins the queue after doing a immediate retry (after it changed to timeout) */            
                    next_node->state = REMOVED;
                    timed_out_nodes_removed_by_lock_holder++;
                }

            }
            else
            {
                // Published timestamp is stale.... possibly Thread is preempted.// Lock holder changes state to REMOVED
                // possible states here are WAITING and TIMED_OUT
                next_node->state = REMOVED;
                preempted_perf_counter++;

            }

        }

        else if(next_node->state == TIMED_OUT)
        {
            // possible states here are WAITING and TIMED_OUT
            //  Lock holder changes state to REMOVED
            next_node->state = REMOVED;
            timed_out_nodes_removed_by_lock_holder++;
        }

        curr_node = next_node;
        ++scanned_nodes;

    }

    /* jumped out of while loop due to loop condition... then we dont have the correct next_node... so find the next_node first...*/
    if(flag == 1)
    {   
        next_node = curr_node->next;
    
        // no successor... last in the queue... so make the global lock as NULL
        if(next_node == NULL)
        {

            qnode_t *curr_node_temp = curr_node;

            // parameters are destination, expected value, desired value
            if(atomic_compare_exchange_weak(&(lock.glock), &curr_node_temp, NULL))
            {
                //free(curr_node);
               // curr_node->state = REMOVED; 
               // we have moved this before while loop
                release_by_last_node_in_queue++;
                return;
            }

            /* The atomic_compare_exchange_weak above failed...so a new successor got enqueued...get that successor and leave this while loop */
            while (next_node == NULL)
            {
                next_node = curr_node->next;   
            }

        }

    }

    /* 
    Possible states here are TIMED_OUT or WAITING...

    This is a corner case required to bound the number of steps the lock header traverses before handing over the lock...

    This is also needed when there is no other node after the next_node...
    */
    while(1)
    {
        temp_state = WAITING;

        // parameters are destination, expected value, desired value
        // WAIT until the state is WAITING and then change it to AVAILABLE
        if(atomic_compare_exchange_weak(&(next_node->state), &temp_state, AVAILABLE))
        {
            force_lock_release_second_while_loop++;
            break;
        }
    }


}


void *operation(void *vargp) {
    qnode_t *mylock;
    mylock = (qnode_t *) calloc(1, sizeof(qnode_t));

    // initialize first time
    mylock->state = WAITING;
    mylock->next = NULL;
   
    int ret_val;

    ret_val = AcquireQLock(mylock);

    while(1)
    {
        if(ret_val == -1)
        {
            // TODO: try removing free and calloc from here and make sure they dont break correctness

            // reclaim the node
            free(mylock);

            // allocate new space for new mylock
            mylock = (qnode_t *) calloc(1, sizeof(qnode_t));
            mylock->state = WAITING;
            mylock->next = NULL;

           // printf(" preemption retry.....ret_val == -1 \n");

        }

        else if(ret_val == -2)
        {
            // node TIMED_OUT...
            // retry acquire and try to rejoin the queue
            // retry is done below.... do nothing here.
          // printf(" timeout retry....ret_val == -2 \n");

            //some delay before doing a retry 
            /*long delay = 1000000000;
            while(delay)
                delay--;*/
        }

        else // returned 1... lock acquired...
            break;

        // retry acquire...
        ret_val = AcquireQLock(mylock);
    }


    /* Start of CRITICAL SECTION */

   // printf("Lock obtained \n");

    //double time1 = get_time_func();
    x++;

    long delay = 1000000000;
    while(delay)
        delay--;

    //double time2 = get_time_func();

    //double time_diff = time2-time1;

 //printf("Time1 %lf \n", time1);
   // printf("Time2 %lf \n", time2);

   // printf("Time diff %lf \n", time_diff);

    /* End of CRITICAL SECTION */


    ReleaseQLock(mylock);

    // free the mylock as the job is over here...
    free(mylock);

    return vargp;
}


int main() {
    x = 0;

    // initialize global lock node
    lock.glock = NULL;
    lock.crit_sec_start_time = 0;

    pthread_t threads[NUM_THREADS];
    int i, j;

    for (i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, operation, NULL);    // make the threads run the operation function
    }

    for (j = 0; j < NUM_THREADS; j++) {
        pthread_join(threads[j], NULL);                      // waits for all threads to be finished before function returns
    }

   // printf("The value of x is : %d\n", x);
    printf("Predicted Preemptions : %d\n\n", preempted_perf_counter);

    printf("not_preempted_lock_release_success : %d\n\n", not_preempted_lock_release_success);

    printf("lock_release_by_last_node_in_queue : %d\n\n", release_by_last_node_in_queue);

    printf("forced_lock_release_second_while_loop : %d\n\n", force_lock_release_second_while_loop);

    printf("timed_out_nodes_removed_by_lock_holder : %d\n\n", timed_out_nodes_removed_by_lock_holder);

    printf("Total timeouts occurred: %d\n\n", total_timeout_occurrences);

    printf("sched_yield calls due to timeouts : %d\n\n", timeout_yield_count);

    printf("qnodes_rejoined_after_timeout_success: %d\n\n", qnodes_rejoined_after_timeout_success);

    printf("sched_yield calls to help preempted lock holder to progress : %d\n\n", lock_holder_progress_yield_count);

    return 0;

}


