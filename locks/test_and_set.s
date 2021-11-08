#
#  Atomic test-and-set instruction:
#  ts R0, mem[addr] // load mem[addr] into R0
#  // if mem[addr] is 0, set mem[addr] to 1
#
#
#
#

lock:
    ts   R0, mem[addr]      // load word into R0
    bnz  R0, lock           // if 0, lock obtained

unlock:
    st   mem[addr], #0      // store 0 to address