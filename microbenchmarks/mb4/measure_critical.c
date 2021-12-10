/**
 * This microbenchmark is a very simple add operation to a shared variable
 * Creating clh queue lock
 * Check out visual on how this lock works here: https://classes.engineering.wustl.edu/cse539/web/lectures/locks.pdf
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
#define ARRAY_SIZE 10000000
volatile long x[ARRAY_SIZE];

#define ROW_SIZE 20
#define COL_SIZE 20

int *global_matrix_A;
int *global_matrix_B;

//int x;

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

<<<<<<< HEAD
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

=======
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


>>>>>>> 055db6575ec73691edb299f6727123bb02c2fc78

void *operation(void *vargp) {
 

    long time_init = get_thread_time_nanos();

    //x++;
    /*long delay = 100000000;
    while(delay)
        delay--;*/

    int *C = matrix_multiplication(global_matrix_A,global_matrix_B,ROW_SIZE,COL_SIZE,ROW_SIZE,COL_SIZE);



    long time_final= get_thread_time_nanos();

    long time_diff = time_final - time_init;

    free(C);

    printf("CRIT_SEC RUNTIME : %lf\n\n", ((double) time_diff/1000000000));

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
   // x = 0;


    // initialize arrays to be used in the critical section
    global_matrix_A = initialize_matrix(ROW_SIZE, COL_SIZE);
    global_matrix_B = initialize_matrix(ROW_SIZE, COL_SIZE);

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

