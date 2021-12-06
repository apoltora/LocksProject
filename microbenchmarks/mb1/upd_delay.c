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

#define NUM_THREADS 20

volatile int x;
volatile int y;

volatile long timestamp1;
volatile long timestamp2;


typedef struct {
    int threadId;
} WorkerArgs;


long get_wall_clock_time_nanos()
{
    struct timespec t0;
    long time_in_nano_sec;

   /* if(timespec_get(&t0, TIME_UTC) != TIME_UTC) {
        printf("Error in calling timespec_get\n");
        exit(EXIT_FAILURE);
    }*/

    timespec_get(&t0, TIME_UTC);  

    time_in_nano_sec = (((long)t0.tv_sec * 1000000000L) + t0.tv_nsec);

    return time_in_nano_sec; // time_in_nano_seconds
}



void *operation(void *threadArgs) {

    WorkerArgs* args = (WorkerArgs*) threadArgs;

    //printf("Thread %d reached here\n", args->threadId);

    int tid = args->threadId;

    if(tid == 0)
    {
        while(x == 0);
        timestamp2 = get_wall_clock_time_nanos();

    }

    if(tid == 4)
    {
        x++;
        timestamp1 = get_wall_clock_time_nanos();

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


    long time_diff;
    time_diff = (timestamp2 - timestamp1);

    printf("\n\n\ntimestamp2 %ld \n", timestamp2);
    printf("timestamp1 %ld \n", timestamp1);

    printf("time_diff : %lf secs\n\n\n", ((double) time_diff/1000000000));

    return 0;
}

