# MODIFY THIS FILE
#
#

.global test_and_set
.type test_and_set, %function


test_and_set:
    mov $0, %rax # initialize rax to 0
    # this instruction happens atomically
    # copies the value in bit 0 of the memory location passed via %rdi to the Carry Flag
    # sets the bit 0 value to 1 for the memory location passed via %rdi
    lock bts $0, (%rdi) 
    adc $0, %rax # now rax contains the CF...means the return value is CF
    ret

