/**
 * Faulty code, as there are no locks
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

void *operation(void *vargp) {
    // place a start timer here
    // place an end timer here
    int x = *(int*)vargp;
    x++;
    *(int*)vargp = x;
    // place an end timer here
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
    printf("what is the end result %d\n",*(int*)tmp_result);
    return *(int*)tmp_result;
}

