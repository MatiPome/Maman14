#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "data_struct.h"

node *add_to_macro_list(node **head, const char *name) {
    node *new_node = malloc(sizeof(node));
    if (!new_node) return NULL;

    strcpy(new_node->name, name);
    new_node->line_count = 0;
    new_node->next = *head;
    *head = new_node;
    return new_node;
}

void add_line_to_macro(node *macro, const char *line) {
    if (macro->line_count >= MAX_MACRO_LINES) return;

    macro->lines[macro->line_count] = malloc(strlen(line) + 1);
    strcpy(macro->lines[macro->line_count], line);
    macro->line_count++;
}

void free_macro_list(node *head) {
    node *temp;
    while (head) {
        int i;
        for (i = 0; i < head->line_count; i++) {
            free(head->lines[i]);
        }
        temp = head;
        head = head->next;
        free(temp);
    }
}

