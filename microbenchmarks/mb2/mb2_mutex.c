/**
 * This microbenchmark is a very simple memory access operation to a shared structure
 * Using mutex lock for this: pthread_mutex_init, pthread_mutex_lock, pthread_mutex_unlock
 * 
 * We know that the array is correct when all of its values are the same;
 * Regardless of the value, we want there to be synchronization for this struct
 * 
 * Use this command to compile:
 * gcc -o mutex -lpthread mb2_mutex.c
 * Then to run:
 * ./mutex
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
#define ARRAY_SIZE 10

extern void lock_atomic(volatile int *x);
volatile int x[ARRAY_SIZE];

typedef struct {
    int thread_id;
    pthread_t thread;
} thread_t;

pthread_mutex_t mutex;

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

void *operation(void *vargp) {
    thread_t *current_thread = (thread_t *)vargp;
    int thread_id = current_thread->thread_id;

    // place a start timer here
    pthread_mutex_lock(&mutex);
    // place an end timer here
    int i;
    for (i = 0; i < ARRAY_SIZE; i++)
        x[i] = thread_id;
    
    
    pthread_mutex_unlock(&mutex);
    // place an end timer here
}

void print_final_array(volatile int *array) {
    int i;
    for (i = 0; i < NUM_THREADS; i++) {
        printf("Array index: %d, Thread id %d\n",i, array[i]);
    }
}

int main() {
    initialize_array(x); // start all positions in the array to be 0
    thread_t threads[NUM_THREADS];
    initialize_threads(threads);
    int i, j;
    pthread_mutex_init(&mutex, NULL);

    for (i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i].thread, NULL, operation, (void*)&threads[i]);    // make the threads run the operation function
    }
    for (j = 0; j < NUM_THREADS; j++) {
        pthread_join(threads[j].thread, NULL);                      // waits for all threads to be finished before function returns
    }
    print_final_array(x);
    return 0;
}

