; Edge Case: Zero rows
LABEL1: .mat [0][3], 1, 2, 3

; Edge Case: Zero columns
.mat [2][0], 1, 2, 3

; Edge Case: Negative rows
.mat [-2][3], 1, 2, 3

; Edge Case: Negative columns
.mat [2][-3], 1, 2, 3

; Edge Case: Non-numeric row
.mat [x][3], 1, 2, 3

; Edge Case: Non-numeric col
.mat [3][abc], 1, 2, 3

; Edge Case: Missing one dimension
.mat [3], 1, 2, 3

; Edge Case: Missing both dimensions
.mat , 1, 2, 3

; Edge Case: Missing matrix data
.mat [2][2]

; Edge Case: Extra commas between values
.mat [2][2], 1,, 2, 3

; Edge Case: Overflow of data section (optional to test if you want to simulate MAX_DATA_SIZE being reached)
