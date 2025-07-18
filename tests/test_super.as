; בדיקות פקודות חוקיות עם רשמים
mov r1, r2
add r3, r4
sub r5, r6
stop

; בדיקות .data חוקי/לא חוקי
DATA_LABEL: .data 5, -10, 33
BAD_DATA: .data 1, two, 3    ; שגיאה - "two" לא מספר

; בדיקות .string חוקי/לא חוקי
STRING_LABEL: .string "hello, world"
BAD_STRING: .string "missing_end    ; שגיאה - אין סוגר גרשיים

; בדיקות אופרטור לא קיים
notarealop r1, r2

; בדיקת מאקרו (אם תמיכה)
mcro M1
mov r1, r2
endmcro

; שימוש ב-label חוקי ובלתי חוקי
goodLabel: mov r1, r2
2badLabel: mov r2, r3    ; שגיאה - label מתחיל במספר

; ENTRY/EXTERN
.entry goodLabel
.extern OUT_LABEL
mov OUT_LABEL, r1

; בדיקת רשמים לא חוקיים
mov @r1, @r2      ; לא חוקי - עם שטרודל
mov r8, r2        ; לא חוקי - אין רשם r8
mov r1, r99       ; לא חוקי - אין רשם r99

; פקודות עם אופרטור בודד ובלי אופרטור
inc r3
stop
add

; טווחי נתונים
.data 32767, -32768     ; ערכים קצה חוקיים
.data 32768, -32769     ; ערכים לא חוקיים

; פסיק מיותר
mov r1,,r2
add ,r1, r2

; פקודה חוקית עם label
LABEL1: add r1, r2

; תגובה לפקודות עם label ארוך מדי
ThisLabelIsWayTooLongToBeValidAccordingToTheSpec: mov r1, r2

; תווים לא חוקיים ב-label
BAD#LABEL: mov r1, r2

; בדיקת הערות inline
mov r2, r3 ; inline comment

; שורות ריקות ורווחים

    mov r4, r5

; עוד פקודות תקינות
dec r1
prn r3
