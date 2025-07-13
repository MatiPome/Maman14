/* code_conversion.c - Handles instruction encoding and word formatting */
#include <stdio.h>
#include <string.h>
#include "code_conversion.h"
#include "table.h"
#include "globals.h"
#include "Errors.h"
#include "util.h"

/* Dummy opcode mapping (you should adapt this to real opcode values) */
int get_opcode(const char *op) {
    if (strcmp(op, "mov") == 0) return 0;
    if (strcmp(op, "cmp") == 0) return 1;
    if (strcmp(op, "add") == 0) return 2;
    if (strcmp(op, "sub") == 0) return 3;
    if (strcmp(op, "not") == 0) return 4;
    if (strcmp(op, "clr") == 0) return 5;
    if (strcmp(op, "lea") == 0) return 6;
    if (strcmp(op, "inc") == 0) return 7;
    if (strcmp(op, "dec") == 0) return 8;
    if (strcmp(op, "jmp") == 0) return 9;
    if (strcmp(op, "bne") == 0) return 10;
    if (strcmp(op, "red") == 0) return 11;
    if (strcmp(op, "prn") == 0) return 12;
    if (strcmp(op, "jsr") == 0) return 13;
    if (strcmp(op, "rts") == 0) return 14;
    if (strcmp(op, "stop") == 0) return 15;
    return -1;
}

/* Write a 10-bit word to file (base64 not implemented here) */
void write_encoded_word(FILE *out, int value) {
    value &= 0x3FF; /* Keep only lowest 10 bits */
    fprintf(out, "%03d\n", value); /* Placeholder: use base64 encoding if needed */
}

/* Stub: this needs to be replaced with full instruction encoding */
void encode_instruction(const char *line, const char *opcode, int line_num, FILE *ob_file, FILE *ext_file) {
    int opcode_val = get_opcode(opcode);
    if (opcode_val < 0) {
        report_error("Unknown opcode", line_num);
        return;
    }

    /* Very basic encoding: just store the opcode in the top bits */
    int word = (opcode_val << 6); /* shift to high bits */

    instruction_memory[IC++] = word; /* Save into instruction memory */
}
