#ifndef DATA_STRCT_H
#define DATA_STRCT_H

#define MAX_MACRO_LINES 100

typedef struct node {
    char name[32];                  /* Macro name */
    char *lines[MAX_MACRO_LINES];   /* Lines inside macro */
    int line_count;                 /* Number of lines in macro */
    struct node *next;              /* Pointer to next macro */
} node;

node *add_to_macro_list(node **head, const char *name);
void add_line_to_macro(node *macro, const char *line);
void free_macro_list(node *head);

#endif
