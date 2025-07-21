; Test for macros, data, and string
mcro MY_MACRO
    add r1, r2
    sub r3, r4
endmcro

MAIN:   mov r5, r6
        MY_MACRO
        stop
VALUES: .data 7, -7, 42
MESSAGE: .string "Ok!"