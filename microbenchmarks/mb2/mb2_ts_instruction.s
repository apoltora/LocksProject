# MODIFY THIS FILE
#
#

.global test_and_set
.type test_and_set, %function

# test_and_set:
#     ts R0, mem[addr]

test_and_set:
    bts         %rax, %rdi
    retq

