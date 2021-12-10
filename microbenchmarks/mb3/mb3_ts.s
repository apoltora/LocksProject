#
# 
# 
#

.global Lock, Unlock
.type Lock, %function
.type Unlock, %function

Lock:
    # this instruction happens atomically
    # copies the value in bit 0 of the memory location passed via %rdi to the Carry Flag
    # sets the bit 0 value to 1 for the memory location passed via %rdi
    lock bts $0, (%rdi) 
    # compare CF whether it was 0(lock obtained)...else keep looping
    jb Lock # loop if CF == 1
    retq

Unlock:
    movq   $0, (%rdi)
    retq
