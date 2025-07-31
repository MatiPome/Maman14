#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "globals.h"
#include "table.h"
#include "errors.h"

/* Check if a line is a comment or empty */
int is_comment_or_empty(const char *line) {
    while (*line) {
        if (!isspace(*line)) return (*line == ';' || *line == '\n');
        line++;
    }
    return 1;
}

/* Skip initial whitespace */
const char *skip_whitespace(const char *str) {
    while (*str && isspace((unsigned char)*str)) str++;
    return str;
}

/* Validate a label */
int validate_label(const char *label) {
    int i, len = strlen(label);
    const char *reserved[] = {
        "mov","cmp","add","sub","not","clr","lea","inc","dec","jmp","bne",
        "red","prn","jsr","rts","stop",
        "r0","r1","r2","r3","r4","r5","r6","r7",
        ".data",".string",".mat",".entry",".extern"
    };
    int reserved_count = sizeof(reserved) / sizeof(reserved[0]);

    if (!isalpha(label[0]) || len > MAX_LABEL_LENGTH)
        return 0;

    for (i = 1; i < len; i++) {
        if (!isalnum(label[i]))
            return 0;
    }

    for (i = 0; i < reserved_count; i++) {
        if (strcmp(label, reserved[i]) == 0)
            return 0;
    }

    return 1;
}


int is_reserved_word(const char *word) {
    const char *reserved[] = {
        "mov", "cmp", "add", "sub", "not", "clr",
        "lea", "inc", "dec", "jmp", "bne", "red",
        "prn", "jsr", "rts", "stop",
        "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
        ".data", ".string", ".entry", ".extern", ".mat"
    };
    int i;
    for (i = 0; i < sizeof(reserved)/sizeof(reserved[0]); i++) {
        if (strcmp(word, reserved[i]) == 0)
            return 1;
    }
    return 0;
}



/* Extract label if present */
int detectlabel(const char *line, char *label) {
    const char *p = skip_whitespace(line);
    int i = 0;

    while (*p && *p != ':' && !isspace(*p) && i < MAX_LABEL_LENGTH) {
        label[i++] = *p++;
    }

    label[i] = '\0';

    if (*p == ':' && validate_label(label)) {
        return 1; /* valid label */
    }

    return 0; /* not a valid label */
}


/* Skip label: */
const char* skip_label_colon(const char *line) {
    const char *p = strchr(line, ':');
    if (!p) return line;
    p++;
    while (*p && isspace(*p)) p++;
    return p;
}

/* Check for directive */
int is_directive(const char *line, char *directive_name) {
    const char *p = skip_whitespace(line);
    int i = 0;
    if (*p != '.') return 0;
    while (*p && !isspace(*p) && i < 9) directive_name[i++] = *p++;
    directive_name[i] = '\0';
    return (!strcmp(directive_name, ".data") || !strcmp(directive_name, ".string") ||
            !strcmp(directive_name, ".extern") || !strcmp(directive_name, ".entry") ||
            !strcmp(directive_name, ".mat"));
}

/* Handle .data directive */
void handle_data_directive(const char *line, int line_num) {
    const char *p = strstr(line, ".data");
    char num_str[20]; int val, i = 0; char *endptr;
    if (!p) return;
    p += 5;
    while (*p) {
        p = skip_whitespace(p);
        if (!*p) break;
        i = 0;
        while (*p && *p != ',' && !isspace(*p) && i < 19) num_str[i++] = *p++;
        num_str[i] = '\0';
        if (i == 0) { report_error("Missing number in .data", line_num); error_flag = 1; return; }
        val = strtol(num_str, &endptr, 10);
        if (*endptr != '\0') { report_error("Invalid number in .data", line_num); error_flag = 1; return; }
        if (data_counter >= MAX_DATA_SIZE) { report_error("Data memory overflow", line_num); error_flag = 1; return; }
        data_memory[data_counter++] = val;
        p = skip_whitespace(p);
        if (*p == ',') p++;
    }
}

/* Handle .string directive */
void handle_string_directive(const char *line, int line_num) {
    const char *start = strchr(line, '"');
    const char *end;
    if (!start) { report_error("Missing opening quote", line_num); error_flag = 1; return; }
    start++;
    end = strchr(start, '"');
    if (!end) { report_error("Missing closing quote", line_num); error_flag = 1; return; }
    while (start < end) {
        if (data_counter >= MAX_DATA_SIZE) { report_error("Data memory overflow", line_num); error_flag = 1; return; }
        data_memory[data_counter++] = *start++;
    }
    data_memory[data_counter++] = 0;
}

/* Handle .mat directive */
void handle_mat_directive(const char *line, int line_num) {
    const char *p = strstr(line, ".mat");
    int rows = 0, cols = 0, val, i = 0; char *endptr;
    if (!p) return;
    p += 4;
    p = skip_whitespace(p);
    if (*p++ != '[' || (rows = strtol(p, &endptr, 10)) <= 0 || *endptr != ']') {
        report_error("Invalid matrix rows", line_num); error_flag = 1; return;
    }
    p = endptr + 1;
    if (*p++ != '[' || (cols = strtol(p, &endptr, 10)) <= 0 || *endptr != ']') {
        report_error("Invalid matrix cols", line_num); error_flag = 1; return;
    }
    p = endptr + 1;
    for (i = 0; i < rows * cols; i++) {
        p = skip_whitespace(p);
        if (!*p) break;
        val = strtol(p, &endptr, 10);
        if (p == endptr) {
            report_error("Invalid matrix value", line_num); error_flag = 1; return;
        }
        if (data_counter >= MAX_DATA_SIZE) {
            report_error("Matrix data overflow", line_num); error_flag = 1; return;
        }
        data_memory[data_counter++] = val;
        p = endptr;
        p = skip_whitespace(p);
        if (*p == ',') p++;
    }
}

/* Update addresses of data labels after code */
void update_data_symbol_addresses(label_entry *head) {
    while (head) {
        if ((head->attributes & 2) != 0)
            head->address += inst_counter;
        head = head->next;
    }
}

/* First pass logic */
int first_pass(FILE *fp) {
    char line[MAX_LINE_LENGTH];
    char label[MAX_LABEL_LENGTH + 1];
    char directive[10];
    int line_num = 0, has_label, i;
    const char *after_label;

    inst_counter = 100;
    data_counter = 0;
    error_flag = 0;

    while (fgets(line, MAX_LINE_LENGTH, fp)) {
        line_num++;
        if (is_comment_or_empty(line)) continue;

        has_label = detectlabel(line, label);
        after_label = line;
        if (has_label) {
            if (!validate_label(label)) {
                report_error("Invalid label name", line_num);
                error_flag = 1;
            } else if (find_symbol(symbol_table, label)) {
                report_error("Duplicate label", line_num);
                error_flag = 1;
            } else {
                add_symbol(&symbol_table, label, inst_counter, 1);
            }
            after_label = skip_label_colon(line);
        }

        if (is_directive(after_label, directive)) {
            if (has_label && strcmp(directive, ".extern") && strcmp(directive, ".entry")) {
                /* mark as data */
                symbol_table->attributes = 2;
                symbol_table->address = data_counter;
            }
            if (!strcmp(directive, ".data")) handle_data_directive(after_label, line_num);
            else if (!strcmp(directive, ".string")) handle_string_directive(after_label, line_num);
            else if (!strcmp(directive, ".mat")) handle_mat_directive(after_label, line_num);
            else if (!strcmp(directive, ".extern")) {
                const char *p = strstr(after_label, ".extern") + 7;
                char extern_label[MAX_LABEL_LENGTH];
                i = 0;
                p = skip_whitespace(p);
                while (*p && !isspace(*p) && i < MAX_LABEL_LENGTH) extern_label[i++] = *p++;
                extern_label[i] = '\0';
                if (!validate_label(extern_label)) {
                    report_error("Invalid extern label", line_num);
                    error_flag = 1;
                } else {
                    add_symbol(&symbol_table, extern_label, 0, 4);
                }
            }
            continue;
        }

        inst_counter++; /* increase for instruction */
    }

    update_data_symbol_addresses(symbol_table);
    return error_flag;
}

