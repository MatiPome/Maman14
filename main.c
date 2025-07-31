#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "globals.h"
#include "table.h"   /* For label_entry and symbol_table */
#include "util.h"    /* For add_new_file */

/* Forward declarations */
int mcro_exec(char *filename);              /* Macro expansion stage */
int first_pass(FILE *fp);                   /* First pass of assembler */
void second_pass(const char *filename);     /* Second pass (generate .ob, .ent, .ext) */
void free_symbol_table(label_entry *head);  /* Free memory used by the symbol table */

/* Global variable shared with second pass */
FILE *am_file = NULL;

/* Cleanup any global state between files */
void cleanup_all(void) {
    free_symbol_table(symbol_table);
}

/* Main assembler function */
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

        /* Step 1: Macro processing */
        if (!mcro_exec(src_filename)) {
            printf("❌ Macro expansion failed for %s\n", src_filename);
            continue;
        } else {
            printf("✅ Macro expansion OK for %s\n", src_filename);
        }

        /* Step 2: Get .am filename */
        am_filename = add_new_file(src_filename, ".am");
        if (!am_filename) {
            printf("❌ Failed to generate .am filename for %s\n", src_filename);
            continue;
        }

        am_file = fopen(am_filename, "r");
        if (!am_file) {
            printf("❌ Failed to open .am file %s\n", am_filename);
            free(am_filename);
            continue;
        }

        /* Step 3: First pass */
        if (first_pass(am_file) != 0) {
            printf("❌ First pass failed for %s\n", src_filename);
            fclose(am_file);
            free(am_filename);
            cleanup_all();
            continue;
        }

        printf("DEBUG after first_pass: data_counter = %d\n", data_counter);

        rewind(am_file); /* For second pass reread */

        /* Step 4: Second pass */
        second_pass(src_filename);

        /* Cleanup after file */
        fclose(am_file);
        free(am_filename);
        cleanup_all();

        printf("----- Done: %s -----\n", src_filename);
    }

    return 0;
}
