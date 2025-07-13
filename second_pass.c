/* second_pass.c */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "globals.h"
#include "table.h"
#include "code_conversion.h"
#include "Errors.h"
#include "util.h"

extern int IC;
extern int DC;
extern int instruction_memory[];
extern int data_memory[];
extern symbol_node *symbol_table;
extern FILE *am_file;

static FILE *ob_file;
static FILE *ent_file;
static FILE *ext_file;

/* Forward declarations */
void write_object_file();
void write_entry_file();
void process_instruction_line(const char *line, int line_num);

void second_pass(const char *filename)
{
    char line[MAX_LINE_LENGTH];
    int line_num = 0;
    char ob_filename[FILENAME_MAX];
    char ent_filename[FILENAME_MAX];
    char ext_filename[FILENAME_MAX];

    /* Prepare output filenames */
    strcpy(ob_filename, filename);
    strcat(ob_filename, ".ob");
    strcpy(ent_filename, filename);
    strcat(ent_filename, ".ent");
    strcpy(ext_filename, filename);
    strcat(ext_filename, ".ext");

    /* Open output files */
    ob_file = fopen(ob_filename, "w");
    if (!ob_file) {
        perror("Error opening .ob file");
        return;
    }

    ent_file = fopen(ent_filename, "w");
    ext_file = fopen(ext_filename, "w");

    rewind(am_file);
    while (fgets(line, MAX_LINE_LENGTH, am_file)) {
        line_num++;
        if (is_empty_or_comment(line)) continue;
        process_instruction_line(line, line_num);
    }

    write_object_file();
    write_entry_file();

    if (ent_file) fclose(ent_file);
    if (ext_file) fclose(ext_file);
    fclose(ob_file);
}

/* Process instruction lines only (assume directives were handled in first pass) */
void process_instruction_line(const char *line, int line_num)
{
    char label[MAX_LABEL_LENGTH];
    char opcode[MAX_OPCODE_LENGTH];

    if (extract_label(line, label)) {
        line = skip_label(line); /* Skip label part */
    }

    if (!extract_opcode(line, opcode)) {
        report_error("Invalid opcode", line_num);
        return;
    }

    /* Handle instructions like mov, add, lea, etc. */
    encode_instruction(line, opcode, line_num, ob_file, ext_file);
}

/* Write .ob file */
void write_object_file()
{
    int i;
    fprintf(ob_file, "%d %d\n", IC, DC);
    for (i = 0; i < IC; i++) {
        write_encoded_word(ob_file, instruction_memory[i]);
    }
    for (i = 0; i < DC; i++) {
        write_encoded_word(ob_file, data_memory[i]);
    }
}

/* Write .ent file */
void write_entry_file()
{
    symbol_node *curr = symbol_table;
    while (curr) {
        if (curr->attributes & ENTRY_ATTRIBUTE) {
            fprintf(ent_file, "%s %04d\n", curr->name, curr->address);
        }
        curr = curr->next;
    }
}
