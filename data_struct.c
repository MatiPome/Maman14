#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "data_struct.h"

/* Adds a new macro to the linked list.
 * Parameters:
 *   head - pointer to the pointer of the head node of the list
 *   name - the macro name string (assumed <= MAX_MACRO_NAME)
 * Returns:
 *   Pointer to the new node, or NULL on memory allocation failure.
 */
node *create_macro(node **head, const char *name) {
    node *new_node = malloc(sizeof(node));
    if (!new_node) return NULL; /* Memory allocation failed */

    strcpy(new_node->name, name);    /* Copy macro name */
    new_node->line_count = 0;        /* Start with 0 lines */
    new_node->next = *head;          /* Insert at list head */
    *head = new_node;
    return new_node;
}

/* Adds a line of text to a given macro's line array.
 * Parameters:
 *   macro - pointer to the macro node to add a line to
 *   line  - the text of the line to add (will be duplicated)
 * Note: Will not add more than MAX_MACRO_LINES.
 */
void add_line_to_macro(node *macro, const char *line) {
    if (macro->line_count >= MAX_MACRO_LINES) return; /* Too many lines, ignore */
    macro->lines[macro->line_count] = malloc(strlen(line) + 1); /* Allocate for new line */
    strcpy(macro->lines[macro->line_count], line);
    macro->line_count++;
}

/* Frees the entire macro linked list, including all allocated lines for each macro.
 * Parameters:
 *   head - pointer to the first node in the list (can be NULL)
 * After this call, all memory allocated by macros is freed.
 */
void free_macro_list(node *head) {
    node *temp;
    while (head) {
        int i;
        for (i = 0; i < head->line_count; i++) {
            free(head->lines[i]); /* Free each line */
        }
        temp = head;
        head = head->next;
        free(temp); /* Free macro node itself */
    }
}
