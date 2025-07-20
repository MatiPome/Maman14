/* code_conversion.h
* Instruction encoding for assembler (C90)
 * Provides functions for translating assembly lines into encoded machine code.
 */

#ifndef CODE_CONVERSION_H
#define CODE_CONVERSION_H

#include <stdio.h>

/*
 * Encodes a single assembly instruction line.
 * - Parses operands and opcode, performs error checking.
 * - Stores the resulting machine words into the instruction memory.
 * - May also update object/ext files if relevant (usually during second pass).
 *
 * Parameters:
 *   line     - The full instruction line, including operands (e.g., "mov r1, r2")
 *   opcode   - The operation string ("mov", "add", etc.), must be extracted already
 *   line_num - The line number in the original file (for error reporting)
 */
void assemble_instruction(const char *line, const char *opcode, int line_num);

/*
 * Writes a 16-bit machine word in hexadecimal format to the given output file.
 * Used for both instruction and data memory outputs.
 *
 * Parameters:
 *   out   - File handle to write to (.ob file, for example)
 *   value - The 16-bit integer value to output (will be masked to 4 hex digits)
 */
void write_encoded_word(FILE *out, int value);

#endif /* CODE_CONVERSION_H */
