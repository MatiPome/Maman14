#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "symbol_table.h"
#include "errors.h"

#define MAX_LINE_LEN 100
#define MAX_LABEL_LEN 31
#define MAX_DATA_SIZE 1024

/* Global counters */
int IC = 100; /* Instruction Counter */
int DC = 0;   /* Data Counter */

/* Data memory segment */
int data_memory[MAX_DATA_SIZE];

/* Global symbol table head */
symbol_node *symbol_table = NULL;

/* Reserved words for validation */
static const char* reserved_words[] = {
    "mov", "cmp", "add", "sub", "not", "clr", "lea",
    "inc", "dec", "jmp", "bne", "red", "prn", "jsr", "rts", "stop",
    ".data", ".string", ".entry", ".extern",
    "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
    NULL
};

/* Helper functions */

static int is_reserved_word(const char *word)
{
    int i = 0;
    while (reserved_words[i]) {
        if (strcmp(reserved_words[i], word) == 0)
            return 1;
        i++;
    }
    return 0;
}

static const char* skip_whitespace(const char *str)
{
    while (*str && isspace((unsigned char)*str))
        str++;
    return str;
}

/* Validate label according to Maman 14 rules */
static int validate_label(const char *label)
{
    int len = strlen(label);
    int i;

    if (len == 0 || len > MAX_LABEL_LEN)
        return 0;
    if (!isalpha((unsigned char)label[0]))
        return 0;
    for (i = 1; i < len; i++) {
        if (!isalnum((unsigned char)label[i]))
            return 0;
    }
    if (is_reserved_word(label))
        return 0;

    return 1;
}

/* Extract label from line if present */
int detectlabel(const char *line, char *label_name)
{
    const char *p = skip_whitespace(line);
    int i = 0;

    while (*p && *p != ':' && !isspace((unsigned char)*p) && i < MAX_LABEL_LEN + 1) {
        label_name[i++] = *p++;
    }
    label_name[i] = '\0';

    if (*p == ':') {
        if (validate_label(label_name))
            return 1;
        else
            return 0;
    }
    return 0;
}

/* Detect directive */
int is_directive(const char *line, char *directive_name)
{
    const char *p = skip_whitespace(line);

    if (*p != '.') return 0;

    int i = 0;
    while (*p && !isspace((unsigned char)*p) && i < 9) {
        directive_name[i++] = *p++;
    }
    directive_name[i] = '\0';

    if (strcmp(directive_name, ".data") == 0 ||
        strcmp(directive_name, ".string") == 0 ||
        strcmp(directive_name, ".extern") == 0 ||
        strcmp(directive_name, ".entry") == 0)
        return 1;

    return 0;
}

/* Handle .data directive */
void handle_data_directive(const char *line, int line_num)
{
    const char *p = strstr(line, ".data");
    char num_str[20];
    int i, val;

    if (!p) return;
    p += strlen(".data");

    while (*p) {
        p = skip_whitespace(p);
        if (!*p) break;

        i = 0;
        while (*p && *p != ',' && !isspace((unsigned char)*p) && i < 19) {
            num_str[i++] = *p++;
        }
        num_str[i] = '\0';

        if (i == 0) {
            report_error("Missing number in .data directive", line_num);
            return;
        }

        char *endptr;
        val = (int)strtol(num_str, &endptr, 10);
        if (*endptr != '\0') {
            report_error("Invalid number in .data directive", line_num);
            return;
        }

        if (DC >= MAX_DATA_SIZE) {
            report_error("Data memory overflow", line_num);
            return;
        }

        data_memory[DC++] = val;

        p = skip_whitespace(p);
        if (*p == ',') p++;
    }
}

/* Handle .string directive */
void handle_string_directive(const char *line, int line_num)
{
    const char *start = strchr(line, '"');
    const char *end;

    if (!start) {
        report_error("Missing opening quote in .string directive", line_num);
        return;
    }
    start++;

    end = strchr(start, '"');
    if (!end) {
        report_error("Missing closing quote in .string directive", line_num);
        return;
    }

    while (start < end) {
        if (DC >= MAX_DATA_SIZE) {
            report_error("Data memory overflow", line_num);
            return;
        }
        data_memory[DC++] = (int)(*start++);
    }
    if (DC >= MAX_DATA_SIZE) {
        report_error("Data memory overflow", line_num);
        return;
    }
    data_memory[DC++] = 0; /* Null terminator */
}

/* Update data symbol addresses by adding IC */
void update_data_symbol_addresses(symbol_node *head)
{
    while (head) {
        if ((head->attributes & 2) != 0) /* data attribute */
            head->address += IC;
        head = head->next;
    }
}

/* Main first pass function */
int first_pass(FILE *fp)
{
    char line[MAX_LINE_LEN];
    char label[MAX_LABEL_LEN + 1];
    char directive[10];
    int line_num = 0;
    int ret;
    int has_label;

    IC = 100;
    DC = 0;

    while (fgets(line, MAX_LINE_LEN, fp)) {
        line_num++;

        if (line[0] == ';' || line[0] == '\n')
            continue;

        has_label = detectlabel(line, label);
        if (has_label) {
            if (find_symbol(symbol_table, label)) {
                report_error("Duplicate label found", line_num);
            } else {
                /* Decide attribute based on next token (simplified: code = 1) */
                add_symbol(&symbol_table, label, IC, 1);
            }
        }

        ret = is_directive(line, directive);
        if (ret) {
            if (strcmp(directive, ".data") == 0)
                handle_data_directive(line, line_num);
            else if (strcmp(directive, ".string") == 0)
                handle_string_directive(line, line_num);
            else if (strcmp(directive, ".extern") == 0) {
                /* Handle extern */
                char extern_label[MAX_LABEL_LEN + 1];
                const char *p = line + strlen(".extern");
                p = skip_whitespace(p);
                int i = 0;
                while (*p && !isspace((unsigned char)*p) && i < MAX_LABEL_LEN) {
                    extern_label[i++] = *p++;
                }
                extern_label[i] = '\0';

                if (!validate_label(extern_label)) {
                    report_error("Invalid label name in .extern directive", line_num);
                } else if (find_symbol(symbol_table, extern_label)) {
                    report_error("Label already defined, cannot be extern", line_num);
                } else {
                    add_symbol(&symbol_table, extern_label, 0, 4);
                }
            } else if (strcmp(directive, ".entry") == 0) {
                /* Entry handled in second pass, optionally check label */
                /* Skipped here */
            }
            continue;
        }

        /* Instruction line */
        IC++;
    }

    update_data_symbol_addresses(symbol_table);

    return 0;
}

