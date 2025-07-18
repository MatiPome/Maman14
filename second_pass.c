#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "globals.h"
#include "table.h"
#include "code_conversion.h"
#include "Errors.h"
#include "util.h"
#include <ctype.h>

#define MAX_LINE_LENGTH 100
#define MAX_OPCODE_LENGTH 16
#define MAX_LABEL_LENGTH 32

/* Helper: check if line is empty or comment */
int is_empty_or_comment(const char *line) {
    const char *p = line;
    while (*p && (*p == ' ' || *p == '\t')) p++;
    return (*p == ';' || *p == '\n' || *p == '\0');
}

/* Helper: extract label if present */
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

/* Helper: skip label */
const char *skip_label(const char *line) {
    const char *p = line;
    while (*p && *p != ':') p++;
    if (*p == ':') p++;
    while (*p && (*p == ' ' || *p == '\t')) p++;
    return p;
}

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

    /* If this is a directive, macro line, or empty, do not treat as opcode */
    if (opcode[0] == '\0' ||
        opcode[0] == '.'  ||
        strcmp(opcode, "macro") == 0 ||
        strcmp(opcode, "endmcro") == 0 ||
        opcode[0] == ';')
        return 0;
    return 1;
}

extern int IC;
extern int DC;
extern int instruction_memory[];
extern int data_memory[];
extern symbol_node *symbol_table;
extern FILE *am_file;
extern int had_error;    /* ← חובה להוסיף */

static FILE *ob_file;
static FILE *ent_file;
static FILE *ext_file;

void write_object_file(void);
void write_entry_file(void);
void process_instruction_line(const char *line, int line_num);

void second_pass(const char *filename)
{
    char line[MAX_LINE_LENGTH];
    int line_num = 0;
    char ob_filename[FILENAME_MAX];
    char ent_filename[FILENAME_MAX];
    char ext_filename[FILENAME_MAX];

    IC = 100;
    had_error = 0;   /* ← אתחול הדגל בתחילת כל קובץ */

    strcpy(ob_filename, filename);
    strcat(ob_filename, ".ob");
    strcpy(ent_filename, filename);
    strcat(ent_filename, ".ent");
    strcpy(ext_filename, filename);
    strcat(ext_filename, ".ext");

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

    /* בדוק אם הייתה שגיאה */
    if (had_error) {
        if (ob_file) fclose(ob_file);
        remove(ob_filename);  /* מוחק את הקובץ ob אם נוצרה שגיאה */
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

/* Process a line from the .am file during the second pass */
void process_instruction_line(const char *line, int line_num)
{
    char label[MAX_LABEL_LENGTH];
    char opcode[MAX_OPCODE_LENGTH];
    const char *inst_line = line;

    /* Skip label if present */
    if (extract_label(line, label)) {
        inst_line = skip_label(line);
    }

    /* Attempt to extract an opcode */
    if (!extract_opcode(inst_line, opcode)) {
        /* Not an instruction line, skip */
        return;
    }

    /* Extra defensive check: skip if opcode is empty */
    if (opcode[0] == '\0') {
        return;
    }

    /* Extra: skip if line is a comment or empty */
    if (inst_line[0] == '\0' || inst_line[0] == ';' || inst_line[0] == '\n') {
        return;
    }

    printf("DEBUG: line_num=%d | opcode='%s' | inst_line='%s'\n", line_num, opcode, inst_line);

    /* Call encoder only if this is a real instruction! */
    encode_instruction(inst_line, opcode, line_num, ob_file, ext_file);
}

void write_object_file(void)
{
    int i;
    int start = 100; /* MMN14 convention */
    fprintf(ob_file, "%d %d\n", IC - start, DC);
    for (i = start; i < IC; i++) {
        write_encoded_word(ob_file, instruction_memory[i]);
        printf("instruction_memory[%d]=%04X\n", i, instruction_memory[i]); /* רק הדפסה למסך אם תרצה */
    }
    for (i = 0; i < DC; i++) {
        write_encoded_word(ob_file, data_memory[i]);
    }
}

void write_entry_file(void)
{
    symbol_node *curr = symbol_table;
    while (curr) {
        if (curr->attributes & ENTRY_ATTRIBUTE) {
            fprintf(ent_file, "%s %04d\n", curr->name, curr->address);
        }
        curr = curr->next;
    }
}
