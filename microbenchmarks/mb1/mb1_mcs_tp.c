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

#define NUM_THREADS 8
#define CACHE_LINE_SIZE 64 

// How long should a thread keep spinning while waiting in the queue ? 
// Also known as PATIENCE INTERVAL...
// After this timeout has reached the thread can perform a yield or execute an alternate execution path
#define PATIENCE 200 // in microseconds

// maximum time taken to execute the critical section...lock holder is preempted if it is holding the lock more than this time
// yield now to help the lock holder to get rescheduled
// measure the time of critical section and then use that time here
#define MAX_CS_TIME 2000 // in microseconds



// Lock Holder checks this value while handing the lock to other thread
// If the current system time is more than the (thread's latest published time + UPD_DELAY) by this value then we can assume the thread has been preempted.
// no need to handover the lock to this thread in that case

// preemption condition: 
// current_sys_time > ((published_time + UPD_DELAY) + PREEMPTION_TOLERANCE)
// This value should be very small...
// this is just to give some tolerence to this condition.....
// just to find that published time has really become stale or not...
#define PREEMPTION_TOLERANCE 1 // in microseconds

// the approx time it takes for a thread to see a timestamp from another thread
#define UPD_DELAY 10 // in microseconds


typedef enum { AVAILABLE, WAITING, TIMED_OUT, REMOVED } qnode_state;


// A structure to represent a particular queue node in the queue lock
typedef struct q_node {

    volatile qnode_state _Atomic state;
    char padding[CACHE_LINE_SIZE - sizeof(qnode_state)]; //padding
    volatile double published_time;
    char padding[CACHE_LINE_SIZE - sizeof(double)]; //padding
    struct q_node* volatile next;

    //not sure whether we need this
    q_lock_t* last_lock;

} qnode_t;


// A structure to represent the global lock node
typedef struct q_lock {

    // start time of the critical section
    volatile double crit_sec_start_time;
    char padding[CACHE_LINE_SIZE - sizeof(double)]; //padding
    qnode_t* volatile _Atomic glock;

} q_lock_t;


// global lock variable
q_lock_t lock;


int x;

//function to return current time
double get_time_func()
{


}


int AcquireQLock(qnode_t *mlock) {

    double acquire_start_time = get_time_func();

    qnode_state temp_state = TIMED_OUT;
    // If current status is timeout...then make it to waiting
    if(atomic_compare_exchange_weak(&(mlock->state), &temp_state, WAITING))
    { 
        // atomic state swap success....rejoined the queue
    }
    else // start a new try
    {
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
            if(get_time_func() > (lock.crit_sec_start_time + MAX_CS_TIME))
                sched_yield(2);

            //mlock->last_lock = lock.glock;   

            // lock acquire failed because the thread was preempted while the lock head tried to handover the lock 
            return -1;
        }

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
                // breaks from this immediate while loop and loops in outer while loop
                break;
            }

            /* State has already been changed to TIMED_OUT at this point by the previous atomic_compare_exchange_weak */

            // check whether lock holder is preempted... perform a yield...
            // help lock holder to make progress
            if(get_time_func() > (lock.crit_sec_start_time + MAX_CS_TIME))
                sched_yield(2);

            // lock acquire failed because the thread TIMED_OUT
            return -2;

        }

    }
    
}


void ReleaseQLock(qnode_t *mlock) {

    qnode_t *curr_node;
    qnode_t *next_node;
    qnode_t *scanned_qhead = NULL;
    int scanned_nodes = 0;

    curr_node = mlock;

    while(1)
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
                curr_node->state = REMOVED;
                return;
            }

            /* The atomic_compare_exchange_weak above failed...so a new successor got enqueued...get that successor and leave this while loop */
            while (next_node == NULL)
            {
                next_node = curr_node->next;   
            }

        }

        if(++scanned_nodes < NUM_THREADS)
            curr_node->state = REMOVED;
        else if(scanned_qhead == NULL)
            scanned_qhead = curr_node;    


        if(next_node->state == WAITING)
        {
            double next_node_published_time = next_node->published_time;

            qnode_state temp_state = WAITING;

            // check if successor is not preempted... if so handover the lock...

            if((get_time_func() > ((next_node_published_time + UPD_DELAY) + PREEMPTION_TOLERANCE)) && (atomic_compare_exchange_weak(&(next_node->state), &temp_state, AVAILABLE)))
            {

                if(scanned_qhead != NULL)
                {
                    while(scanned_qhead != NULL && scanned_qhead != curr_node)
                    {
                        //free(scanned_qhead);
                        scanned_qhead->state = REMOVED;
                        scanned_qhead = scanned_qhead->next;
                    }

                }

                return;

            }

        }

        curr_node = next_node;

    }

}


void *operation(void *vargp) {

    qnode_t *mylock;
    mylock = (qnode_t *) calloc(sizeof(qnode_t));

    // initialize first time
    mylock->state = WAITING;
   
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
            mylock = (qnode_t *) calloc(sizeof(qnode_t));
            mylock->state = WAITING;

        }

        else if(ret_val == -2)
        {
            // node TIMED_OUT...
            // retry acquire and try to rejoin the queue
            // retry is done below.... do nothing here.
        }

        else // returned 1... lock acquired
            break;

        // retry acquire...
        ret_val = AcquireQLock(mylock);
    }


    /* Start of CRITICAL SECTION */


    x++;

   // long delay = 1000000000;
    //while(delay)
      //  delay--;



    /* End of CRITICAL SECTION */


    ReleaseQLock(mylock);

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

    printf("The value of x is : %d\n", x);
    return 0;

}


