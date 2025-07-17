/* table.c - Symbol table implementation */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "table.h"
#include "globals.h"
#include "table.h"


symbol_node *symbol_table = NULL;

symbol_node *find_symbol(symbol_node *head, const char *name) {
    while (head) {
        if (strcmp(head->name, name) == 0) {
            return head;
        }
        head = head->next;
    }
    return NULL;
}

void add_symbol(symbol_node **head, const char *name, int address, int attributes) {
    symbol_node *new_node = (symbol_node *)malloc(sizeof(symbol_node));
    if (!new_node) {
        fprintf(stderr, "Memory allocation error while adding symbol\n");
        exit(1);
    }
    strncpy(new_node->name, name, MAX_LABEL_LENGTH);
    new_node->name[MAX_LABEL_LENGTH - 1] = '\0';
    new_node->address = address;
    new_node->attributes = attributes;
    new_node->next = *head;
    *head = new_node;
}

void free_symbol_table(symbol_node *head) {
    symbol_node *tmp;
    while (head) {
        tmp = head;
        head = head->next;
        free(tmp);
    }
}
