/**
 * Code to measure critical section time
 * 
 * Use this command to compile:
 * clang -lpthread -o measure_critical measure_critical.c
 * Then to run:
 * ./measure_critical
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

#define NUM_THREADS 4
#define CACHE_LINE_SIZE 64 



int x;

//function to return current wall clock time in nanosecs
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


//function to return thread specific clock time in nanosecs
long get_thread_time_nanos()
{
    struct timespec t0;
    long time_in_nano_sec;

    if(clock_gettime(CLOCK_THREAD_CPUTIME_ID, &t0) == -1)
    {
        printf("Error in calling clock_gettime\n");
        exit(EXIT_FAILURE);
    }

    time_in_nano_sec = (((long)t0.tv_sec * 1000000000L) + t0.tv_nsec);

    return time_in_nano_sec; // time_in_nano_seconds

}


void *operation(void *vargp) {
 

    long time_init = get_thread_time_nanos();

    //x++;
    long delay = 100000000;
    while(delay)
        delay--;


    long time_final= get_thread_time_nanos();

    long time_diff = time_final - time_init;

    printf("CRIT_SEC RUNTIME : %lf\n\n", ((double) time_diff/1000000000));

    return vargp;
}


int main() {
    x = 0;



    //long time_init = get_wall_clock_time_nanos();

    pthread_t threads[NUM_THREADS];
    int i, j;

    for (i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, operation, NULL);    // make the threads run the operation function
    }

    for (j = 0; j < NUM_THREADS; j++) {
        pthread_join(threads[j], NULL);                      // waits for all threads to be finished before function returns
    }

    //long time_final= get_wall_clock_time_nanos();

    //long time_diff = time_final - time_init;
    
    //printf("The value of x is : %d\n", x);
    //printf("Total RUNTIME : %lf\n\n", ((double) time_diff/1000000000));

    // free the final tail node of glock
    
    return 0;

}

