# atomic increment of input variable (takes in the address of said variable)

.global atomic_increment
.type atomic_increment, %function

.section .rodata                    # read-only static data

atomic_increment:
    lock incl       (%rdi)     # increment the input by 1
    retq                       # return from the function
