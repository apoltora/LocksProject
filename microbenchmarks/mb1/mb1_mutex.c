/**
 * This microbenchmark is a very simple add operation to a shared variable
 * Using mutex lock for this: pthread_mutex_init, pthread_mutex_lock, pthread_mutex_unlock
 * 
 * Authors: Alexandra Poltorak & Kiran Kumar Rajan Babu
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

pthread_mutex_t mutex;

void *operation(void *vargp) {
    // place a start timer here
    pthread_mutex_lock(&mutex);
    // place an end timer here
    *(int*)vargp += 1;
    pthread_mutex_unlock(&mutex);
    // place an end timer here
    return vargp;
}


int main() {
    int x = 0;
    pthread_t threads[NUM_THREADS];
    void *tmp_result;
    int i, j;
    pthread_mutex_init(&mutex, NULL);

    for (i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, operation, (void*)&x);    // make the threads run the operation function
    }
    for (j = 0; j < NUM_THREADS; j++) {
        pthread_join(threads[j], &tmp_result);                      // waits for all threads to be finished before function returns
    }
    return *(int*)tmp_result;
}

