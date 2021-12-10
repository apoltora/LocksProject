/**
 * This microbenchmark is a very simple add operation to a shared variable
 * Using test-and-set assembly instruction to create lock
 * 
 * Use this command to compile:
 * clang -o ts -lpthread mb3_ts.c mb3_ts.s
 * Then to run:
 * ./ts
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

#define NUM_THREADS 32
#define MAX_CRIT_ITERS 1
#define MAX_NON_CRIT_ITERS 1

#define ARRAY_SIZE 10000000
volatile long x[ARRAY_SIZE];

extern void Lock(volatile int *lock_var);
extern void Unlock(volatile int *lock_var);

volatile int LOCK;


void initialize_array(volatile long *array) {
    int i;
    for (i = 0; i < ARRAY_SIZE; i++) {
        array[i] = 0;
    }
}


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

void *operation(void *vargp) {
    /*
    Lock(&LOCK);
    x++;
    // delay added here to spend some time in critical section
    int delay = 100000;
    while(delay)
        delay--;
    Unlock(&LOCK);*/
 
 
    Lock(&LOCK);

    /**** CRITICAL SECTION *****/

 
    
    long delay = 100000000;
    while(delay)
        delay--;
  
    /**** END OF CRITICAL SECTION *****/    

    Unlock(&LOCK);


    return vargp;

}

int main() {
    LOCK = 0;
    pthread_t threads[NUM_THREADS];
    int i, j;

    //init array
    initialize_array(x);

    long time_init = get_wall_clock_time_nanos();

    for (i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, operation, NULL);    // make the threads run the operation function
    }
    for (j = 0; j < NUM_THREADS; j++) {
        pthread_join(threads[j], NULL);                      // waits for all threads to be finished before function returns
    }

    long time_final= get_wall_clock_time_nanos();

    long time_diff = time_final - time_init;
    
    //printf("The value of x is : %d\n", x);
    printf("Total RUNTIME : %lf\n\n", ((double) time_diff/1000000000));

   
    return 0;
}

