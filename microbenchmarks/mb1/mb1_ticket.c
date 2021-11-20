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
    int next_ticket;
    int now_serving;
} lock_t;

extern void atomic_increment(int *x);
volatile lock_t l;

int x;

void Lock(volatile lock_t *l) {
    atomic_increment(&l->next_ticket);
    int my_ticket = l->next_ticket;
    while (my_ticket != l->now_serving);
}

void unlock(volatile lock_t *l) {
    l->now_serving++;
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
    l.next_ticket = 0;
    l.now_serving = 1;

    for (i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, operation, NULL);    // make the threads run the operation function
    }
    for (j = 0; j < NUM_THREADS; j++) {
        pthread_join(threads[j], NULL);                      // waits for all threads to be finished before function returns
    }
    return x;
}

