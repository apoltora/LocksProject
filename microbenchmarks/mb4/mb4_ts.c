/**
 * This microbenchmark is a very simple add operation to a shared variable
 * Using test-and-set assembly instruction to create lock
 * 
 * Use this command to compile:
 * clang -o ts -lpthread mb4_ts.c mb4_ts.s
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

#define ROW_SIZE 20
#define COL_SIZE 20

extern void Lock(volatile int *lock_var);
extern void Unlock(volatile int *lock_var);

volatile int LOCK;

int *global_matrix_A;
int *global_matrix_B;

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

    // place an end timer here
    //x++;
    // call matrix multiplication to be done on 20x20 global matrices
    int *C = matrix_multiplication(global_matrix_A,global_matrix_B,ROW_SIZE,COL_SIZE,ROW_SIZE,COL_SIZE);

    /**** END OF CRITICAL SECTION *****/    

    Unlock(&LOCK);
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
    LOCK = 0;
    pthread_t threads[NUM_THREADS];
    int i, j;

    // initialize arrays to be used in the critical section
    global_matrix_A = initialize_matrix(ROW_SIZE, COL_SIZE);
    global_matrix_B = initialize_matrix(ROW_SIZE, COL_SIZE);

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

