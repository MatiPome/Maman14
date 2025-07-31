#ifndef GLOBALS_H
#define GLOBALS_H

/* -----------------------------------------------------------------
 * MAX_INSTRUCTIONS:
 *   - The maximum number of machine code instructions that can be
 *     stored in the instruction memory.
 *   - Used for bounds checking and array sizing.
 * ----------------------------------------------------------------- */
#define MAX_INSTRUCTIONS 1024

/* -----------------------------------------------------------------
 * MAX_DATA_SIZE:
 *   - The maximum number of data elements (.data/.string) that can
 *     be stored in the data memory.
 *   - Used for bounds checking and array sizing.
 * ----------------------------------------------------------------- */
#define MAX_DATA_SIZE 1024

/* -----------------------------------------------------------------
 * MAX_LINE_LENGTH:
 *   - The maximum allowed length for a line in the assembly source file.
 *   - Used to allocate buffers for reading lines.
 * ----------------------------------------------------------------- */
#define MAX_LINE_LENGTH 80

/* -----------------------------------------------------------------
 * MAX_LABEL_LENGTH:
 *   - The maximum allowed length for a label name.
 *   - Used for validation and to size label buffers.
 * ----------------------------------------------------------------- */
#define MAX_LABEL_LENGTH 32

/* -----------------------------------------------------------------
 * Global variables shared across the assembler components:
 *   - Declared as 'extern' here, defined in globals.c.
 * ----------------------------------------------------------------- */

/* Instruction Counter: Points to the next available instruction memory slot. */
extern int inst_counter;

/* Data Counter: Points to the next available data memory slot. */
extern int data_counter;

/* Error flag: Set to 1 if an error is encountered anywhere in the assembler. */
extern int error_flag;

/* The main instruction memory array, storing encoded instruction words. */
extern int code_array[MAX_INSTRUCTIONS];

/* The data memory array, storing all .data/.string directive values. */
extern int data_memory[MAX_DATA_SIZE];

#endif /* GLOBALS_H */
