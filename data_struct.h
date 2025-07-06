#ifndef DATA_STRCT_H
#define DATA_STRCT_H

#define MAX_MACRO_LINES 100

typedef struct node {
    char name[32];                  // Macro name
    char *lines[MAX_MACRO_LINES];  // Lines inside macro
    int line_count;                // Number of lines in macro
    struct node *next;             // Pointer to next macro
} node;

// Adds a new macro node to the list
node *add_to_macro_list(node **head, const char *name);

// Adds a line to the current macro node
void add_line_to_macro(node *macro, const char *line);

// Frees the macro list and all allocated lines
void free_macro_list(node *head);

#endif
