mov:     mov r1, r2      ; Invalid: reserved word
R0:      add r1, r2      ; Invalid: register name
My-Label: sub r1, r2     ; Invalid: non-alphanumeric
AReallyReallyLongLabelNameThatIsMoreThanThirtyOneCharacters: prn r1 ; Invalid: too long
ValidLabel: stop         ; OK