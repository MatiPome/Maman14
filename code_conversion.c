/* code_conversion.c
 * Converts assembly instructions to encoded machine words.
 * MMN14, ANSI C (C90) - supports matrix/indexed addressing.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "globals.h"
#include "table.h"
#include "util.h"

extern int inst_counter;
extern int data_counter;
extern int code_array[];
extern int data_memory[];
extern label_entry *symbol_table;
extern int error_flag;
extern FILE *ext_file;  /* For writing to .ext output file */

#define ADDR_IMMEDIATE 0 /* #number */
#define ADDR_DIRECT    1 /* label */
#define ADDR_MATRIX    2 /* label[index][index] */
#define ADDR_REGISTER  3 /* r0 - r7 */

/* Safely store a word in code_array, with overflow protection */
static int safe_store_code(int word, int line_num) {
    if (inst_counter + 1 >= MAX_INSTRUCTIONS) {
        printf("Error (line %d): instruction memory overflow (max = %d)\n", line_num, MAX_INSTRUCTIONS);
        error_flag = 1;
        return 0;
    }

    code_array[inst_counter] = word & 0x3FF; /* 10 bits only */
    inst_counter++;
    printf("DEBUG: Writing word for line %d, inst_counter now %d\n", line_num, inst_counter);
    return 1;
}




/* Converts an opcode name to its numerical value. */
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
    if (s[0] == '@') return -1;
    if (strlen(s) == 2 && s[0] == 'r' && s[1] >= '0' && s[1] <= '7' && s[2] == '\0')
        return s[1] - '0';
    return -1;
}

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

static void parse_operands(const char *line, char *src, char *dst) {
    const char *p = line;
    char *comma;
    src[0] = '\0';
    dst[0] = '\0';

    while (*p && isspace((unsigned char)*p)) p++;
    while (*p && !isspace((unsigned char)*p)) p++;
    while (*p && isspace((unsigned char)*p)) p++;
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

/* Checks if operand is a matrix (label[reg][reg]), sets out_label, out_reg1, out_reg2 */
static int is_matrix_operand(const char *operand, char *out_label, int *out_reg1, int *out_reg2) {
    int len, r1 = -1, r2 = -1;
    char label[MAX_LABEL_LENGTH+1];
    const char *p = strchr(operand, '[');
    if (!p) return 0; /* not a matrix */
    len = p - operand;
    if (len > MAX_LABEL_LENGTH) return 0;
    strncpy(label, operand, len);
    label[len] = '\0';
    /* Strict match: [rX][rY] */
    if (sscanf(p, "[r%d][r%d]", &r1, &r2) == 2 &&
        r1 >= 0 && r1 <= 7 && r2 >= 0 && r2 <= 7) {
        strcpy(out_label, label);
        *out_reg1 = r1;
        *out_reg2 = r2;
        return 1;
    }
    return 0;
}

/* Determines addressing mode: immediate, direct, matrix, register */
static int get_addressing(const char *operand) {
    char label[MAX_LABEL_LENGTH+1];
    int r1, r2;
    if (operand[0] == '#') return ADDR_IMMEDIATE;
    if (get_register(operand) != -1) return ADDR_REGISTER;
    if (is_matrix_operand(operand, label, &r1, &r2)) return ADDR_MATRIX;
    return ADDR_DIRECT;
}

/* Extract base label from operand (e.g. M1[r2][r7] -> M1) */
static void extract_base_label(const char* operand, char* out_label) {
    int i = 0;
    while (operand[i] && operand[i] != '[' && i < MAX_LABEL_LENGTH) {
        out_label[i] = operand[i];
        i++;
    }
    out_label[i] = '\0';
}

void assemble_instruction(const char *line, const char *opcode, int line_num)
{
    int opcode_val;
    int word;
    char src[50], dst[50];
    int src_addr = 0, dst_addr = 0;
    int src_reg = 0, dst_reg = 0;
    int src_num = 0, dst_num = 0;
    int need_extra_src = 0, need_extra_dst = 0;
    char label[MAX_LABEL_LENGTH+1];
    int mreg1, mreg2;
    label_entry *sym;
    int val;

    printf("assemble_instruction CALLED: '%s', opcode: '%s', line: %d\n", line, opcode, line_num);

    opcode_val = get_opcode_value(opcode);
    if (opcode_val == -1) {
        printf("Error (line %d): Unknown opcode '%s'\n", line_num, opcode);
        error_flag = 1;
        return;
    }

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
        } else if (src_addr == ADDR_MATRIX) {
            need_extra_src = 3; /* label, r1, r2 */
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
        } else if (dst_addr == ADDR_MATRIX) {
            need_extra_dst = 3;
        } else {
            need_extra_dst = 1;
        }
    }

    word = (opcode_val << 8) | (src_addr << 6) | (dst_addr << 4) | (src_reg << 2) | dst_reg;
    printf("DEBUG: Writing main word for line %d, inst_counter = %d\n", line_num, inst_counter);
    if (!safe_store_code(word, line_num)) return;

    /* Extra words for source */
    if (need_extra_src) {
        if (src_addr == ADDR_IMMEDIATE) {
            printf("DEBUG: Writing src immediate for line %d, value=%d, inst_counter=%d\n", line_num, src_num, inst_counter);
            if (!safe_store_code(src_num, line_num)) return;
        } else if (src_addr == ADDR_MATRIX) {
            if (is_matrix_operand(src, label, &mreg1, &mreg2)) {
                sym = find_symbol(symbol_table, label);
                val = 0;
                if (sym) {
                    val = sym->address;
                    if (sym->attributes & EXTERN_ATTRIBUTE && ext_file)
                        fprintf(ext_file, "%s %04d\n", sym->name, inst_counter);
                } else {
                    printf("Error (line %d): Undefined label '%s'\n", line_num, label);
                    error_flag = 1;
                }
                printf("DEBUG: Writing src matrix label for line %d, val=%d, inst_counter=%d\n", line_num, val, inst_counter);
                if (!safe_store_code(val, line_num)) return;
                printf("DEBUG: Writing src matrix reg1 for line %d, mreg1=%d, inst_counter=%d\n", line_num, mreg1, inst_counter);
                if (!safe_store_code(mreg1, line_num)) return;
                printf("DEBUG: Writing src matrix reg2 for line %d, mreg2=%d, inst_counter=%d\n", line_num, mreg2, inst_counter);
                if (!safe_store_code(mreg2, line_num)) return;
            }
        } else if (src_addr == ADDR_DIRECT) {
            char label_only[MAX_LABEL_LENGTH+1];
            extract_base_label(src, label_only);
            sym = find_symbol(symbol_table, label_only);
            val = 0;
            if (sym) {
                val = sym->address;
                if (sym->attributes & EXTERN_ATTRIBUTE && ext_file)
                    fprintf(ext_file, "%s %04d\n", sym->name, inst_counter);
            } else {
                printf("Error (line %d): Undefined label '%s'\n", line_num, label_only);
                error_flag = 1;
            }
            printf("DEBUG: Writing src direct for line %d, val=%d, inst_counter=%d\n", line_num, val, inst_counter);
            if (!safe_store_code(val, line_num)) return;
        }
    }

    /* Extra words for destination */
    if (need_extra_dst) {
        if (dst_addr == ADDR_IMMEDIATE) {
            printf("DEBUG: Writing dst immediate for line %d, value=%d, inst_counter=%d\n", line_num, dst_num, inst_counter);
            if (!safe_store_code(dst_num, line_num)) return;
        } else if (dst_addr == ADDR_MATRIX) {
            if (is_matrix_operand(dst, label, &mreg1, &mreg2)) {
                sym = find_symbol(symbol_table, label);
                val = 0;
                if (sym) {
                    val = sym->address;
                    if (sym->attributes & EXTERN_ATTRIBUTE && ext_file)
                        fprintf(ext_file, "%s %04d\n", sym->name, inst_counter);
                } else {
                    printf("Error (line %d): Undefined label '%s'\n", line_num, label);
                    error_flag = 1;
                }
                printf("DEBUG: Writing dst matrix label for line %d, val=%d, inst_counter=%d\n", line_num, val, inst_counter);
                if (!safe_store_code(val, line_num)) return;
                printf("DEBUG: Writing dst matrix reg1 for line %d, mreg1=%d, inst_counter=%d\n", line_num, mreg1, inst_counter);
                if (!safe_store_code(mreg1, line_num)) return;
                printf("DEBUG: Writing dst matrix reg2 for line %d, mreg2=%d, inst_counter=%d\n", line_num, mreg2, inst_counter);
                if (!safe_store_code(mreg2, line_num)) return;
            }
        } else if (dst_addr == ADDR_DIRECT) {
            char label_only[MAX_LABEL_LENGTH+1];
            extract_base_label(dst, label_only);
            sym = find_symbol(symbol_table, label_only);
            val = 0;
            if (sym) {
                val = sym->address;
                if (sym->attributes & EXTERN_ATTRIBUTE && ext_file)
                    fprintf(ext_file, "%s %04d\n", sym->name, inst_counter);
            } else {
                printf("Error (line %d): Undefined label '%s'\n", line_num, label_only);
                error_flag = 1;
            }
            printf("DEBUG: Writing dst direct for line %d, val=%d, inst_counter=%d\n", line_num, val, inst_counter);
            if (!safe_store_code(val, line_num)) return;
        }
    }
}


void write_encoded_word(FILE *ob_file, int word) {
    print_base4(ob_file, word & 0x3FF, 5);
    fprintf(ob_file, "\n");
}

