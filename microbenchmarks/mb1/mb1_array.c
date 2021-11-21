/**
 * This microbenchmark is a very simple add operation to a shared variable
 * Creating array lock
 * 
 * Use this command to compile:
 * gcc -o array -lpthread mb1_mb1_array.c
 * Then to run:
 * ./array
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
//typically 64 bytes....but we have to check with the machine in which we run it
#define CACHE_LINE_SIZE 64 

typedef struct {
    // padded bytes here so that each int is in a different cache
    volatile int y;
    volatile char dummy[CACHE_LINE_SIZE - sizeof(int)]; //padding
} padded_int;


typedef struct {
    padded_int status[NUM_THREADS];
    volatile int head;
} lock_t;

lock_t lock;

volatile int x;

//int my_element;

int Lock(lock_t* l) {
    // assume circular increment;
    // TODO: create this function!!!!
    int my_element = atomic_circ_increment(&l->head);

    while ((l->status[my_element].y) == 1);

    return my_element;
}

void Unlock(lock_t* l, int element_number) {
    (l->status[element_number].y) = 1;
    (l->status[((element_number+1) % NUM_THREADS)].y) = 0;       
}

void *operation(void *vargp) {
    // place a start timer here
    int element_number = Lock(&lock);
    // place an end timer here

    // place a start timer here
    x++;
    // place an end timer here

    // place a start timer here
    Unlock(&lock, element_number);
    // place an end timer here
}


int main() {
    x = 0;
    pthread_t threads[NUM_THREADS];
    int i, j, k;
    lock.head = 0;     // the start of the array

    for (k = 0; k < NUM_THREADS; k++) {
        lock.status[k].y = 1;
    }

    // only the head element made as 0... acquires lock initially
    lock.status[lock.head].y = 0;

    for (i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, operation, NULL);    // make the threads run the operation function
    }

    for (j = 0; j < NUM_THREADS; j++) {
        pthread_join(threads[j], NULL);                      // waits for all threads to be finished before function returns
    }

    printf("The value of x is : %d\n", x);
    return 0;
}

