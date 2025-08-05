; MMN14 Everything Test - legal for all standard MMN14 assemblers

; -- Data and String Directives --
LABELDATA: .data 1, -2, 32767, -32768, 0
EXTRADATA: .data 99, 100, 101
EMPTYDATA: .data
STR1: .string "Hello MMN14!"
STR2: .string ""
STR3: .string " "
STR4: .string "a"

; -- Matrix Directives --
M1: .mat [2][2], 1, 2, 3, 4
M2: .mat [1][4], 5, 6, 7, 8
M3: .mat [3][1], 9, 10, 11

; -- Labels, Comments, White Space --
LABELONLY:
    ; This is a comment
LABEL2: add r0, r1
    sub r2, r3
inc r4
dec r5
not r6
clr r7

; -- Various Addressing --
mov r0, r1
mov #42, r2
lea LABELDATA, r3
add EXTRADATA, r4
cmp r4, LABEL2
prn STR1
red r6

; -- Jump/Branch/JSR/RTS/STOP --
JMPLABEL: jmp JMPLABEL
BNELABEL: bne JMPLABEL
JSRLABEL: jsr LABEL2
stop
rts

; -- .entry and .extern --
.extern EXTLABEL
.extern EXTLABEL2
.entry LABEL2
.entry JMPLABEL

; -- Use Extern Labels --
mov EXTLABEL, r1
add EXTLABEL2, r2

; -- More White Space --

LABEL3: prn #1
LABEL4: prn #-99

; -- Test with empty line below --

; -- End --
