/**
 * This microbenchmark is a very simple add operation to a shared variable
 * Using test-and-set assembly instruction to create lock
 * 
 * Use this command to compile:
 * gcc -o ts -lpthread mb1_ts.c mb1_ts.s
 * Then to run:
 * ./ts
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

#define NUM_THREADS 128

extern void lock(volatile int *lock_var);
extern void unlock(volatile int *lock_var);

volatile int LOCK;
volatile int x;

void *operation(void *vargp) {
    // place a start timer here
    lock(&LOCK);
    // place an end timer here
    x++;
    unlock(&LOCK);
    // place an end timer here
}

int main() {
    LOCK = 0;
    x = 0;
    pthread_t threads[NUM_THREADS];
    void *tmp_result;
    int i, j;

    for (i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, operation, NULL);    // make the threads run the operation function
    }
    for (j = 0; j < NUM_THREADS; j++) {
        pthread_join(threads[j], &tmp_result);                      // waits for all threads to be finished before function returns
    }
    printf("what is x?? %d\n",x);
    return x;
}

