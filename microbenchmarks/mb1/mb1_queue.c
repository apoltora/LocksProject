/**
 * This microbenchmark is a very simple add operation to a shared variable
 * Creating queue lock
 * 
 * Use this command to compile:
 * gcc -o ---- -lpthread ------
 * Then to run:
 * ./-----
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

#define NUM_THREADS 8

typedef struct lock {
    int next_ticket;
    int now_serving;
} lock_t;

extern void atomic_increment(int *x);
volatile lock_t l;

int x;



AcquireQLock(*glock, *mlock) {
    mlock->next = NULL;
    mlock->state = UNLOCKED;
    ATOMIC();
    prev = glock
    *glock = mlock
    END_ATOMIC();
    if (prev == NULL) return;
    mlock->state = LOCKED;
    prev->next = mlock;
    while (mlock->state == LOCKED); // SPIN
}

ReleaseQLock(*glock, *mlock) {
    do {
        if (mlock->next) {
            x = CMPXCHG(glock, mlock, NULL);
            if (x == mlock) return;
        }
        else {
            mlock->next->state = UNLOCKED;
            return;
        }
    } while(1);
}


void *operation(void *vargp) {
    // place a start timer here
    Lock(&l);
    // place an end timer here
    x++;
    unlock(&l);
    // place an end timer here
    return vargp;
}


int main() {
    x = 0;
    pthread_t threads[NUM_THREADS];
    void *tmp_result;
    int i, j;


    for (i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, operation, NULL);    // make the threads run the operation function
    }
    for (j = 0; j < NUM_THREADS; j++) {
        pthread_join(threads[j], NULL);                      // waits for all threads to be finished before function returns
    }
    return x;
}
