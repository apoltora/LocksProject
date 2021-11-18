/**
 * This microbenchmark is a very simple addition to a shared variable
 * Alexandra Poltorak & Kiran Kumar Rajan Babu
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

extern int lock_atomic(int x);


void *operation(void *vargp) {
    int x = *(int *)vargp;
    lock_atomic(x);         // increments x by 1
    return &x;
}


int main() {
    int x = 0;
    pthread_t threads[NUM_THREADS];
    int result;
    void *tmp_result;

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, operation, (void*)&x);    // make the threads run the operation function
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(&threads[i], &tmp_result);                     // waits for all threads to be finished before function returns
        int *part_result = (int*) tmp_result;
        result += *part_result;
    }

    return result;
}

