#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "globals.h"
#include "table.h"  /* for symbol_node, symbol_table, free_symbol_table */
#include "util.h"   /* for add_new_file */

int mcro_exec(char *filename);
int first_pass(FILE *fp);
void second_pass(const char *filename);
void free_symbol_table(symbol_node *head);

FILE *am_file = NULL;

void cleanup_all(void) {
    free_symbol_table(symbol_table);
}

int main(int argc, char *argv[]) {
    int i;

    if (argc < 2) {
        printf("Usage: %s <source_file1> [source_file2 ...]\n", argv[0]);
        return 1;
    }

    for (i = 1; i < argc; i++) {
        char *src_filename;
        char *am_filename;

        src_filename = argv[i];
        am_filename = NULL;

        printf("----- Assembling: %s -----\n", src_filename);

        if (!mcro_exec(src_filename)) {
            printf("Macro expansion failed for %s\n", src_filename);
            continue;
        }
        am_filename = add_new_file(src_filename, ".am");
        if (!am_filename) {
            printf("Failed to allocate filename for %s\n", src_filename);
            continue;
        }

        am_file = fopen(am_filename, "r");
        if (!am_file) {
            printf("Failed to open .am file %s\n", am_filename);
            free(am_filename);
            continue;
        }

        if (first_pass(am_file) != 0) {
            printf("First pass failed for %s\n", src_filename);
            fclose(am_file);
            free(am_filename);
            cleanup_all();
            continue;
        }
        rewind(am_file);

        second_pass(src_filename);

        fclose(am_file);
        free(am_filename);

        cleanup_all();

        printf("----- Done: %s -----\n", src_filename);
    }

    return 0;
}
