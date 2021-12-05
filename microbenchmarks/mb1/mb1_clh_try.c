/**
 * This microbenchmark is a very simple add operation to a shared variable
 * Creating clh_try queue lock
 * 
 * 
 * Use this command to compile:
 * clang -std=c11 -lpthread -o clh_try mb1_clh_try.c
 * Then to run:
 * ./clh_try
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
#include <sched.h>

#define NUM_THREADS 8
#define CACHE_LINE_SIZE 64 

// Timeout threshold // TODO: tune this value
#define T 0.0005 // 500 microsecs  


typedef enum {waiting,      // lock is held
            available,    // lock is free
            leaving,      // node owner is giving up
            transient,    // successor is giving up
            recycled}     // no pointers to node remain
    qlock_state;


typedef struct qlock {
    volatile qlock_state _Atomic status;
    char padding[CACHE_LINE_SIZE - sizeof(qlock_state)]; //padding
    struct qlock* volatile prev;
} qlock_t;


// typedef volatile clh_qnode *clh_qnode_ptr;
// typedef clh_qnode_ptr clh_try_lock;

qlock_t* volatile _Atomic glock;

int x;


// TODO: complete this
double get_time_func()
{


}


bool AcquireQLock(qlock_t *mlock) 
{
    double start_time;
    qlock_state stat;

    mlock->status = waiting;
    mlock->prev = NULL;

    // predecessor in the queue
    qlock_t *pred; 

    while(1)
    {
        pred = glock;
        // parameters are destination, expected value, desired value
        if(atomic_compare_exchange_weak(&glock, &pred, mlock))
            break;
    }

    // As an optimization, check the lock once before querying timer
    if (pred->status == available) {
        mlock->prev = NULL;
        free(pred);
        //mlock->prev = pred;
        // lock acquired
        return true;
    }
    
    start_time = get_time_func();       

    // execute this while loop until the node times out
    while (get_time_func() - start_time < T) {
        stat = pred->status;
        if (stat == available) {
            mlock->prev = NULL;
            free(pred);
            //mlock->prev = pred;
            // lock acquired
            return true;
        }
        if (stat == leaving) {
            qlock_t *temp = pred->prev;
            pred->status = recycled;
            pred = temp;
        }
        // stat can also be transient,
        // if some node has left the queue just before my node entered
    }


    // my node timed out at this point

    while (1) {

        /* there is possiblility that pred's status can be transient while entering this while loop as some other node might be in the process 
        of leaving the queue. 
        if so just spin until state changes to waiting again */
        while (pred->status == transient); 

        while(1)
        {
            stat = pred->status;
            // parameters are destination, expected value, desired value
            if(atomic_compare_exchange_weak(&pred->status, &stat, transient))
                break;
        }


        if (stat == available) {
            mlock->prev = NULL;
            free(pred);
            //mlock->prev = pred;
            // lock acquired
            return true;
        }

        // if pred is leaving help pred to leave...
        if (stat == leaving) {
            qlock_t *temp = pred->prev;
            pred->status = recycled;
            pred = temp;
            continue;
        }
        break; // stat == waiting
    }

    /* updating prev pointer (as pred might have changed by now) so that successor can find it */
    mlock->prev = pred; 

    // indicate leaving, so successor can't leave
    while (1) {
        stat = waiting;
        atomic_compare_exchange_weak(&mlock->status, &stat, leaving);

        //swap success
        if (stat == waiting) 
            break; 

        // spin until the status becomes waiting again
        while (mlock->status != waiting);
    }


    /******* START LEAVING FROM THE QUEUE *********/

    /* special case: if last thread in the queue, swap the lock tail with pred and leave */

    qlock_t *mlock_temp = mlock;

    if(atomic_compare_exchange_weak(&glock, &mlock_temp, pred)){
        pred->status = waiting;

        /* no need to free mlock here as we are anyways going to do retry with mlock after sometime */
        return false;
    }

    // node is not the last in the queue
    while (1) {
        stat = mlock->status;
        if (stat == recycled) {
            pred->status = waiting;

            /* no need to free mlock here as we are anyways going to do retry with mlock after sometime */
            return false;
        }
    }
}



void ReleaseQLock(qlock_t *mlock)
{
    qlock_state temp_state = waiting;
    // If status is waiting change it to available (atomically)
 while (!atomic_compare_exchange_weak(&mlock->status, &temp_state, available)) {
        /* can't set my node to available if it's currently transient, so start spinning */
        while (mlock->status == transient);  
        temp_state = waiting;
    }
}



void *operation(void *vargp) {
    // place a start timer here
    qlock_t *mylock;
    mylock = (qlock_t *) malloc(sizeof(qlock_t));

    bool acquire_status = true;
   
    while(1)
    {
        acquire_status = AcquireQLock(mylock);

        if(acquire_status) // lock acquire success
            break;
        else
        {
            // as the lock timed out, yield now and continue later
            sched_yield();

          //? maybe we should wait for sometime here before a retry ?
        }


    }

    /**** CRITICAL SECTION *****/

    // place an end timer here
    x++;
    long delay = 1000000000;
    while(delay)
        delay--;

    ReleaseQLock(mylock);

    // place an end timer here
    return vargp;
}



int main() {
    x = 0;

    // initialize glock
    glock = NULL;
    qlock_t *glock_init;
    glock_init = (qlock_t *) malloc(sizeof(qlock_t));

    glock_init->status = available;
    glock_init->prev = NULL;

    qlock_t *prev_glock = glock;
    atomic_compare_exchange_weak(&glock, &prev_glock, glock_init);

    // Create and spawn threads
    pthread_t threads[NUM_THREADS];
    int i, j;

    for (i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, operation, NULL);    // make the threads run the operation function
    }

    for (j = 0; j < NUM_THREADS; j++) {
        pthread_join(threads[j], NULL);                      // waits for all threads to be finished before function returns
    }

    printf("The value of x is : %d\n", x);

    // free the final tail node of glock
    free(glock);
    return 0;

}

