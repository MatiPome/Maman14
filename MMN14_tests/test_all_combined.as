; Combined final test of all valid features

.extern EXTLABEL
.entry MYENTRY

DATA1: .data 5, -3, 15
STR1:  .string "hello"
MAT1:  .mat [2][2] 1,2,3,4

MYENTRY: mov r1, r2
         add #5, r3
         jsr EXTLABEL
         stop
