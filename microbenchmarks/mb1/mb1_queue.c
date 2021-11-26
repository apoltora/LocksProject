/**
 * This microbenchmark is a very simple add operation to a shared variable
 * Creating queue lock
 * 
 * Use this command to compile:
 * gcc -o queue -lpthread mb1_queue.c mb1_queue.s
 * Then to run:
 * ./queue
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

#define NUM_THREADS 8
#define CACHE_LINE_SIZE 64 

typedef enum { LOCKED, UNLOCKED } qlock_state;

typedef struct qlock {

    volatile qlock_state state;
    char padding[CACHE_LINE_SIZE - sizeof(qlock_state)]; //padding
    struct qlock* volatile next;

} qlock_t;


qlock_t* volatile glock;

int x;

qlock_t *AcquireQLock() {

    qlock_t *mlock;
    mlock = (qlock_t *) malloc(sizeof(qlock_t));

    assert(mlock != NULL);

    mlock->next = NULL;
    mlock->state = UNLOCKED;

    qlock_t *prev_glock;
    qlock_t *prev_glock_temp;
    long temp;


    while(1)
    {
        prev_glock = glock;

        temp = at_cmp_swap(&glock, prev_glock, mlock);
        //temp = at_cmp_swap(glock, prev_glock, mlock);
        prev_glock_temp = (qlock_t *) temp;

        // ptr, expected old value, new value to be inserted
       // bool result = __atomic_compare_exchange_n (glock, prev_glock, mlock, false);

        if(prev_glock == prev_glock_temp)
            break;  

        // printf("I am here 2\n");

    }

   // prev_glock = (void *) xchg_64((void *)mlock, (void *)glock);

    // no thread in the queue lock yet.
    if (prev_glock == NULL)
    {
        printf("I am here....1\n");
        return mlock;
    }

    mlock->state = LOCKED;
    prev_glock->next = mlock;
    printf("I am here\n");

    while (mlock->state == LOCKED); // SPIN HERE...

    return mlock;

}


void ReleaseQLock(qlock_t *mlock) {
    do {
        // last element condition
        if (mlock->next == NULL) {
            
            qlock_t *prev_glock_temp;
            long temp;

            // ptr, expected old value, new value to be inserted
            temp = at_cmp_swap(&glock, mlock, NULL);

            prev_glock_temp = (qlock_t *) temp;


            //printf("prev_glock_temp: %p \n", prev_glock_temp);

           // printf("mlock: %p\n", mlock);


            if(mlock == prev_glock_temp)
            {
                free(mlock);
                return; 
            }

        }
        else {
            mlock->next->state = UNLOCKED;
            free(mlock);
            return;
        }
    } while(1);
}


void *operation(void *vargp) {
    // place a start timer here
    qlock_t *mylock;
   
    mylock = AcquireQLock();

    // place an end timer here
    x++;

    //printf("I am here \n");
    
    ReleaseQLock(mylock);

  //  free(mylock);
    // place an end timer here
    return vargp;
}


int main() {
    x = 0;

    ///initialize glock
    glock = NULL;

    pthread_t threads[NUM_THREADS];
    int i, j;

    for (i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, operation, NULL);    // make the threads run the operation function
    }

    for (j = 0; j < NUM_THREADS; j++) {
        pthread_join(threads[j], NULL);                      // waits for all threads to be finished before function returns
    }


    // free the linkedlist
    /*while(glock != NULL)
    {
        qlock_t *lock = glock;
        glock = glock->next;
        free(lock);
    }*/

    printf("The value of x is : %d\n", x);
    return 0;

}

