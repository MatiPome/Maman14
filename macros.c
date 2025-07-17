#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "data_struct.h"   // contains 'node' definition
#include "util.h"         // contains 'add_new_file'

#define MAX_LINE_LEN 1000

/*
 * Checks if a line is the start of a macro.
 * Returns the macro name if found.
 */
int is_macro_start(const char *line, char *macro_name) {
    return sscanf(line, "macro %s", macro_name) == 1;
}

/*
 * Checks if a line is the end of a macro.
 */
int is_macro_end(const char *line) {
    return strstr(line, "endmcro") != NULL;
}

/*
 * Finds a macro by name in the macro list.
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
 * Performs macro expansion:
 * - Reads .as file
 * - Builds .am file with expanded macros
 */
int mcro_exec(char *filename) {
    FILE *in_fp = fopen(filename, "r");
    if (!in_fp) return 0;

    char *out_filename = add_new_file(filename, ".am");
    FILE *out_fp = fopen(out_filename, "w");
    if (!out_fp) {
        fclose(in_fp);
        return 0;
    }

    char line[MAX_LINE_LEN];
    char macro_name[32];
    node *macro_list = NULL;
    node *current_macro = NULL;
    int in_macro = 0;

    while (fgets(line, MAX_LINE_LEN, in_fp)) {
        if (!in_macro && is_macro_start(line, macro_name)) {
            in_macro = 1;
            current_macro = add_to_macro_list(&macro_list, macro_name);
            continue;
        }

        if (in_macro) {
            if (is_macro_end(line)) {
                in_macro = 0;
                current_macro = NULL;
            } else {
                add_line_to_macro(current_macro, line);
            }
            continue;
        }

        // Check if the line is a macro call
        char copy[MAX_LINE_LEN];
        strcpy(copy, line);
        char *first_word = strtok(copy, " \t\n");

        node *macro = find_macro(macro_list, first_word);
        if (macro) {
            for (int i = 0; i < macro->line_count; i++)
                fprintf(out_fp, "%s", macro->lines[i]);
        } else {
            fprintf(out_fp, "%s", line);
        }
    }

    fclose(in_fp);
    fclose(out_fp);
    free(out_filename);
    free_macro_list(macro_list);
    return 1;
}