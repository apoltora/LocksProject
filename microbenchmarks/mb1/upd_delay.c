/**
 * Checks the delay of a packet across interconnection network
 * 
 * 
 * Use this command to compile:
 * clang -std=c11 -o upd_delay -lpthread upd_delay.c
 * Then to run:
 * ./upd_delay
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
#include <stdint.h>
#include <stdatomic.h>
#include <time.h>

#define NUM_THREADS 16

volatile int x;
volatile int y;

volatile double timestamp1;
volatile double timestamp2;


typedef struct {
    int threadId;
} WorkerArgs;


double get_time_func()
{
    struct timespec t0;
    double time_in_sec;

    if(timespec_get(&t0, TIME_UTC) != TIME_UTC) {
        printf("Error in calling timespec_get\n");
        exit(EXIT_FAILURE);
    }

//   time_in_sec = ((double) t0.tv_sec);

    //? Will we need to do this? and is this correct ?
time_in_sec = ( ((double) t0.tv_sec) + ( ((double) t0.tv_nsec)/1000000000L ));
    
    return time_in_sec; // time_in_seconds
}


void *operation(void *threadArgs) {

    WorkerArgs* args = (WorkerArgs*) threadArgs;

    //printf("Thread %d reached here\n", args->threadId);

    int tid = args->threadId;

    if(tid == 0)
    {
        while(x == 0);
        timestamp2 = get_time_func();

    }

    if(tid == 4)
    {
        x++;
        timestamp1 = get_time_func();

    }

    if(tid == 1)
    {
        int i = 0;
        while(++i < 10000)
            y++;
    }


    if(tid == 5)
    {
        int i = 0;
        while(++i < 10000)
            y++;
    }


    
    return threadArgs;

}


int main() {
    x = 0;
    y = 0;
    pthread_t threads[NUM_THREADS];
    WorkerArgs args[NUM_THREADS];
    int i, j;

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
    //printf("The value of x is : %d\n", x);


    double time_diff;
    time_diff = (timestamp2 - timestamp1);

    printf("\n\n\ntimestamp2 %lf \n", timestamp2);
    printf("timestamp1 %lf \n", timestamp1);

    printf("time_diff %lf \n\n\n", time_diff);

    return 0;
}

