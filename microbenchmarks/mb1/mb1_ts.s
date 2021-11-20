#
# This is ARM assembly -- cannot be run by a Mac
# 
#

.global lock, unlock
.type lock, %function
.type unlock, %function

lock:
    bts   %rax, %rdi
    test  %rax, %rax
    je   lock
    retq
    # bnz  %rax, lock

unlock:
    movq   $0, %rdi
    retq
    # st   mem[addr], #0

