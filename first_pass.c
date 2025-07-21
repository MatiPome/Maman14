#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "globals.h"
#include "table.h"
#include "errors.h"

/* --------------------------------------
 * Reserved words array
 * Contains all instruction mnemonics, directives, and register names.
 * Used to prevent using reserved words as labels.
 * -------------------------------------- */
static const char* reserved_words[] = {
    "mov", "cmp", "add", "sub", "not", "clr", "lea",
    "inc", "dec", "jmp", "bne", "red", "prn", "jsr", "rts", "stop",
    ".data", ".string", ".entry", ".extern",
    "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
    NULL
};

/* --------------------------------------
 * Returns 1 if 'word' is reserved (cannot be used as a label), 0 otherwise
 * -------------------------------------- */
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

/* --------------------------------------
 * Skips spaces/tabs at the start of a string.
 * Returns pointer to first non-whitespace character.
 * -------------------------------------- */
static const char* skip_whitespace(const char *str)
{
    while (*str && isspace((unsigned char)*str))
        str++;
    return str;
}

/* --------------------------------------
 * Checks if a label is valid:
 * - Must start with a letter.
 * - Only letters and digits allowed.
 * - Cannot be a reserved word.
 * - Must not exceed MAX_LABEL_LENGTH.
 * Returns 1 if valid, 0 if not.
 * -------------------------------------- */
static int validate_label(const char *label)
{
    int len = strlen(label), i;
    if (len == 0 || len > MAX_LABEL_LENGTH)
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

/* --------------------------------------
 * Checks if a label is present at start of line.
 * If so, copies the label to label_name and returns 1.
 * Otherwise returns 0.
 * -------------------------------------- */
int detectlabel(const char *line, char *label_name)
{
    const char *p = skip_whitespace(line);
    int i = 0;

    /* Copy up to ':' or whitespace (limit MAX_LABEL_LENGTH) */
    while (*p && *p != ':' && !isspace((unsigned char)*p) && i < MAX_LABEL_LENGTH) {
        label_name[i++] = *p++;
    }
    label_name[i] = '\0';

    /* If next char is ':', check if valid label */
    if (*p == ':') {
        if (validate_label(label_name))
            return 1;
        else
            return 0;
    }
    return 0;
}

/* --------------------------------------
 * Checks if a line starts with a directive (.data, .string, .extern, .entry).
 * Copies the directive name to directive_name if found.
 * Returns 1 if it's a directive, 0 otherwise.
 * -------------------------------------- */
int is_directive(const char *line, char *directive_name)
{
    const char *p = skip_whitespace(line);
    int i = 0;

    /* Directives always start with '.' */
    if (*p != '.') return 0;

    /* Copy directive word */
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

/* --------------------------------------
 * Parses and stores the numbers from a .data directive into data_memory.
 * Each number separated by commas.
 * Sets error_flag=1 if error occurs.
 * -------------------------------------- */
void handle_data_directive(const char *line, int line_num)
{
    const char *p = strstr(line, ".data");
    char num_str[20];
    int i, val;
    char *endptr;

    if (!p) return;
    p += strlen(".data");

    while (*p) {
        p = skip_whitespace(p);
        if (!*p) break;

        /* Parse number string */
        i = 0;
        while (*p && *p != ',' && !isspace((unsigned char)*p) && i < 19) {
            num_str[i++] = *p++;
        }
        num_str[i] = '\0';

        /* Error: comma with no number */
        if (i == 0) {
            report_error("Missing number in .data directive", line_num);
            error_flag = 1;
            return;
        }

        /* Convert string to integer */
        val = (int)strtol(num_str, &endptr, 10);
        if (*endptr != '\0') {
            report_error("Invalid number in .data directive", line_num);
            error_flag = 1;
            return;
        }

        /* Check memory overflow */
        if (data_counter >= MAX_DATA_SIZE) {
            report_error("Data memory overflow", line_num);
            error_flag = 1;
            return;
        }

        data_memory[data_counter++] = val;

        p = skip_whitespace(p);
        if (*p == ',') p++; /* skip comma */
    }
}

/* --------------------------------------
 * Parses a .string directive.
 * Each character between quotes is stored as an integer in data_memory.
 * Adds a null terminator (0) at the end.
 * Sets error_flag=1 if any error occurs.
 * -------------------------------------- */
void handle_string_directive(const char *line, int line_num)
{
    const char *start = strchr(line, '"');
    const char *end;

    if (!start) {
        report_error("Missing opening quote in .string directive", line_num);
        error_flag = 1;
        return;
    }
    start++; /* Skip opening quote */
    end = strchr(start, '"');
    if (!end) {
        report_error("Missing closing quote in .string directive", line_num);
        error_flag = 1;
        return;
    }

    while (start < end) {
        if (data_counter >= MAX_DATA_SIZE) {
            report_error("Data memory overflow", line_num);
            error_flag = 1;
            return;
        }
        data_memory[data_counter++] = (int)(*start++);
    }
    if (data_counter >= MAX_DATA_SIZE) {
        report_error("Data memory overflow", line_num);
        error_flag = 1;
        return;
    }
    data_memory[data_counter++] = 0; /* Add null terminator */
}

/* --------------------------------------
 * After first pass, adds inst_counter to the address of each data symbol in the symbol table.
 * Ensures .data locations appear after all code.
 * -------------------------------------- */
void update_data_symbol_addresses(label_entry *head)
{
    while (head) {
        if ((head->attributes & 2) != 0) /* 2 = data attribute */
            head->address += inst_counter;
        head = head->next;
    }
}

/* --------------------------------------
 * Helper: Skips over label and colon, returns pointer after label: and any spaces.
 * -------------------------------------- */
const char* skip_label_colon(const char *line) {
    const char *p = line;
    while (*p && *p != ':') p++;
    if (*p == ':') p++;
    while (*p && isspace((unsigned char)*p)) p++;
    return p;
}

/* --------------------------------------
 * first_pass - Scans the source file for:
 *   - Labels (and stores them in the symbol table)
 *   - Data and string directives (fills data_memory)
 *   - Extern directives (adds extern labels to the table)
 *   - Skips comments and empty lines
 *   - Updates inst_counter (Instruction Counter) for each line of code
 * After the pass, updates addresses of all data labels.
 * Returns 1 if there were errors, 0 otherwise.
 * -------------------------------------- */
int first_pass(FILE *fp)
{
    char line[MAX_LINE_LENGTH];
    char label[MAX_LABEL_LENGTH + 1];
    char directive[10];
    int line_num = 0;
    int ret, has_label, i;
    const char *after_label;

    /* Initialize inst_counter, data_counter, error flag */
    inst_counter = 100;
    data_counter = 0;
    error_flag = 0;

    while (fgets(line, MAX_LINE_LENGTH, fp)) {
        line_num++;

        /* Ignore comment or empty line */
        if (line[0] == ';' || line[0] == '\n')
            continue;

        /* Detect label (if any) and add to symbol table */
        has_label = detectlabel(line, label);
        after_label = line;
        if (has_label) {
            if (find_symbol(symbol_table, label)) {
                report_error("Duplicate label found", line_num);
                error_flag = 1;
            } else {
                /* Add as code by default (attribute 1) */
                add_symbol(&symbol_table, label, inst_counter, 1);
            }
            after_label = skip_label_colon(line);
        }

        /* Check if line is a directive (after label, if present) */
        ret = is_directive(after_label, directive);
        if (ret) {
            if (strcmp(directive, ".data") == 0)
                handle_data_directive(after_label, line_num);
            else if (strcmp(directive, ".string") == 0)
                handle_string_directive(after_label, line_num);
            else if (strcmp(directive, ".extern") == 0) {
                char extern_label[MAX_LABEL_LENGTH + 1];
                const char *p = after_label + strlen(".extern");
                p = skip_whitespace(p);
                i = 0;
                /* Parse extern label name */
                while (*p && !isspace((unsigned char)*p) && i < MAX_LABEL_LENGTH) {
                    extern_label[i++] = *p++;
                }
                extern_label[i] = '\0';

                /* Check for validity and duplicates */
                if (!validate_label(extern_label)) {
                    report_error("Invalid label name in .extern directive", line_num);
                    error_flag = 1;
                } else if (find_symbol(symbol_table, extern_label)) {
                    report_error("Label already defined, cannot be extern", line_num);
                    error_flag = 1;
                } else {
                    /* Attribute 4 = extern */
                    add_symbol(&symbol_table, extern_label, 0, 4);
                }
            }
            continue; /* Done handling directive, go to next line */
        }

        /* For instruction/code line, increment inst_counter */
        inst_counter++;
    }

    /* Update addresses of all data labels to appear after code */
    update_data_symbol_addresses(symbol_table);

    /* Return error flag status */
    return error_flag;
}
