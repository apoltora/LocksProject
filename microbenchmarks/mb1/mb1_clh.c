/**
 * This microbenchmark is a very simple add operation to a shared variable
 * Creating clh queue lock
 * Check out visual on how this lock works here: https://classes.engineering.wustl.edu/cse539/web/lectures/locks.pdf
 * 
 * Use this command to compile:
 * clang -std=c11 -lpthread -o clh mb1_clh.c
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

#define NUM_THREADS 8
#define CACHE_LINE_SIZE 64 

typedef enum { LOCKED, UNLOCKED } qlock_state;

typedef struct qlock {

    volatile qlock_state state;
    char padding[CACHE_LINE_SIZE - sizeof(qlock_state)]; //padding
    struct qlock* volatile prev;

} qlock_t;

qlock_t* volatile _Atomic glock;

int x;

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

    // printf("I am here...\n");

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

    glock_init->state = UNLOCKED;
    glock_init->prev = NULL;

    qlock_t *prev_glock = glock;
    atomic_compare_exchange_weak(&glock, &prev_glock, glock_init);



    pthread_t threads[NUM_THREADS];
    int i, j;

    for (i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, operation, NULL);    // make the threads run the operation function
    }

    for (j = 0; j < NUM_THREADS; j++) {
        pthread_join(threads[j], NULL);                      // waits for all threads to be finished before function returns
    }

    printf("The value of x is : %d\n", x);

    // free the tail node of glock
    free(glock);
    return 0;

}

