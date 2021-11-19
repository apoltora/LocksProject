#
# This is ARM assembly -- cannot be run by a Mac
# 
#

.global lock, 
.type lock, %function
.type unlock, %function

lock:
    ts   R0, mem[addr]
    bnz  R0, lock

unlock:
    st   mem[addr], #0