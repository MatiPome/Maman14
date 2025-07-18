/* code_conversion.c */
/* C90-compliant: All comments in C-style, ANSI C only */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "globals.h"
#include "table.h"

#define ADDR_IMMEDIATE 0
#define ADDR_DIRECT    1
#define ADDR_REGISTER  3

/* Map opcodes to their numeric values */
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

static int get_register(const char *s) {
    /* Reject any operand that starts with '@' */
    if (s[0] == '@') return -1;
    if (strlen(s) == 2 && s[0] == 'r' && s[1] >= '0' && s[1] <= '7' && s[2] == '\0')
        return s[1] - '0';
    return -1;
}

/* Removes leading and trailing whitespace (in-place) */
static void trim(char *s) {
    char *start = s;
    char *end;
    int len;

    while (*start && isspace((unsigned char)*start)) start++;
    if (start != s) {
        memmove(s, start, strlen(start) + 1);
    }
    len = strlen(s);
    if (len == 0) return;
    end = s + len - 1;
    while (end > s && isspace((unsigned char)*end)) {
        *end = '\0';
        end--;
    }
}

/* Parses the operands from a line, separating into src and dst */
static void parse_operands(const char *line, char *src, char *dst) {
    const char *p = line;
    char *comma;
    src[0] = '\0';
    dst[0] = '\0';

    /* Skip whitespace and opcode */
    while (*p && isspace((unsigned char)*p)) p++;
    while (*p && !isspace((unsigned char)*p)) p++;
    while (*p && isspace((unsigned char)*p)) p++;

    /* If no operand */
    if (*p == '\0' || *p == '\n') return;

    {
        char operands[100], *first, *second;
        strncpy(operands, p, 99);
        operands[99] = '\0';
        comma = strchr(operands, ',');
        if (comma) {
            *comma = '\0';
            first = operands;
            second = comma + 1;
            while (*first && isspace((unsigned char)*first)) first++;
            while (*second && isspace((unsigned char)*second)) second++;
            strcpy(src, first);
            strcpy(dst, second);
        } else {
            first = operands;
            while (*first && isspace((unsigned char)*first)) first++;
            strcpy(dst, first);
            src[0] = '\0';
        }
        trim(src);
        trim(dst);
    }
}

/* Returns the addressing mode for an operand */
static int get_addressing(const char *operand) {
    if (operand[0] == '#') return ADDR_IMMEDIATE;
    if (get_register(operand) != -1) return ADDR_REGISTER;
    return ADDR_DIRECT; /* assume label */
}

/* Encodes an instruction and stores it in instruction_memory[] */
void encode_instruction(const char *line, const char *opcode, int line_num, FILE *ob_file, FILE *ext_file)
{
    int opcode_val;
    int word;
    char src[50], dst[50];
    int src_addr = 0, dst_addr = 0;
    int src_reg = 0, dst_reg = 0;
    int src_num = 0, dst_num = 0;
    int need_extra_src = 0, need_extra_dst = 0;

    opcode_val = get_opcode_value(opcode);
    if (opcode_val == -1) {
        printf("Error (line %d): Unknown opcode '%s'\n", line_num, opcode);
        had_error = 1; /* ← דגל שגיאה */
        return;
    }

    parse_operands(line, src, dst);

    /* Reject illegal register syntax like @r1, @r2, etc */
    if ((src[0] == '@') || (dst[0] == '@')) {
        printf("Error (line %d): Illegal register syntax: '%s' or '%s'\n", line_num, src, dst);
        had_error = 1; /* ← דגל שגיאה */
        return;
    }

    /* Determine addressing modes and register values */
    if (src[0]) {
        src_addr = get_addressing(src);
        if (src_addr == ADDR_IMMEDIATE) {
            src_num = atoi(src + 1);
            need_extra_src = 1;
        } else if (src_addr == ADDR_REGISTER) {
            src_reg = get_register(src);
        } else {
            need_extra_src = 1;
        }
    }
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

    /* Build the main instruction word */
    word = (opcode_val << 8) | (src_addr << 6) | (dst_addr << 4) | (src_reg << 2) | dst_reg;

    printf("encode_instruction: opcode=%s | src='%s' | dst='%s' | word=%04X | IC=%d\n",
           opcode, src, dst, word, IC);

    instruction_memory[IC++] = word;

    /* Only add extra words for operands that are NOT both registers! */
    if (need_extra_src) {
        int val = 0;
        if (src_addr == ADDR_IMMEDIATE) {
            val = src_num;
        } else {
            symbol_node *sym = find_symbol(symbol_table, src);
            if (sym) val = sym->address;
            else {
                printf("Error (line %d): Undefined label '%s'\n", line_num, src);
                had_error = 1; /* ← דגל שגיאה */
            }
        }
        printf("encode_instruction: extra word for src: %04X at IC=%d\n", val, IC);
        instruction_memory[IC++] = val;
    }
    if (need_extra_dst) {
        int val = 0;
        if (dst_addr == ADDR_IMMEDIATE) {
            val = dst_num;
        } else {
            symbol_node *sym = find_symbol(symbol_table, dst);
            if (sym) val = sym->address;
            else {
                printf("Error (line %d): Undefined label '%s'\n", line_num, dst);
                had_error = 1; /* ← דגל שגיאה */
            }
        }
        printf("encode_instruction: extra word for dst: %04X at IC=%d\n", val, IC);
        instruction_memory[IC++] = val;
    }
}

/* Writes a single instruction word to the object file (.ob) */
void write_encoded_word(FILE *ob_file, int word) {
    fprintf(ob_file, "%04X\n", word & 0xFFFF);
}
