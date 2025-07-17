#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "data_struct.h"
#include "util.h"
#include <ctype.h>

#define MAX_LINE_LENGTH 1000

int is_macro_start(const char *line, char *macro_name) {
    return sscanf(line, "macro %s", macro_name) == 1;
}

int is_macro_end(const char *line) {
    return strstr(line, "endmcro") != NULL;
}

node *find_macro(node *head, const char *name) {
    while (head) {
        if (strcmp(head->name, name) == 0)
            return head;
        head = head->next;
    }
    return NULL;
}

/* Helper: extract first word after optional label and whitespace */
void get_opcode_from_line(const char *line, char *opcode) {
    const char *p = line;
    int i = 0;
    opcode[0] = '\0';

    /* skip whitespace */
    while (*p == ' ' || *p == '\t') p++;

    /* skip label */
    if (*p && !isspace((unsigned char)*p)) {
        const char *colon = strchr(p, ':');
        if (colon && colon < p + 32) { /* reasonable label length */
            p = colon + 1;
            while (*p == ' ' || *p == '\t') p++;
        }
    }

    /* extract first word */
    while (*p && !isspace((unsigned char)*p) && *p != '\n' && i < 31)
        opcode[i++] = *p++;
    opcode[i] = '\0';
}

int mcro_exec(char *filename) {
    FILE *in_fp;
    char *out_filename;
    FILE *out_fp;
    char line[MAX_LINE_LENGTH];
    char macro_name[32];
    node *macro_list = NULL;
    node *current_macro = NULL;
    int in_macro = 0;

    in_fp = fopen(filename, "r");
    if (!in_fp) return 0;

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

    while (fgets(line, MAX_LINE_LENGTH, in_fp)) {
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

        /* Check if the first word after optional label is a macro call */
        {
            char opcode[32];
            node *macro;
            int i;

            get_opcode_from_line(line, opcode);
            macro = find_macro(macro_list, opcode);
            if (macro) {
                for (i = 0; i < macro->line_count; i++)
                    fprintf(out_fp, "%s", macro->lines[i]);
            } else {
                fprintf(out_fp, "%s", line);
            }
        }
    }

    fclose(in_fp);
    fclose(out_fp);
    free(out_filename);
    free_macro_list(macro_list);
    return 1;
}
