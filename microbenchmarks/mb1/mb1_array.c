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
#define LOCKS 4                 // arbitrarily set
#define PADDING 8               // arbitrarily set!! make sure padding is correct so each int is in a different cache

volatile int x;



typedef struct {
    // volatile padded_int status[P];              // padded to keep off same cache line, how should we do this????
    volatile status[LOCKS+PADDING];
    volatile int head;
} lock_t;

volatile lock_t lock;


int my_element;
void Lock(lock_t* l) {
  my_element = atomic_circ_increment(&l->head); // assume circular increment; TODO: create this function!!!!
  while (l->status[my_element] == 1);
}
void Unlock(lock_t* l) {
    l->status[my_element] = 1;
    l->status[circ_next(my_element)] = 0;       // next() gives next index
}

void *operation(void *vargp) {
    // place a start timer here
    Lock(&lock);
    // place an end timer here
    x++;
    Unlock(&lock);
    // place an end timer here
}


int main() {
    x = 0;
    pthread_t threads[NUM_THREADS];
    int i, j;
    // lock = 0;
    lock.head = 0;      // the start of the array
    for (i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, operation, NULL);    // make the threads run the operation function
    }
    for (j = 0; j < NUM_THREADS; j++) {
        pthread_join(threads[j], NULL);                      // waits for all threads to be finished before function returns
    }
    return x;
}

