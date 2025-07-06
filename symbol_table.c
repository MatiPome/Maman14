#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbol_table.h"

/* Adds a symbol, error on duplicates */
void add_symbol(symbol_node **head, const char *name, int address, int attributes)
{
    symbol_node *curr = *head;
    while (curr) {
        if (strcmp(curr->name, name) == 0) {
            fprintf(stderr, "Error: Duplicate label '%s'\n", name);
            return;
        }
        curr = curr->next;
    }

    symbol_node *new_node = (symbol_node*)malloc(sizeof(symbol_node));
    if (!new_node) {
        fprintf(stderr, "Error: Memory allocation failed for symbol '%s'\n", name);
        return;
    }

    strncpy(new_node->name, name, MAX_LABEL_LEN);
    new_node->name[MAX_LABEL_LEN] = '\0';
    new_node->address = address;
    new_node->attributes = attributes;
    new_node->next = *head;
    *head = new_node;
}

/* Finds a symbol by name */
symbol_node* find_symbol(symbol_node *head, const char *name)
{
    while (head) {
        if (strcmp(head->name, name) == 0)
            return head;
        head = head->next;
    }
    return NULL;
}

/* Frees all symbols */
void free_symbol_table(symbol_node *head)
{
    symbol_node *temp;
    while (head) {
        temp = head;
        head = head->next;
        free(temp);
    }
}
