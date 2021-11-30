/**
 * This microbenchmark is a very simple add operation to a shared variable
 * Creating MCS Time-Published lock (variant of MCS queue lock)
 * 
 * This lock has two additional states LEFT and REMOVED when compared to *original MCS Queue lock.
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
#define TIMEOUT 200 // in microseconds

// If the current system time is more than the thread's latest published time by this value then we can assume the thread has been preempted.
#define PREEMPTION_THRESHOLD_TIME 10 // in microseconds


typedef enum { AVAILABLE, WAITING, LEFT, REMOVED } qnode_state;


// A structure to represent a particular queue node in the queue lock
typedef struct q_node {

    volatile qnode_state state;
    char padding[CACHE_LINE_SIZE - sizeof(qnode_state)]; //padding
    volatile double publish_time;
    char padding[CACHE_LINE_SIZE - sizeof(double)]; //padding
    struct q_node* volatile next;

} qnode_t;


// A structure to represent the global lock node
typedef struct q_lock {

    qnode_t* volatile _Atomic glock;
    volatile double crit_sec_start_time;

} q_lock_t;


// global lock variable
q_lock_t lock;


int x;

qnode_t *AcquireQLock() {

    qnode_t *mlock;
    mlock = (qnode_t *) malloc(sizeof(qnode_t));

    mlock->next = NULL;
    mlock->state = AVAILABLE;

    qnode_t *prev_glock;
    qnode_t *prev_glock_temp;
    unsigned long long temp;

    while(1)
    {
        prev_glock = lock.glock;

        // parameters are destination, expected value, desired value
        if(atomic_compare_exchange_weak(&(lock.glock), &prev_glock, mlock))
            break;

    }

    // no thread in the queue lock yet.
    if (prev_glock == NULL)
    {
        return mlock;
    }

    mlock->state = WAITING;
    prev_glock->next = mlock;
    printf("I am here\n");

    while (mlock->state == WAITING); // SPIN HERE...

    return mlock;

}


void ReleaseQLock(qnode_t *mlock) {
    do {
        // last element condition
        if (mlock->next == NULL) {
            
            qnode_t *prev_glock_temp;
            long temp;

            qnode_t *mlock_temp = mlock;

            // parameters are destination, expected value, desired value
            if(atomic_compare_exchange_weak(&(lock.glock), &mlock_temp, NULL))
            {
                free(mlock);
                return;
            }

        }
        else {
            mlock->next->state = AVAILABLE;
            free(mlock);
            return;
        }
    } while(1);
}


void *operation(void *vargp) {
    // place a start timer here
    qnode_t *mylock;
   
    mylock = AcquireQLock();

    // place an end timer here
    x++;

   // long delay = 1000000000;
    //while(delay)
      //  delay--;

    ReleaseQLock(mylock);

    // place an end timer here
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


