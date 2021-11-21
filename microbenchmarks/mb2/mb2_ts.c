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

#define NUM_THREADS 8
#define ARRAY_SIZE 10

extern void lock(volatile int *lock_var);
extern void unlock(volatile int *lock_var);

volatile int LOCK;
volatile int x[ARRAY_SIZE];

typedef struct {
    int thread_id;
    pthread_t thread;
} thread_t;

void initialize_array(volatile int *array) {
    int i;
    for (i = 0; i < ARRAY_SIZE; i++) {
        array[i] = 0;
    }
}

void initialize_threads(thread_t *threads) {
    int thread;
    for (thread = 0; thread < NUM_THREADS; thread++) {
        threads[thread].thread_id = thread;
    }
}

void print_final_array(volatile int *array) {
    int i;
    for (i = 0; i < ARRAY_SIZE; i++) {
        printf("Array index: %d, Thread id %d\n",i, array[i]);
    }
}

void *operation(void *vargp) {
    thread_t thread = *(thread_t*)vargp;
    int thread_id = thread.thread_id;
    printf("I don't know why but infinite loop in lock for thread 0 if I don't have a print statement here\n");
    // place a start timer here
    lock(&LOCK);
    // place an end timer here
    int i;
    for (i = 0; i< ARRAY_SIZE; i++) {
        x[i] = thread_id;
    }
    unlock(&LOCK);
    // place an end timer here
}


int main() {
    LOCK = 0;
    initialize_array(x);
    thread_t threads[NUM_THREADS];
    initialize_threads(threads);
    int i, j;

    for (i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i].thread, NULL, operation, (void*)&threads[i]);    // make the threads run the operation function
    }
    for (j = 0; j < NUM_THREADS; j++) {
        pthread_join(threads[j].thread, NULL);                      // waits for all threads to be finished before function returns
    }
    return 0;
}

