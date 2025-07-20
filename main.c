#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "globals.h"
#include "table.h"  /* For symbol_node, symbol_table, free_symbol_table */
#include "util.h"   /* For add_new_file */

/* Forward declarations for major steps of the assembler */
int mcro_exec(char *filename);                /* Macro expansion processor */
int first_pass(FILE *fp);                     /* First assembler pass */
void second_pass(const char *filename);       /* Second assembler pass (generates output) */
void free_symbol_table(label_entry *head);    /* Frees memory for symbol table */

/* File pointer to the .am (preprocessed) file, used globally for second pass */
FILE *am_file = NULL;

/*
 * Releases any dynamically allocated resources (mainly, the symbol table).
 * This is called after assembling each file to prevent memory leaks.
 */
void cleanup_all(void) {
    free_symbol_table(symbol_table);
}

/*
 * Main entry point for the assembler.
 * Processes each input file: expands macros, runs both passes, writes output.
 */
int main(int argc, char *argv[]) {
    int i;

    /* Require at least one input file */
    if (argc < 2) {
        printf("Usage: %s <source_file1> [source_file2 ...]\n", argv[0]);
        return 1;
    }

    /* Process each file given on the command line */
    for (i = 1; i < argc; i++) {
        char *src_filename;
        char *am_filename;

        src_filename = argv[i];
        am_filename = NULL;

        printf("----- Assembling: %s -----\n", src_filename);

        /* Step 1: Run macro preprocessor, outputting .am file */
        if (!mcro_exec(src_filename)) {
            printf("Macro expansion failed for %s\n", src_filename);
            continue;
        }

        /* Step 2: Compute new .am filename and open for reading */
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

        /* Step 3: First pass (builds symbol table and checks code/data) */
        if (first_pass(am_file) != 0) {
            printf("First pass failed for %s\n", src_filename);
            fclose(am_file);
            free(am_filename);
            cleanup_all();
            continue;
        }
        rewind(am_file); /* Rewind for second pass to reread from beginning */

        /* Step 4: Second pass (generate output files: .ob, .ent, .ext) */
        second_pass(src_filename);

        /* Cleanup: close file, free memory, reset tables */
        fclose(am_file);
        free(am_filename);

        cleanup_all();

        printf("----- Done: %s -----\n", src_filename);
    }

    return 0;
}
