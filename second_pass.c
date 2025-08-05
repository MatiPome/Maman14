#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "globals.h"
#include "table.h"
#include "code_conversion.h"
#include "errors.h"
#include "util.h"
#include <ctype.h>


#define MAX_OPCODE_LENGTH 16
#define MAX_LABEL_LENGTH 32




/*
 * Returns 1 if the line is empty or a comment, 0 otherwise.
 */
int is_empty_or_comment(const char *line) {
    const char *p = line;
    while (*p && (*p == ' ' || *p == '\t')) p++;
    return (*p == ';' || *p == '\n' || *p == '\0');
}

/*
 * Attempts to extract a label from the beginning of a line.
 * If found, stores the label in 'label' and returns 1. Otherwise returns 0.
 */
int extract_label(const char *line, char *label) {
    int i = 0;
    const char *p = line;
    while (*p && *p != ':' && i < MAX_LABEL_LENGTH - 1) {
        label[i++] = *p++;
    }
    if (*p == ':') {
        label[i] = '\0';
        return 1;
    }
    label[0] = '\0';
    return 0;
}

/*
 * Returns a pointer to the first character after a label (if any).
 * Skips whitespace after the label.
 */
const char *skip_label(const char *line) {
    const char *p = line;
    while (*p && *p != ':') p++;
    if (*p == ':') p++;
    while (*p && (*p == ' ' || *p == '\t')) p++;
    return p;
}

/*
 * Extracts the opcode from a line (skipping label if present).
 * Returns 1 if opcode was found, 0 otherwise.
 */
int extract_opcode(const char *line, char *opcode) {
    int i = 0;
    const char *p = line;

    /* Skip whitespace */
    while (*p == ' ' || *p == '\t') p++;

    /* Skip label if present */
    if (*p && !isspace((unsigned char)*p)) {
        const char *label_end = strchr(p, ':');
        if (label_end && (label_end - p < MAX_LABEL_LENGTH)) {
            p = label_end + 1;
            while (*p == ' ' || *p == '\t') p++;
        }
    }

    /* Read opcode */
    while (*p && !isspace((unsigned char)*p) && *p != '\n' && i < MAX_OPCODE_LENGTH-1) {
        opcode[i++] = *p++;
    }
    opcode[i] = '\0';

    /* Do not treat directives, macro, or empty as opcode */
    if (opcode[0] == '\0' ||
        opcode[0] == '.'  ||
        strcmp(opcode, "macro") == 0 ||
        strcmp(opcode, "endmcro") == 0 ||
        opcode[0] == ';')
        return 0;
    return 1;
}

/* Extern global variables */
extern int inst_counter;
extern int data_counter;
extern int code_array[];
extern int data_memory[];
extern label_entry *symbol_table;
extern FILE *am_file;
extern int error_flag;  /* Error flag for this file */

FILE *ob_file = NULL;
FILE *ent_file = NULL;
FILE *ext_file = NULL;


/* Internal function prototypes */
void write_object_file(void);
void write_entry_file(void);
void process_instruction_line(const char *line, int line_num);

/*
 * Main function for the assembler's second pass.
 * Reads each line of the preprocessed (.am) file, encodes instructions,
 * and writes output files (.ob, .ent, .ext).
 */
void second_pass(const char *filename)
{
    char line[MAX_LINE_LENGTH];
    int line_num = 0;
    char ob_filename[FILENAME_MAX];
    char ent_filename[FILENAME_MAX];
    char ext_filename[FILENAME_MAX];

    inst_counter = 100;       /* Reset instruction counter for this file */
    error_flag = 0;  /* Reset error flag */

    /* Compose output filenames */
    strcpy(ob_filename, filename); strcat(ob_filename, ".ob");
    strcpy(ent_filename, filename); strcat(ent_filename, ".ent");
    strcpy(ext_filename, filename); strcat(ext_filename, ".ext");

    ob_file = fopen(ob_filename, "w");
    if (!ob_file) {
        perror("Error opening .ob file");
        return;
    }
    ent_file = fopen(ent_filename, "w");
    ext_file = fopen(ext_filename, "w");

    rewind(am_file);
    line[0] = '\0'; /* make sure buffer is clear */

    while (1) {
        char *fgets_result;
        fgets_result = fgets(line, MAX_LINE_LENGTH, am_file);

        if (!fgets_result) {
            /* Handle last line if not followed by a newline */
            if (line[0] != '\0') {
                line_num++;
                if (!is_empty_or_comment(line)) {
                    process_instruction_line(line, line_num);
                }
            }
            break;
        }

        line_num++;
        /* Remove trailing newline if present */
        {
            int len = strlen(line);
            if (len > 0 && line[len-1] == '\n') {
                line[len-1] = '\0';
            }
        }

        if (is_empty_or_comment(line)) continue;
        process_instruction_line(line, line_num);

        line[0] = '\0'; /* clear for possible partial final read */
    }


    /* Handle the last line if the file does not end with a newline */
    if (line[0] != '\0' && line[strlen(line)-1] != '\n') {
        /* Make sure we didn't already process it */
        if (!is_empty_or_comment(line)) {
            line_num++;
            process_instruction_line(line, line_num);
        }
    }


    /*
     * If any errors were detected during the pass,
     * delete the output files (no output should be produced).
     */
    if (error_flag) {
        if (ob_file) fclose(ob_file);
        remove(ob_filename);
        if (ent_file) fclose(ent_file);
        if (ext_file) fclose(ext_file);
        printf("----- Done: %s -----\n  ❌ No output file\n", filename);
        return;
    }

    write_object_file();
    write_entry_file();

    if (ent_file) fclose(ent_file);
    if (ext_file) fclose(ext_file);
    fclose(ob_file);
}

/*
 * Processes a single line during the second pass.
 * Skips labels and comments, and passes valid instructions to encode_instruction().
 */
void process_instruction_line(const char *line, int line_num)
{
    char label[MAX_LABEL_LENGTH];
    char opcode[MAX_OPCODE_LENGTH];
    const char *inst_line = line;

    /* If line starts with a label, skip it */
    if (extract_label(line, label)) {
        inst_line = skip_label(line);
    }

    /* Attempt to extract an opcode */
    if (!extract_opcode(inst_line, opcode)) {
        return; /* Not an instruction line */
    }

    if (opcode[0] == '\0') return;
    if (inst_line[0] == '\0' || inst_line[0] == ';' || inst_line[0] == '\n') return;



    /* Pass instruction line to the encoder for translation to machine code */
assemble_instruction(inst_line, opcode, line_num);
}

/*
 * Writes the object code (.ob) file.
 * The first line contains the code/data lengths.
 * Code words are written from address 100 to inst_counter-1.
 * Data words are written starting at inst_counter.
 */
void write_object_file(void)
{
    int i;
    int start = 100;
    int code_len = inst_counter - start + 1;

    /* Header in base 4 */
    print_base4(ob_file, code_len, 3);
    fprintf(ob_file, " ");
    print_base4(ob_file, data_counter, 2);
    fprintf(ob_file, "\n");

    /* Write code words in base 4 */
    for (i = start; i <= inst_counter; i++) {
        write_encoded_word(ob_file, code_array[i]);
    }

    /* Write data words in base 4 */
    for (i = 0; i < data_counter; i++) {
        write_encoded_word(ob_file, data_memory[i]);
    }
}




/*
 * Writes the entry symbols (if any) to the .ent file.
 */
void write_entry_file(void)
{
    label_entry *curr = symbol_table;
    while (curr) {
        if (curr->attributes & ENTRY_ATTRIBUTE) {
            fprintf(ent_file, "%s %04d\n", curr->name, curr->address);
        }
        curr = curr->next;
    }
}
