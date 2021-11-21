/**
 * This microbenchmark is a very simple add operation to a shared variable
 * Using test-and-test-and-set lock
 * 
 * Use this command to compile:
 * gcc -o tss -lpthread mb2_tss.c mb2_ts_instruction.s
 * Then to run:
 * ./tss
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

volatile int lock;
volatile int x[ARRAY_SIZE];

extern volatile int test_and_set(volatile int L);

typedef struct {
    int thread_id;
    pthread_t thread;
} thread_t;

extern volatile int test_and_set(volatile int L);

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

void Lock(volatile int* l) {
    while (1) {
        while (*l != 0);
        if (test_and_set(*l) == 0) // need to complete writing the test_and_set in assembly (it requires ARM)
            return;
    }
}
void Unlock(volatile int* l) { 
    *l = 0;
}

void *operation(void *vargp) {
    thread_t thread = *(thread_t*)vargp;
    int thread_id = thread.thread_id;
    // place a start timer here
    Lock(&lock);
    // place an end timer here
    int i;
    for (i = 0; i < ARRAY_SIZE; i++) {
        x[i] = thread_id;
    }
    Unlock(&lock);
    // place an end timer here
}


int main() {
    lock = 0;
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

