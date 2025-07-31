; Test all directive types and basic instruction

.extern EXTLABEL
.entry MYENTRY

LABEL1: .data 5, -3, 15
LABEL2: .string "hello"
LABEL3: .mat [2][2] 1,2,3,4

MYENTRY: mov r1, r2
        add #5, r3
        jsr EXTLABEL
        stop
