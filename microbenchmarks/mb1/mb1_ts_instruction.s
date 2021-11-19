# MODIFY THIS FILE
#
#

.global lock, 
.type test_and_set, %function

test_and_set:
    ts R0, mem[addr]