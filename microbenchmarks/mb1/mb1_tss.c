/**
 * This microbenchmark is a very simple add operation to a shared variable
 * Using test-and-test-and-set lock
 * 
 * Use this command to compile:
 * gcc -o tss -lpthread mb1_tss.c mb1_ts_instruction.s
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

int lock;
int x;

void Lock(int* lock) {
    while (1) {
        while (*lock != 0);
        if (test_and_set(*lock) == 0) // need to complete writing the test_and_set in assembly (it requires ARM)
            return;
    }
}
void Unlock(int* lock) { 
    *lock = 0;
}

void *operation(void *vargp) {
    // place a start timer here
    Lock(&lock);
    // place an end timer here

    // int x = *(int*)vargp;
    // x++;
    // *(int*)vargp = x;

    x++;
    Unlock(&lock);
    // place an end timer here
}


int main() {
    x = 0;
    pthread_t threads[NUM_THREADS];
    void *tmp_result;
    int i, j;
    lock = 0;
    for (i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, operation, NULL);    // make the threads run the operation function
    }
    for (j = 0; j < NUM_THREADS; j++) {
        pthread_join(threads[j], NULL);                      // waits for all threads to be finished before function returns
    }
    return x;
}

