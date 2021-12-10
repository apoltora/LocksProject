/**
 * Microbenchmark 1 
 * Use this command to compile:
 * clang -lpthread -o clh mb1_clh.c
 * Then to run:
 * ./clh
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

#define NUM_THREADS 32
#define CACHE_LINE_SIZE 64 

#define MAX_CRIT_ITERS 1
#define MAX_NON_CRIT_ITERS 1

typedef enum { LOCKED, UNLOCKED } qlock_state;

typedef struct qlock {

    volatile qlock_state state;
    char padding[CACHE_LINE_SIZE - sizeof(qlock_state)]; //padding
    struct qlock* volatile prev;

} qlock_t;

qlock_t* volatile _Atomic glock;

int x;

//function to return current wall clock time in nanosecs
long get_wall_clock_time_nanos()
{
    struct timespec t0;
    long time_in_nano_sec;

    timespec_get(&t0, TIME_UTC);  

    time_in_nano_sec = (((long)t0.tv_sec * 1000000000L) + t0.tv_nsec);

    return time_in_nano_sec; // time_in_nano_seconds
}

qlock_t *AcquireQLock() {

    qlock_t *mlock;
    mlock = (qlock_t *) malloc(sizeof(qlock_t));

    mlock->state = LOCKED;

    qlock_t *prev_glock;

    while(1)
    {
        prev_glock = glock;

        // parameters are destination, expected value, desired value
        if(atomic_compare_exchange_weak(&glock, &prev_glock, mlock))
            break;

    }

    mlock->prev = prev_glock;

    while (mlock->prev->state == LOCKED); // SPIN HERE...

    // free the previous node here as it wont be used anymore.
    free(prev_glock);

    return mlock;

}


void ReleaseQLock(qlock_t *mlock) {

    mlock->state = UNLOCKED;

}


void *operation(void *vargp) {
    // place a start timer here
    qlock_t *mylock;
    int crit_sec_executed = 0;
    int non_crit_sec_executed = 0;
    
    while(non_crit_sec_executed < MAX_NON_CRIT_ITERS || crit_sec_executed < MAX_CRIT_ITERS)
    {
    
        if(crit_sec_executed < MAX_CRIT_ITERS)
        {
            mylock = AcquireQLock();

            /**** CRITICAL SECTION *****/

            // place an end timer here
            //x++;
            long delay = 100000000;
            while(delay)
                delay--;

            /**** END OF CRITICAL SECTION *****/    

            crit_sec_executed++;
            ReleaseQLock(mylock);
        }


        if(non_crit_sec_executed < MAX_NON_CRIT_ITERS)
        {
            /* Start of NON-CRITICAL SECTION */

            // 1 times the delay of critical section //
            long delay = 100000000;
            while(delay)
                delay--;

            /* End of NON-CRITICAL SECTION */
            non_crit_sec_executed++;

        }


    }


    return vargp;
}


int main() {
    x = 0;

    // initialize glock
    glock = NULL;
    qlock_t *glock_init;
    glock_init = (qlock_t *) malloc(sizeof(qlock_t));

    glock_init->state = UNLOCKED;
    glock_init->prev = NULL;

    qlock_t *prev_glock = glock;
    atomic_compare_exchange_weak(&glock, &prev_glock, glock_init);

    long time_init = get_wall_clock_time_nanos();

    pthread_t threads[NUM_THREADS];
    int i, j;

    for (i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, operation, NULL);    // make the threads run the operation function
    }

    for (j = 0; j < NUM_THREADS; j++) {
        pthread_join(threads[j], NULL);                      // waits for all threads to be finished before function returns
    }

    long time_final= get_wall_clock_time_nanos();

    long time_diff = time_final - time_init;
    
    printf("The value of x is : %d\n", x);
    printf("Total RUNTIME : %lf\n\n", ((double) time_diff/1000000000));

    // free the final tail node of glock
    free(glock);
    return 0;

}

