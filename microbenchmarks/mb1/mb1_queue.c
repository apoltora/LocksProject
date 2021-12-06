/**
 * This microbenchmark is a very simple add operation to a shared variable
 * Creating queue lock
 * 
 * Use this command to compile:
 * clang -std=c11 -lpthread -o queue mb1_queue.c
 * Then to run:
 * ./queue
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

#define NUM_THREADS 25
#define CACHE_LINE_SIZE 64 

typedef enum { LOCKED, UNLOCKED } qlock_state;

typedef struct qlock {

    volatile qlock_state state;
    char padding[CACHE_LINE_SIZE - sizeof(qlock_state)]; //padding
    struct qlock* volatile next;

} qlock_t;

qlock_t* volatile _Atomic glock;

int x;


//function to return current wall clock time in nanosecs
long get_wall_clock_time_nanos()
{
    struct timespec t0;
    long time_in_nano_sec;

   /* if(timespec_get(&t0, TIME_UTC) != TIME_UTC) {
        printf("Error in calling timespec_get\n");
        exit(EXIT_FAILURE);
    }*/

    timespec_get(&t0, TIME_UTC);  

    time_in_nano_sec = (((long)t0.tv_sec * 1000000000L) + t0.tv_nsec);

    return time_in_nano_sec; // time_in_nano_seconds
}


qlock_t *AcquireQLock() {

    qlock_t *mlock;
    mlock = (qlock_t *) malloc(sizeof(qlock_t));

    mlock->next = NULL;
    mlock->state = UNLOCKED;

    qlock_t *prev_glock;
    qlock_t *prev_glock_temp;
    unsigned long long temp;

    while(1)
    {
        prev_glock = glock;

        // parameters are destination, expected value, desired value
        if(atomic_compare_exchange_weak(&glock, &prev_glock, mlock))
            break;

    }

    // no thread in the queue lock yet.
    if (prev_glock == NULL)
    {
        return mlock;
    }

    mlock->state = LOCKED;
    prev_glock->next = mlock;

    while (mlock->state == LOCKED); // SPIN HERE...
    //printf("I am here\n");

    return mlock;

}


void ReleaseQLock(qlock_t *mlock) {
    do {
        // last element condition
        if (mlock->next == NULL) {
            
            qlock_t *prev_glock_temp;
            long temp;

            qlock_t *mlock_temp = mlock;

            // parameters are destination, expected value, desired value
            if(atomic_compare_exchange_weak(&glock, &mlock_temp, NULL))
            {
                free(mlock);
                return;
            }

        }
        else {
            mlock->next->state = UNLOCKED;
            free(mlock);
            return;
        }
    } while(1);
}


void *operation(void *vargp) {

    qlock_t *mylock;
   
    mylock = AcquireQLock();

    /* Start of CRITICAL SECTION */
    x++;

    long delay = 100000000;
    while(delay)
        delay--;
    

    /* End of CRITICAL SECTION */

    ReleaseQLock(mylock);


    /* Start of NON-CRITICAL SECTION */

    // 10 times the delay of critical section //
    delay = 1000000000;
    while(delay)
        delay--;

    /* End of NON-CRITICAL SECTION */    

    return vargp;
}


int main() {
    x = 0;

    // initialize glock
    glock = NULL;

    pthread_t threads[NUM_THREADS];
    int i, j;

    long time_init = get_wall_clock_time_nanos();

    for (i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, operation, NULL);    // make the threads run the operation function
    }

    for (j = 0; j < NUM_THREADS; j++) {
        pthread_join(threads[j], NULL);                      // waits for all threads to be finished before function returns
    }

    long time_final= get_wall_clock_time_nanos();

    long time_diff = time_final - time_init;


    printf("********* RUNTIME STATS *********\n\n");

    printf("The value of x is : %d\n", x);

    printf("Total RUNTIME : %lf\n\n", ((double) time_diff/1000000000));
    
    return 0;

}

