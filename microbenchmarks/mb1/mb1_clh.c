/**
 * This microbenchmark is a very simple add operation to a shared variable
 * Creating clh queue lock
 * Check out visual on how this lock works here: https://classes.engineering.wustl.edu/cse539/web/lectures/locks.pdf
 * 
 * Use this command to compile:
 * clang -std=c11 -lpthread -o clh mb1_clh.c
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

#define NUM_THREADS 8
#define CACHE_LINE_SIZE 64 
#define TIMEOUT 10000000
#define DELAY_TIME 1000

typedef enum { LOCKED, UNLOCKED } qlock_state;

typedef struct qlock {

    volatile qlock_state state;
    char padding[CACHE_LINE_SIZE - sizeof(qlock_state)]; // padding
    struct qlock* volatile next;
    struct qlock* volatile prev;
    bool succ_must_wait;

} qlock_t;

qlock_t* volatile _Atomic glock;

int x;


/* AUTHOR'S CLH CODE BELOW IN THE COMMENT */
/*
typedef struct clh_qnode {
    volatile bool waiting;
    volatile struct clh_qnode *volatile prev;
} clh_qnode;

typedef volatile clh_qnode *clh_qnode_ptr;
typedef clh_qnode_ptr clh_lock;
// initialized to point to an unowned qnode

void clh_acquire (clh_lock *L, clh_qnode_ptr I) {
    I->waiting = true;
    clh_qnode_ptr pred = I->prev = swap(L, I);
    while (pred->waiting); // spin
}

void clh_release(clh_qnode_ptr *I) {
    clh_qnode_ptr pred = (*I)->preav;
    (*I)->waiting = false;
    *I = pred; // take pred's qnode
}
*/

typedef struct qnode {
    volatile bool waiting;
    volatile struct qnode *volatile prev;
    volatile struct qnode *volatile next;

} qnode;

qnode *tail;

void initialize_queue() {
    tail = (qnode *)malloc(sizeof(qnode));
    tail->prev = NULL;
    tail->next = NULL;
    tail->waiting = false;
}

/* TODO: MAKE THIS FUNCTION ACTUALLY WORK ATOMICALLY */
void atomic_swap(qnode *local_var, qnode *global_var) {
    qnode *tmp_local = (qnode*)malloc(sizeof(qnode));
    // temporarily store local variable before overwriting it
    tmp_local->waiting = local_var->waiting;
    tmp_local->prev = local_var->prev;
    tmp_local->next = local_var->next;

    // overwrite local variable to have whatever global variable has
    local_var->waiting = global_var->waiting;
    local_var->prev = global_var->prev;
    local_var->next = global_var->next;

    // take the local variable that was stored in temp and place in global variable
    global_var->waiting = tmp_local->waiting;
    global_var->prev = tmp_local->prev;
    global_var->next = tmp_local->next;

    // cleanup anything remaining unnecessary
    free(tmp_local);
}

qnode *acquire() {

    // create new qnode for this thread requesting
    qnode *new_qnode = (qnode *)malloc(sizeof(qnode));
    new_qnode->waiting = true;
    new_qnode->prev = tail;


    // add qnode to the queue; tail to next continuously
    while (1) {
        tail->next = new_qnode;
        // make sure the current tail points that its next node is 
        if (atomic_compare_exchange_weak(tail->next, new_qnode, new_qnode))
            break;
    }

    // swap tail with the new qnode
    atomic_swap(new_qnode, tail); /* TODO: MUST BE AN ATOMIC OPERATION OTHERWISE WILL CAUSE RACE CONDITIONS */


    // want to continuously loop until predecessor is finally done using the lock
    qnode *pred = new_qnode->prev;
    while (!pred->waiting); // while predecessor is NOT waiting (meaning, it still has its lock)

    // we leave spinning when lock is up for grabs from the predecessor

    new_qnode->waiting = false; // assign that this thread is done waiting for a lock

    return new_qnode;
}

void release(qnode *curr_qnode) {
    do {
        curr_qnode->waiting = false;

        // we want to keep looping until we are sure that the next thread in line gets the lock
        // IN AUTHOR'S IMPLEMENTATION there is no ->next field, but how can we free unless
        // we know that the next has addressed that the lock has been released
        if (atomic_compare_exchange_weak(curr_qnode->next->waiting,false,false))
            break;
    } while (1);

    // cleanup this lock request
    free(curr_qnode);
}

void *operation(void *vargp) {
    // place a start timer here
    qlock_t *mylock;
   
    mylock = AcquireQLock();

    // place an end timer here
    x++;

   // long delay = 1000000000;
    //while(delay)
      //  delay--;

    ReleaseQLock(mylock);

    // place an end timer here
    return vargp;
}


int main() {
    x = 0;

    // initialize glock
    initialize_queue();    

    pthread_t threads[NUM_THREADS];
    int i, j;

    for (i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, operation, NULL);    // make the threads run the operation function
    }

    for (j = 0; j < NUM_THREADS; j++) {
        pthread_join(threads[j], NULL);                      // waits for all threads to be finished before function returns
    }

    printf("The value of x is : %d\n", x);
    return 0;

}

