This folder is dedicated to testing locks with microbenchmark 1
Microbenchmark 1 consists of a simple operation with multiple pthreads running (see below)

lock();
x++;
unlock();

This differs from how the atomic lock works as that is done with the assembly instruction:
lock xadd

To run files, call following commands:

shell>> gcc -lpthread -o "name of compiled file" "c file here" "assembly file here"
shell>> ./"name of compiled file"

Note: not all compilations will require an assembly file

We will have compiled files have the naming convention "locktype", where locktype is the type of lock
eg. atomic_lock, ticket_lock, mutex, etc.

Some thoughts before we start:
- mutex has overhead of initializing mutex (atomic does not)
- mutex needs to unlock (atomic does not)
