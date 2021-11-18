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

struct {
    
} thread_t;

void *operation(void *vargp) {
    int x = (int)vargp;
    lock_atomic(x);         // increments x by 1
    return NULL;
}


int main() {
    int x = 0;
    pthread_t thread;

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&thread, NULL, operation, (void*)&x);    // make the threads run the operation function
        pthread_join(&thread, NULL);                            // waits for all threads to be finished before function returns
    }
    return 0;
}

