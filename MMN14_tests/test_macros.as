mcro PRINT_R3
    mov r3, r1
    prn r3
endmcro

LABEL_A: .data 5
PRINT_R3

mcro PRINT_R3   ; duplicate macro name -> should report error
