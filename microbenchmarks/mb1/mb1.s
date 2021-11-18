#  First microbenchmark
#  Will be testing:
#  lock();
#  x++;
#  unlock();

.global lock_atomic
.type lock_atomic, %function

.section .rodata                    # read-only static data

lock_atomic:
    lock incl       (%rdi)     # increment the input by 1
    retq                          # return from the function
