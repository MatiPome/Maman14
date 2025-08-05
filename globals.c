#include "globals.h"

/* ----------------------------------------------------------
 * IC (Instruction Counter):
 *   - Tracks the next available instruction address in memory.
 *   - Initialized to 100 (MMN14 convention).
 * ---------------------------------------------------------- */
int inst_counter = 99;

/* ----------------------------------------------------------
 * DC (Data Counter):
 *   - Tracks the next available data memory address.
 *   - Starts at 0 and increments for every .data/.string entry.
 * ---------------------------------------------------------- */
int data_counter = 0;

/* ----------------------------------------------------------
 * had_error:
 *   - Boolean flag (0 = no error, 1 = error occurred)
 *   - Set to 1 if an error is detected during assembly passes.
 *   - Can be checked after each pass to determine if output files should be written.
 * ---------------------------------------------------------- */
int error_flag = 0;

/* ----------------------------------------------------------
 * code_array:
 *   - Array storing encoded instruction words.
 *   - Size limited by MAX_INSTRUCTIONS (from globals.h).
 * ---------------------------------------------------------- */
int code_array[MAX_INSTRUCTIONS];

/* ----------------------------------------------------------
 * data_memory:
 *   - Array storing all .data and .string directive values.
 *   - Size limited by MAX_DATA_SIZE (from globals.h).
 * ---------------------------------------------------------- */
int data_memory[MAX_DATA_SIZE];
