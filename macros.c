#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "data_struct.h"
#include "util.h"
#include <ctype.h>
#include "globals.h"

/*
 * Checks if a line is the start of a macro definition.
 * Handles optional label before the "mcro" keyword.
 * If found, stores the macro name in macro_name and returns 1.
 * Otherwise, returns 0.
 */
int is_macro_start(const char *line, char *macro_name) {
    const char *p = line;

    /* Skip label if present */
    if (strchr(p, ':') != NULL) {
        p = strchr(p, ':') + 1;
    }

    /* Skip whitespace */
    while (*p == ' ' || *p == '\t') {
        p++;
    }

    /* Check for 'mcro' keyword */
    if (strncmp(p, "mcro", 4) == 0) {
        p += 4;
        while (*p == ' ' || *p == '\t') {
            p++;
        }
        sscanf(p, "%31s", macro_name);
        return 1;
    }

    return 0;
}



/*
 * Checks if a line is the end of a macro definition.
 * Returns 1 if "endmcro" appears anywhere in the line, else 0.
 */
int is_macro_end(const char *line) {
    return strstr(line, "endmcro") != NULL;
}

/*
 * Looks for a macro by name in the macro list.
 * Returns pointer to node if found, else NULL.
 */
node *find_macro(node *head, const char *name) {
    while (head) {
        if (strcmp(head->name, name) == 0)
            return head;
        head = head->next;
    }
    return NULL;
}

/*
 * Extracts the first word from a line, skipping optional label and whitespace.
 * Used to check if a line is a macro call (possibly after a label).
 * - line: input line (may start with "LABEL: macrocall ...")
 * - opcode: output buffer for macro name (max 32 chars)
 */
void get_opcode_from_line(const char *line, char *opcode) {
    const char *p = line;
    int i = 0;
    opcode[0] = '\0';

    /* Skip whitespace at start of line */
    while (*p == ' ' || *p == '\t') p++;

    /* Skip label if present (look for ':') */
    if (*p && !isspace((unsigned char)*p)) {
        const char *colon = strchr(p, ':');
        if (colon && colon < p + 32) { /* Only if label is reasonably short */
            p = colon + 1;
            while (*p == ' ' || *p == '\t') p++;
        }
    }

    /* Extract first word (macro call or instruction) */
    while (*p && !isspace((unsigned char)*p) && *p != '\n' && i < 31)
        opcode[i++] = *p++;
    opcode[i] = '\0';
}

/*
 * Processes a file to expand macros.
 * For each macro definition, stores its lines in a linked list.
 * When a macro call is found, replaces it with its body.
 * Writes output to a .am file.
 * Returns 1 on success, 0 on failure.
 */
int mcro_exec(char *filename) {
    FILE *in_fp;         /* Input (original) file */
    char *out_filename;  /* Output (.am) filename */
    FILE *out_fp;        /* Output file */
    char line[MAX_LINE_LENGTH];
    char macro_name[32];
    node *macro_list = NULL;     /* Linked list of all macros found */
    node *current_macro = NULL;  /* Macro being currently defined */
    int in_macro = 0;            /* Flag: inside macro definition */
    int line_num = 0;            /* Track input line number for error reporting */
    int skip_macro = 0;          /* Flag: skip lines until endmcro after duplicate */

    in_fp = fopen(filename, "r");
    if (!in_fp) return 0;

    /* Create new .am output filename and file */
    out_filename = add_new_file(filename, ".am");
    if (!out_filename) {
        fclose(in_fp);
        return 0;
    }

    out_fp = fopen(out_filename, "w");
    if (!out_fp) {
        fclose(in_fp);
        free(out_filename);
        return 0;
    }

    /* Main loop: process each line */
    while (fgets(line, MAX_LINE_LENGTH, in_fp)) {

        if (strchr(line, '\n') == NULL && !feof(in_fp)) {
            fprintf(stderr, "Error (line %d): Line exceeds maximum allowed length of %d characters.\n", line_num, MAX_LINE_LENGTH);
            error_flag = 1;
            /* Optional: skip to end of long line */
            while (fgetc(in_fp) != '\n' && !feof(in_fp));
            continue;
        }

        line_num++;

        /* If skipping macro after duplicate, continue until endmcro */
        if (skip_macro) {
            if (is_macro_end(line)) {
                skip_macro = 0;
            }
            continue;
        }

        /* Macro definition start: begin recording macro lines */
        if (!in_macro && is_macro_start(line, macro_name)) {
            if (find_macro(macro_list, macro_name)) {
                fprintf(stderr, "Error (line %d): Duplicate macro name '%s'. Skipping this macro definition.\n", line_num, macro_name);
                skip_macro = 1;
                continue;
            }
            in_macro = 1;
            current_macro = create_macro(&macro_list, macro_name);
            continue;
        }

        /* Inside a macro: store each line, stop when end is found */
        if (in_macro) {
            if (is_macro_end(line)) {
                in_macro = 0;
                current_macro = NULL;
            } else {
                add_line_to_macro(current_macro, line);
            }
            continue;
        }

        /* If not a macro definition, check if the line calls a macro */
        {
            char opcode[32];
            node *macro;
            int i;

            get_opcode_from_line(line, opcode);
            macro = find_macro(macro_list, opcode);
            if (macro) {
                /* If it's a macro call, write macro's lines to output */
                for (i = 0; i < macro->line_count; i++)
                    fprintf(out_fp, "%s", macro->lines[i]);
            } else if (opcode[0] != '\0') {
                /* Not a macro: copy line as-is to output */
                fprintf(out_fp, "%s", line);
            }
        }
    }

    /* Cleanup: close files and free macro memory */
    fclose(in_fp);
    fclose(out_fp);
    free(out_filename);
    free_macro_list(macro_list);
    return 1;
}
