/**
 * This microbenchmark is a very simple add operation to a shared variable
 * Using atomic lock for this, assembly instruction lock incl &x
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

extern void lock_atomic(int *x);


void *operation(void *vargp) {
    // set start timer here
    lock_atomic((int*)vargp);                                                 // increments x by 1
    // set end timer here
    return vargp;
}


int main() {
    int x = 0;
    pthread_t threads[NUM_THREADS];
    void *tmp_result;
    int i, j;

    for (i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, operation, (void*)&x);    // make the threads run the operation function
    }
    for (j = 0; j < NUM_THREADS; j++) {
        pthread_join(threads[j], &tmp_result);                      // waits for all threads to be finished before function returns
    }
    return *(int*)tmp_result;
}

