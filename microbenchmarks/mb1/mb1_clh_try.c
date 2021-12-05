/**
 * This microbenchmark is a very simple add operation to a shared variable
 * Creating clh_try queue lock
 * 
 * 
 * Use this command to compile:
 * clang -std=c11 -lpthread -o clh_try mb1_clh_try.c
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

#define NUM_THREADS 8
#define CACHE_LINE_SIZE 64 



typedef enum {waiting,      // lock is held
            available,    // lock is free
            leaving,      // node owner is giving up
            transient,    // successor is giving up
            recycled}     // no pointers to node remain
    clh_status;


typedef struct clh_qnode {
    volatile clh_status status;
    char padding[CACHE_LINE_SIZE - sizeof(clh_status)]; //padding
    volatile struct clh_qnode *volatile prev;
} clh_qnode;


typedef volatile clh_qnode *clh_qnode_ptr;
typedef clh_qnode_ptr clh_try_lock;

bool clh_try_acquire(clh_try_lock *L, clh_qnode_ptr I, hrtime_t T)
{
    hrtime_t start;
    clh_status stat;

    I->status = waiting;
    clh_qnode_ptr pred = swap(L, I);

    // check lock once before querying timer -- optimization
    if (pred->status == available) {
        I->prev = pred;
        return true;
    }
    start = gethrtime();       // high resolution clock

    while (gethrtime() - start < T) {
        stat = pred->status;
        if (stat == available) {
            I->prev = pred;
            return true;
        }
        if (stat == leaving) {
            clh_qnode_ptr temp = pred->prev;
            pred->status = recycled;
            pred = temp;
        }
        // stat might also be transient,
        // if somebody left the queue just before I entered
    }

    // timed out

    while (1) {
        while (pred->status == transient);  // spin
        stat = swap(&pred->status, transient);
        if (stat == available) {
            I->prev = pred;
            return true;
        }

        if (stat == leaving) {
            clh_qnode_ptr temp = pred->prev;
            pred->status = recycled;
            pred = temp;
            continue;
        }
        break;          // stat == waiting
    }
    I->prev = pred;     // so successor can find it

    // indicate leaving, so successor can't leave
    while (1) {
        stat = compare_and_swap(&I->status, waiting, leaving);
        if (stat == waiting) break; //swap success
        // spin until the status becomes waiting again
        while (I->status != waiting);
    }

    /* special case: if last thread in the queue, swap the lock tail with pred and leave */
    if (compare_and_store(L, I, pred)) {
        pred->status = waiting;
        return false;
    }

    // not last in the queue
    while (1) {
        stat = I->status;
        if (stat == recycled) {
            pred->status = waiting;
            return false;
        }
    }
}

void clh_release(clh_qnode_ptr *I)
{
    clh_qnode_ptr pred = (*I)->prev;
    while (!compare_and_store(&(*I)->status, waiting, available)) {
        // can't set my node to available if it's currently transient
        while ((*I)->status == transient);  // spin
    }
    *I = pred;
}

