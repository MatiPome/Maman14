; MMN14 test file

macro m1
mov r1, r2
add #5, r3
endmcro

MAIN:   mov  r3,  r1
        m1
LOOP:   prn  #45
        lea  STR, r6
        inc  r6
        mov  r6,  K
        sub  r1, r4
        bne  LOOP
        stop

STR:    .string "abcdef"
LIST:   .data 6, -9
K:      .data 22
