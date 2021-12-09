/**
 * This microbenchmark is a very simple add operation to a shared variable
 * Creating clh_try queue lock
 * 
 * 
 * Use this command to compile:
 * clang -lpthread -o clh_try mb4_clh_try.c
 * Then to run:
 * ./clh_try
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
#include <sched.h>

#define NUM_THREADS 32
#define CACHE_LINE_SIZE 64 

// Timeout threshold // TODO: tune this value
// critical section 0.0266 sec
#define PATIENCE 79800000//slightly more time than 10 critical sections

#define MAX_CS_TIME 26600000 // critical section 0.0266 sec

#define UPD_DELAY 100000 // 100 microsecs 

#define ROW_SIZE 20
#define COL_SIZE 20

volatile int lock_holder_preemption_yield = 0;

volatile int num_timeouts = 0;

int *global_matrix_A;
int *global_matrix_B;

typedef enum {waiting,      // lock is held
            available,    // lock is free
            leaving,      // node owner is giving up
            transient,    // successor is giving up
            recycled}     // no pointers to node remain
    qnode_state;


typedef struct qnode {
    volatile qnode_state _Atomic status;
    char padding[CACHE_LINE_SIZE - sizeof(qnode_state)]; //padding
    struct qnode* volatile prev;
} qnode_t;


//qlock_t* volatile _Atomic glock;

// A structure to represent the global lock node
typedef struct qlock {

    // start time of the critical section
    volatile long crit_sec_start_time;
    char padding3[CACHE_LINE_SIZE - sizeof(long)]; //padding
    qnode_t* volatile _Atomic glock;

} qlock_t;


qlock_t lock;

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

bool AcquireQLock(qnode_t *mlock) 
{
    long start_time;
    qnode_state stat;

    mlock->status = waiting;
    mlock->prev = NULL;

    // predecessor in the queue
    qnode_t *pred; 

    while(1)
    {
        pred = lock.glock;
        // parameters are destination, expected value, desired value
        if(atomic_compare_exchange_weak(&(lock.glock), &pred, mlock))
            break;
    }

    // As an optimization, check the lock once before querying timer
    if (pred->status == available) {
        mlock->prev = NULL;
        free(pred);
        //mlock->prev = pred;
        // lock acquired
        lock.crit_sec_start_time = get_wall_clock_time_nanos();;
        return true;
    }
    
    start_time = get_thread_time_nanos();       

    // execute this while loop until the node times out
    while (get_thread_time_nanos() - start_time < PATIENCE) {
        stat = pred->status;
        if (stat == available) {
            mlock->prev = NULL;
            free(pred);
            //mlock->prev = pred;
            // lock acquired
            lock.crit_sec_start_time = get_wall_clock_time_nanos();;
            return true;
        }
        if (stat == leaving) {
            qnode_t *temp = pred->prev;
            pred->status = recycled;
            pred = temp;
        }
        // stat can also be transient,
        // if some node has left the queue just before my node entered
    }


    // my node timed out at this point

    while (1) {

        /* there is possiblility that pred's status can be transient while entering this while loop as some other node might be in the process 
        of leaving the queue. 
        if so just spin until state changes to waiting again */
        while (pred->status == transient); 

        while(1)
        {
            stat = pred->status;
            // parameters are destination, expected value, desired value
            if(atomic_compare_exchange_weak(&pred->status, &stat, transient))
                break;
        }


        if (stat == available) {
            mlock->prev = NULL;
            free(pred);
            //mlock->prev = pred;
            // lock acquired
            lock.crit_sec_start_time = get_wall_clock_time_nanos();;
            return true;
        }

        // if pred is leaving help pred to leave...
        if (stat == leaving) {
            qnode_t *temp = pred->prev;
            pred->status = recycled;
            pred = temp;
            continue;
        }
        break; // stat == waiting
    }

    /* updating prev pointer (as pred might have changed by now) so that successor can find it */
    mlock->prev = pred; 

    // indicate leaving, so successor can't leave
    while (1) {
        stat = waiting;
        atomic_compare_exchange_weak(&mlock->status, &stat, leaving);

        //swap success
        if (stat == waiting) 
            break; 

        // spin until the status becomes waiting again
        while (mlock->status != waiting);
    }


    /******* START LEAVING FROM THE QUEUE *********/

    /* special case: if last thread in the queue, swap the lock tail with pred and leave */

    qnode_t *mlock_temp = mlock;

    if(atomic_compare_exchange_weak(&(lock.glock), &mlock_temp, pred)){
        pred->status = waiting;

        /* no need to free mlock here as we are anyways going to do retry with mlock after sometime */
        return false;
    }

    // node is not the last in the queue
    while (1) {
        stat = mlock->status;
        if (stat == recycled) {
            pred->status = waiting;

            /* no need to free mlock here as we are anyways going to do retry with mlock after sometime */
            return false;
        }
    }
}



void ReleaseQLock(qnode_t *mlock)
{
    qnode_state temp_state = waiting;
    // If status is waiting change it to available (atomically)
 while (!atomic_compare_exchange_weak(&mlock->status, &temp_state, available)) {
        /* can't set my node to available if it's currently transient, so start spinning */
        while (mlock->status == transient);  
        temp_state = waiting;
    }
}



void *operation(void *vargp) {
    qnode_t *mylock;
    mylock = (qnode_t *) malloc(sizeof(qnode_t));

    bool acquire_status = true;

   
    while(1)
    {
        acquire_status = AcquireQLock(mylock);

        if(acquire_status) // lock acquire success
            break;
        else
        {
           // num_timeouts++;
            long temp = (lock.crit_sec_start_time + MAX_CS_TIME + UPD_DELAY);
            long ret_time = get_wall_clock_time_nanos();
            // as the lock timed out, yield now and continue later
            if(ret_time > temp)
            {
                //lock_holder_preemption_yield++;
                //printf("The value of temp is : %ld\n", temp);
                //printf("The value of ret_time is : %ld\n", ret_time);
                //printf("The value of lock.crit_sec_start_time is : %ld\n", lock.crit_sec_start_time);
                sched_yield();
            }

          // do immediate retry from here

        }

    }
    
    /**** CRITICAL SECTION *****/

    //x++;
    /*long delay = 100000000;
    while(delay)
        delay--;*/

    // call matrix multiplication to be done on 20x20 global matrices
    int *C = matrix_multiplication(global_matrix_A,global_matrix_B,ROW_SIZE,COL_SIZE,ROW_SIZE,COL_SIZE);


    /* End of CRITICAL SECTION */ 

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
    lock.crit_sec_start_time = 0;
    lock.glock = NULL;
    //glock = NULL;

    qnode_t *glock_init;
    glock_init = (qnode_t *) malloc(sizeof(qnode_t));

    glock_init->status = available;
    glock_init->prev = NULL;

    // initialize arrays to be used in the critical section
    global_matrix_A = initialize_matrix(ROW_SIZE, COL_SIZE);
    global_matrix_B = initialize_matrix(ROW_SIZE, COL_SIZE);

    qnode_t *prev_glock = lock.glock;
    atomic_compare_exchange_weak(&(lock.glock), &prev_glock, glock_init);

    // Create and spawn threads
    pthread_t threads[NUM_THREADS];
    int i, j;

    long time_init = get_wall_clock_time_nanos();

    for (i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, operation, NULL);    // make the threads run the operation function
    }

    for (j = 0; j < NUM_THREADS; j++) {
        pthread_join(threads[j], NULL);                      // waits for all threads to be finished before function returns
    }

    long time_final= get_wall_clock_time_nanos();

    long time_diff = time_final - time_init;
    
    printf("The value of num_timeouts is : %d\n", num_timeouts);
   // printf("The value of lock_holder_preemption_yield is : %d\n", lock_holder_preemption_yield);
    printf("Total RUNTIME : %lf\n\n", ((double) time_diff/1000000000));
    

    // free the final tail node of glock
    free(lock.glock);
    return 0;

}

