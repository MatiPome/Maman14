/* code_conversion.h - Instruction encoding header */
#ifndef CODE_CONVERSION_H
#define CODE_CONVERSION_H

#include <stdio.h>

/* Encodes an instruction line into machine code and stores it */
void encode_instruction(const char *line, const char *opcode, int line_num, FILE *ob_file, FILE *ext_file);

/* Writes a 10-bit machine word to the output file (placeholder format) */
void write_encoded_word(FILE *out, int value);

#endif
