/**
 * This microbenchmark is a very simple add operation to a shared variable
 * Creating array lock
 * 
 * Use this command to compile:
 * gcc -o array -lpthread mb1_array.c
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

#define NUM_THREADS 10
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

lock_t lock_var;

volatile int x;

//pthread_mutex_t mutex;

/*int atomic_circ_increment(volatile int* lock_head)
{
    int ret_val= *lock_head;

    *lock_head = ((*lock_head + 1) % NUM_THREADS);

    return ret_val;    
}*/


int lock(lock_t* l) {

    // ? is it fine to use mutex to implement atomic_circ_increment ?
    //pthread_mutex_lock(&mutex);
    int num_t = (NUM_THREADS - 1);
    int my_element = atomic_circ_increment(&l->head, num_t);
    //pthread_mutex_unlock(&mutex);

    while ((l->status[my_element].y) == 1);

    return my_element;
}

void unlock(lock_t* l, int element_number) {
    (l->status[element_number].y) = 1;
    (l->status[((element_number+1) % NUM_THREADS)].y) = 0;       
}

void *operation(void *vargp) {
    // place a start timer here
    int element_number = lock(&lock_var);
    // place an end timer here

    // place a start timer here

    x++;
    /* delay added here to spend some time in critical section so that we observe effects of backoff */
    int delay = 100000;
    while(delay)
        delay--;

    // place an end timer here

    // place a start timer here
    unlock(&lock_var, element_number);
    // place an end timer here
}


int main() {
    x = 0;
    pthread_t threads[NUM_THREADS];
    int i, j, k;
    lock_var.head = 0;     // the start of the array

    for (k = 0; k < NUM_THREADS; k++) {
        lock_var.status[k].y = 1;
    }

    // only the head element made as 0... acquires lock initially
    lock_var.status[lock_var.head].y = 0;

    // pthread_mutex_init(&mutex, NULL);

    for (i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, operation, NULL);    // make the threads run the operation function
    }

    for (j = 0; j < NUM_THREADS; j++) {
        pthread_join(threads[j], NULL);                      // waits for all threads to be finished before function returns
    }

    printf("The value of x is : %d\n", x);
    return 0;
}

