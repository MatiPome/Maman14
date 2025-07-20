#ifndef DATA_STRCT_H
#define DATA_STRCT_H

/* Maximum lines that a macro can contain */
#define MAX_MACRO_LINES 100

/*
 * Macro list node structure:
 * - Represents a macro in the program, storing its name and the lines that define it.
 * - Linked list: Each macro points to the next defined macro (if any).
 */
typedef struct node {
    char name[32];                   /* Macro name (null-terminated string) */
    char *lines[MAX_MACRO_LINES];    /* Pointers to the lines inside the macro */
    int line_count;                  /* Current number of lines stored */
    struct node *next;               /* Pointer to the next macro node */
} node;

/*
 * Adds a new macro node to the macro list.
 * Parameters:
 *   head - pointer to pointer to the macro list head
 *   name - macro name to insert (copied into node)
 * Returns:
 *   Pointer to the new node, or NULL on allocation failure.
 */
node *create_macro(node **head, const char *name);

/*
 * Adds a line to a macro's storage.
 * Parameters:
 *   macro - pointer to the macro node to modify
 *   line  - the line string to add (will be duplicated)
 * Does nothing if line limit reached.
 */
void add_line_to_macro(node *macro, const char *line);

/*
 * Frees the entire macro list, including all stored lines.
 * Parameters:
 *   head - pointer to the head of the list
 */
void free_macro_list(node *head);

#endif /* DATA_STRCT_H */
