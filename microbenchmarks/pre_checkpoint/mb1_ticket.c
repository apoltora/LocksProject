/**
 * This microbenchmark is a very simple add operation to a shared variable
 * Using atomic increment to implement ticket lock
 * 
 * Use this command to compile:
 * gcc -o ticket -lpthread mb1_ticket.c mb1_atomic.s
 * Then to run:
 * ./ticket
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
    volatile int next_ticket;
    volatile int now_serving;
} lock_t;

extern int atomic_increment(volatile int *ticket);

lock_t l;

volatile int x;

void Lock(lock_t *l) {
    int my_ticket = atomic_increment(&l->next_ticket);
    while (my_ticket != l->now_serving);
}

void unlock(lock_t *l) {
    l->now_serving++;
}


void *operation(void *vargp) {
    // place a start timer here
    Lock(&l);
    // place an end timer here

    x++;
    /* delay added here to spend some time in critical section so that we observe effects of backoff */
    int delay = 100000;
    while(delay)
        delay--;

    unlock(&l);
    // place an end timer here
    return vargp;
}


int main() {
    x = 0;
    pthread_t threads[NUM_THREADS];
    int i, j;
    l.next_ticket = 0;
    l.now_serving = 0;

    for (i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, operation, NULL);    // make the threads run the operation function
    }
    for (j = 0; j < NUM_THREADS; j++) {
        pthread_join(threads[j], NULL);                      // waits for all threads to be finished before function returns
    }

    printf("The value of x is : %d\n", x);
    return 0;
}

