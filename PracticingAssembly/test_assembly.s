#  Testing how to run assembly code in this file
#  The function below just prints "Hello, world!"
#  To compile it, in the terminal of this directory run "gcc test_assembly.s"
#  Then run it by calling "./a.out"
#  Basically, looks like it compiles/runs in the same way a C/C++ program would
#

.section .rodata             # read-only static data
.global hello
hello:
  .string "Hello, world!"    # zero-terminated C string

.text
.global main
main:
    push    %rbp
    mov     %rsp,  %rbp                 # create a stack frame

    mov     $hello, %edi                # put the address of hello into RDI
    call    puts                        #  as the first arg for puts

    mov     $0,    %eax                 # return value = 0.  Normally xor %eax,%eax
    leave                               # tear down the stack frame
    ret                            # pop the return address off the stack into RIP
