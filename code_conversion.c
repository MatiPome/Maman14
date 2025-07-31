/* code_conversion.c
 * Converts assembly instructions to encoded machine words.
 * MMN14, ANSI C (C90) - includes instruction memory overflow check.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "globals.h"
#include "table.h"

/* Externs from other modules */
extern int inst_counter;
extern int data_counter;
extern int code_array[];
extern int data_memory[];
extern label_entry *symbol_table;
extern int error_flag;
extern FILE *ext_file;  /* For writing to .ext output file */

/* Addressing mode definitions */
#define ADDR_IMMEDIATE 0 /* #number */
#define ADDR_DIRECT    1 /* label */
#define ADDR_REGISTER  3 /* r0 - r7 */

/* Safely store a word in code_array, with overflow protection */
static int safe_store_code(int word, int line_num) {
    if (inst_counter >= MAX_INSTRUCTIONS) {
        printf("Error (line %d): instruction memory overflow (max = %d)\n", line_num, MAX_INSTRUCTIONS);
        error_flag = 1;
        return 0; /* failed */
    }
    code_array[inst_counter++] = word & 0x3FF; /* 10 bits only */
    return 1; /* success */
}

/* Converts an opcode name to its numerical value.
 * Returns -1 if the opcode is unknown. */
static int get_opcode_value(const char *opcode) {
    if (strcmp(opcode, "mov") == 0) return 0;
    if (strcmp(opcode, "cmp") == 0) return 1;
    if (strcmp(opcode, "add") == 0) return 2;
    if (strcmp(opcode, "sub") == 0) return 3;
    if (strcmp(opcode, "not") == 0) return 4;
    if (strcmp(opcode, "clr") == 0) return 5;
    if (strcmp(opcode, "lea") == 0) return 6;
    if (strcmp(opcode, "inc") == 0) return 7;
    if (strcmp(opcode, "dec") == 0) return 8;
    if (strcmp(opcode, "jmp") == 0) return 9;
    if (strcmp(opcode, "bne") == 0) return 10;
    if (strcmp(opcode, "red") == 0) return 11;
    if (strcmp(opcode, "prn") == 0) return 12;
    if (strcmp(opcode, "jsr") == 0) return 13;
    if (strcmp(opcode, "rts") == 0) return 14;
    if (strcmp(opcode, "stop") == 0) return 15;
    return -1;
}

/* Checks if operand is a register (r0-r7, must not begin with '@').
 * Returns register number 0-7, or -1 if not a valid register. */
static int get_register(const char *s) {
    if (s[0] == '@') return -1; /* '@' is not allowed in register names */
    if (strlen(s) == 2 && s[0] == 'r' && s[1] >= '0' && s[1] <= '7' && s[2] == '\0')
        return s[1] - '0';
    return -1;
}

/* Trims whitespace from both ends of a string in-place. */
static void trim(char *s) {
    char *start = s, *end;
    int len;
    while (*start && isspace((unsigned char)*start)) start++;
    if (start != s) memmove(s, start, strlen(start) + 1);
    len = strlen(s);
    if (len == 0) return;
    end = s + len - 1;
    while (end > s && isspace((unsigned char)*end)) {
        *end = '\0';
        end--;
    }
}

/* Extracts the source and destination operands from an instruction line.
 * Example: "mov r1, r2" -> src = "r1", dst = "r2"
 * If only one operand exists, src is empty and dst contains the operand.
 */
static void parse_operands(const char *line, char *src, char *dst) {
    const char *p = line;
    char *comma;
    src[0] = '\0';
    dst[0] = '\0';

    /* Skip whitespace and opcode */
    while (*p && isspace((unsigned char)*p)) p++;
    while (*p && !isspace((unsigned char)*p)) p++;
    while (*p && isspace((unsigned char)*p)) p++;

    /* No operand case */
    if (*p == '\0' || *p == '\n') return;

    {
        char operands[100], *first, *second;
        strncpy(operands, p, 99);
        operands[99] = '\0';
        comma = strchr(operands, ',');
        if (comma) {
            /* Split into src and dst */
            *comma = '\0';
            first = operands;
            second = comma + 1;
            while (*first && isspace((unsigned char)*first)) first++;
            while (*second && isspace((unsigned char)*second)) second++;
            strcpy(src, first);
            strcpy(dst, second);
        } else {
            /* Only one operand (destination) */
            first = operands;
            while (*first && isspace((unsigned char)*first)) first++;
            strcpy(dst, first);
            src[0] = '\0';
        }
        trim(src);
        trim(dst);
    }
}

/* Determines the addressing mode of an operand (immediate, direct, register). */
static int get_addressing(const char *operand) {
    if (operand[0] == '#') return ADDR_IMMEDIATE;
    if (get_register(operand) != -1) return ADDR_REGISTER;
    return ADDR_DIRECT;
}

/* Encodes an instruction:
 * - Writes main instruction word to code_array (using safe_store_code)
 * - Adds extra words for immediate values or direct (label) addressing
 * - Performs error checking on opcode and operands
 */
void assemble_instruction(const char *line, const char *opcode, int line_num)
{
    int opcode_val;
    int word;
    char src[50], dst[50];
    int src_addr = 0, dst_addr = 0;
    int src_reg = 0, dst_reg = 0;
    int src_num = 0, dst_num = 0;
    int need_extra_src = 0, need_extra_dst = 0;

    /* Check opcode */
    opcode_val = get_opcode_value(opcode);
    if (opcode_val == -1) {
        printf("Error (line %d): Unknown opcode '%s'\n", line_num, opcode);
        error_flag = 1;
        return;
    }

    /* Parse operands and check syntax */
    parse_operands(line, src, dst);
    if ((src[0] == '@') || (dst[0] == '@')) {
        printf("Error (line %d): Illegal register syntax: '%s' or '%s'\n", line_num, src, dst);
        error_flag = 1;
        return;
    }

    /* Source operand */
    if (src[0]) {
        src_addr = get_addressing(src);
        if (src_addr == ADDR_IMMEDIATE) {
            src_num = atoi(src + 1); /* skip '#' */
            need_extra_src = 1;
        } else if (src_addr == ADDR_REGISTER) {
            src_reg = get_register(src);
        } else {
            need_extra_src = 1;
        }
    }
    /* Destination operand */
    if (dst[0]) {
        dst_addr = get_addressing(dst);
        if (dst_addr == ADDR_IMMEDIATE) {
            dst_num = atoi(dst + 1);
            need_extra_dst = 1;
        } else if (dst_addr == ADDR_REGISTER) {
            dst_reg = get_register(dst);
        } else {
            need_extra_dst = 1;
        }
    }

    /* Build the encoded word: [opcode][src_addr][dst_addr][src_reg][dst_reg] */
    word = (opcode_val << 8) | (src_addr << 6) | (dst_addr << 4) | (src_reg << 2) | dst_reg;
    if (!safe_store_code(word, line_num)) return;

    /* Add extra words if operand is immediate or direct (label) */
    if (need_extra_src) {
        int val = 0;
        if (src_addr == ADDR_IMMEDIATE) {
            val = src_num;
        } else {
            label_entry *sym = find_symbol(symbol_table, src);
            if (sym) {
                val = sym->address;
                if (sym->attributes & EXTERN_ATTRIBUTE && ext_file) {
                    fprintf(ext_file, "%s %04d\n", sym->name, inst_counter);
                }
            } else {
                printf("Error (line %d): Undefined label '%s'\n", line_num, src);
                error_flag = 1;
            }
        }
        if (!safe_store_code(val, line_num)) return;
    }

    if (need_extra_dst) {
        int val = 0;
        if (dst_addr == ADDR_IMMEDIATE) {
            val = dst_num;
        } else {
            label_entry *sym = find_symbol(symbol_table, dst);
            if (sym) {
                val = sym->address;
                if (sym->attributes & EXTERN_ATTRIBUTE && ext_file) {
                    fprintf(ext_file, "%s %04d\n", sym->name, inst_counter);
                }
            } else {
                printf("Error (line %d): Undefined label '%s'\n", line_num, dst);
                error_flag = 1;
            }
        }
        if (!safe_store_code(val, line_num)) return;
    }
}

/* Writes a 10-bit encoded instruction or data word to the output file */
void write_encoded_word(FILE *ob_file, int word) {
    fprintf(ob_file, "%03X\n", word & 0x3FF);
}
