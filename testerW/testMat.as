; This is a test for .mat directive

MC1: mcro HELLO
    mov r3, r1
    prn r3
endmcro

HELLO
LABEL1: .mat [2][2] 1,2,3,4

mov r1, r2
