
.global at_cmp_swap
.type at_cmp_swap, %function


at_cmp_swap:

    movq %rsi, %rax # move the prev_glock value to rax
    movq %rdx, %r14 # move the mlock value to r14...value to be swapped to the destination

    # compares rax and (%rdi)...if they are equal then puts %r14's (source) #value in (%rdi) which is the destination   
    # lock 
    # rex.w 
    lock cmpxchg %r14, (%rdi) # src, dest
   # lock cmpxchg %r14, %rdi # src, dest
    # rax will have the old glock value when return happens
    # return from the function 
    ret

