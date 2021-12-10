/**
 * CLH test 5
 * 
 * Use this command to compile:
 * clang -lpthread -o clh mb3_clh.c
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

#define ARRAY_SIZE 10000000

volatile long x[ARRAY_SIZE];

void initialize_array(volatile long *array) {
    int i;
    for (i = 0; i < ARRAY_SIZE; i++) {
        array[i] = 0;
    }
}


typedef enum { LOCKED, UNLOCKED } qlock_state;

typedef struct qlock {

    volatile qlock_state state;
    char padding[CACHE_LINE_SIZE - sizeof(qlock_state)]; //padding
    struct qlock* volatile prev;

} qlock_t;

qlock_t* volatile _Atomic glock;


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

    mylock = AcquireQLock();

    /**** CRITICAL SECTION *****/

    // place an end timer here
      long delay = 100000000;
        while(delay)
            delay--;

    /**** END OF CRITICAL SECTION *****/    

    ReleaseQLock(mylock);

    return vargp;
}




int main() {

    // initialize glock
    glock = NULL;
    qlock_t *glock_init;
    glock_init = (qlock_t *) malloc(sizeof(qlock_t));

    glock_init->state = UNLOCKED;
    glock_init->prev = NULL;

    qlock_t *prev_glock = glock;
    atomic_compare_exchange_weak(&glock, &prev_glock, glock_init);

    //init array
    initialize_array(x);

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
    
    printf("Total RUNTIME : %lf\n\n", ((double) time_diff/1000000000));

    // free the final tail node of glock
    free(glock);
    return 0;

}


