#  First microbenchmark
#  Will be testing:
#  lock();
#  x++;
#  unlock();


.section .rodata             # read-only static data
main:
    movq    $0, %rdi         # store 0 inside of a register that will be manipulated
    # want to create threads to run this function
    lock # insert lock call here
    addq    $1, %rdi         # increment the register by 1
    # insert unlock call here
    retq                     # return from the function
