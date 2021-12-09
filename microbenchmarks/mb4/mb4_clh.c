/**
 * This microbenchmark is a very simple add operation to a shared variable
 * Creating clh queue lock
 * Check out visual on how this lock works here: https://classes.engineering.wustl.edu/cse539/web/lectures/locks.pdf
 * 
 * Use this command to compile:
 * clang -lpthread -o clh mb4_clh.c
 * Then to run:
 * ./clh
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

#define NUM_THREADS 32
#define CACHE_LINE_SIZE 64 

#define MAX_CRIT_ITERS 1
#define MAX_NON_CRIT_ITERS 1

#define ROW_SIZE 20
#define COL_SIZE 20

int *global_matrix_A;
int *global_matrix_B;


typedef enum { LOCKED, UNLOCKED } qlock_state;

typedef struct qlock {

    volatile qlock_state state;
    char padding[CACHE_LINE_SIZE - sizeof(qlock_state)]; //padding
    struct qlock* volatile prev;

} qlock_t;

qlock_t* volatile _Atomic glock;


int *matrix_multiplication(int *A, int *B, int rows_A, int cols_A, int rows_B, int cols_B) {
    
    int length_A = rows_A * cols_A;
    int length_B = rows_B * cols_B;

    int *C = malloc(rows_A * cols_B * sizeof(int));
    
    // you cannot do matrix multiplication for matrices that are not the same size !!
    if (length_A != length_B || cols_A != rows_B) {
        return NULL;
    }

    int i,j,k;
    int A_index, B_index, C_index;
  
    for (i = 0; i < rows_A; i++) {
        for (j = 0; j < cols_B; j++) {
            // i * cols_A + j
            // we want row i col j for C
            C_index = i * cols_B + j;

            // C[i][j] = 0;
            C[C_index] = 0;
 
            for (k = 0; k < rows_B; k++) {
                A_index = i * cols_A + k;
                B_index = k * cols_B + j;
                // C[i][j] += A[i][k] * B[k][j];
                C[C_index] += A[A_index] * B[B_index];
            }
         }
     }

    return C;
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

qlock_t *AcquireQLock() {

    qlock_t *mlock;
    mlock = (qlock_t *) malloc(sizeof(qlock_t));

    mlock->state = LOCKED;

    qlock_t *prev_glock;

    while(1)
    {
        prev_glock = glock;

        // parameters are destination, expected value, desired value
        if(atomic_compare_exchange_weak(&glock, &prev_glock, mlock))
            break;

    }

    mlock->prev = prev_glock;

    while (mlock->prev->state == LOCKED); // SPIN HERE...

    // printf("I am here...\n");

    // free the previous node here as it wont be used anymore.
    free(prev_glock);

    return mlock;

}


void ReleaseQLock(qlock_t *mlock) {

    mlock->state = UNLOCKED;

}


void *operation(void *vargp) {
    // place a start timer here
    qlock_t *mylock;

    mylock = AcquireQLock();

    /**** CRITICAL SECTION *****/

    // place an end timer here

    // call matrix multiplication to be done on 20x20 global matrices
    int *C = matrix_multiplication(global_matrix_A,global_matrix_B,ROW_SIZE,COL_SIZE,ROW_SIZE,COL_SIZE);

    /**** END OF CRITICAL SECTION *****/    

    ReleaseQLock(mylock);

    free(C);

    return vargp;
}

/**
  * Initialize matrix; store whatever index it is at at that point
  * Return pointer to the matrix
  */
int *initialize_matrix(int rows, int cols) {
    int *matrix = (int*)malloc(sizeof(int)*rows*cols);

    int i;
    for (i = 0; i < rows * cols; i++) {
        matrix[i] = i;
    }
    return matrix;
}


int main() {

    // initialize glock
    glock = NULL;
    qlock_t *glock_init;
    glock_init = (qlock_t *) malloc(sizeof(qlock_t));

    glock_init->state = UNLOCKED;
    glock_init->prev = NULL;

    qlock_t *prev_glock = glock;
    atomic_compare_exchange_weak(&glock, &prev_glock, glock_init);

    // initialize arrays to be used in the critical section
    global_matrix_A = initialize_matrix(ROW_SIZE, COL_SIZE);
    global_matrix_B = initialize_matrix(ROW_SIZE, COL_SIZE);
    

    long time_init = get_wall_clock_time_nanos();

    pthread_t threads[NUM_THREADS];
    int i, j;

    for (i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, operation, NULL);    // make the threads run the operation function
    }

    for (j = 0; j < NUM_THREADS; j++) {
        pthread_join(threads[j], NULL);                      // waits for all threads to be finished before function returns
    }

    long time_final= get_wall_clock_time_nanos();

    long time_diff = time_final - time_init;
    
   // printf("The value of x is : %d\n", x);
    printf("Total RUNTIME : %lf\n\n", ((double) time_diff/1000000000));

    // free the final tail node of glock
    free(glock);
    free(global_matrix_A);
    free(global_matrix_B);
    return 0;

}


