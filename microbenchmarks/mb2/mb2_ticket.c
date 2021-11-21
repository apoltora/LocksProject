/**
 * This microbenchmark is a very simple memory access operation to a shared structure
 * Using atomic increment to implement ticket lock
 * 
 * We know that the array is correct when all of its values are the same;
 * Regardless of the value, we want there to be synchronization for this struct
 * 
 * Use this command to compile:
 * gcc -o ticket -lpthread mb2_ticket.c mb2_atomic.s
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


#define NUM_THREADS 128
#define ARRAY_SIZE 10

extern void lock_atomic(volatile int *x);
volatile int x[ARRAY_SIZE];

typedef struct lock {
    int next_ticket;
    int now_serving;
} lock_t;

extern void atomic_increment(volatile int *x);
volatile lock_t l;

typedef struct {
    int thread_id;
    pthread_t thread;
} thread_t;

void Lock(volatile lock_t *l) {
    atomic_increment(&l->next_ticket);
    int my_ticket = l->next_ticket;
    while (my_ticket != l->now_serving);
}

void unlock(volatile lock_t *l) {
    l->now_serving++;
}

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
    Lock(&l);
    // place an end timer here
    int i;
    for (i = 0; i < ARRAY_SIZE; i++) {
        x[i] = thread_id;
    }
    unlock(&l);
    // place an end timer here
}

void print_final_array(volatile int *array) {
    int i;
    for (i = 0; i < ARRAY_SIZE; i++) {
        printf("Array index: %d, Thread id %d\n",i, array[i]);
    }
}

int main() {
    initialize_array(x); // start all positions in the array to be 0
    thread_t threads[NUM_THREADS];
    initialize_threads(threads);
    int i, j;
    l.next_ticket = 0;
    l.now_serving = 1;

    for (i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i].thread, NULL, operation, (void*)&threads[i]);    // make the threads run the operation function
    }
    for (j = 0; j < NUM_THREADS; j++) {
        pthread_join(threads[j].thread, NULL);                      // waits for all threads to be finished before function returns
    }
    print_final_array(x); // for debugging correctness of code
    return 0;
}

