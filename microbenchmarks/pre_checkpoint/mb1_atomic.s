# atomic increment of input variable (takes in the address of said variable)

.global atomic_increment
.type atomic_increment, %function

.section .rodata                    # read-only static data

atomic_increment:
    mov (%rdi), %rax # move the current ticket value to rax
    # r14 = ($rdi) + 1... increment ticket value and put it in r14...value to be swapped to the destination
    mov $1, %r14
    add %rax, %r14 
# compares rax and (%rdi)...if they are equal then puts %r14's (source) #value in (%rdi) which is the destination
    lock cmpxchg %r14, (%rdi) # src, dest
    # ZF == 0, then repeat the loop, swap not succeeded yet
    jne atomic_increment
     # rax will have the old ticket value when return happens
    # return from the function 
    ret
