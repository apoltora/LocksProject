CMU 15-418

Locks Project

Authors: Alexandra Poltorak, Kiran Kumar Rajan Babu

Contact: apoltora@andrew.cmu.edu, krajanba@andrew.cmu.edu

Using micro-benches to compare and contrast the consequences of different locks, we will be rigorously testing a set of locks (namely queue, ticket, etc.) to see their overhead, contention, fairness, and other, and measuring how each compares. The micro-benches will be very simple so that the focus remains on the locks.

Writing code in Assembly

Compile assembly using gcc, run by calling ./a.out (or whatever the compiled file is named)

https://apoltora0.wixsite.com/locks

Useful commands:

ts - test and set

lock cmpxchg dst, src; cmpxchg - compare and exchange (atomic when used with lock prefix), lock prefix (makes operation atomic):

if (dst == EAX)

    ZF = 1

    dst = src

else

ZF = 0

EAX = dst