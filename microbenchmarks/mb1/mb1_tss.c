/**
 * This microbenchmark is a very simple add operation to a shared variable
 * Using test-and-test-and-set with backoff lock
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

#define NUM_THREADS 10

volatile int lock;
volatile int x;

extern int test_and_set(volatile int* L);

typedef struct {
    int threadId;
} WorkerArgs;

void Lock(volatile int* l, int threadId) {
    int waiting_time = 0;
    while (1) {
        
        // test operation
        while (*l != 0)
        {
            //printf("Thread %d waiting here\n", threadId);
            waiting_time++;
        }

        // test and set operation
        // if the test and set returns 0...this means CF was 0 and the thread did acquire the lock.
        if (test_and_set(l) == 0)
        {
            printf("Thread %d finished and returning here....waitied for %d cycles to acquire the lock\n", threadId, waiting_time);
            return;
        }
       
    }
}

void Unlock(volatile int* l) { 
    *l = 0;
}

void *operation(void *threadArgs) {

    WorkerArgs* args = (WorkerArgs*) threadArgs;

    //printf("Thread %d reached here\n", args->threadId);

    // place a start timer here
    Lock(&lock, args->threadId);
    // place an end timer here
    x++;
    /* delay added here to spend some time in critical section so that we observe effects of backoff */
    int delay = 100000;
    while(delay)
        delay--;

    Unlock(&lock);
    // place an end timer here
}


int main() {
    x = 0;
    pthread_t threads[NUM_THREADS];
    WorkerArgs args[NUM_THREADS];
    int i, j;
    lock = 0;

    for (i=0; i<NUM_THREADS; i++) {
        // Thread arguments...
        args[i].threadId = i;
    }

    for (i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, operation, &args[i]);    // make the threads run the operation function
    }
    for (j = 0; j < NUM_THREADS; j++) {
        pthread_join(threads[j], NULL);                      // waits for all threads to be finished before function returns
    }
    printf("The value of x is : %d\n", x);
    return 0;
}

