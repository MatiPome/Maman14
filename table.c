/* table.c - Symbol table implementation */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "table.h"
#include "globals.h"
#include "table.h"


label_entry *symbol_table = NULL;

label_entry *find_symbol(label_entry *head, const char *name) {
    while (head) {
        if (strcmp(head->name, name) == 0) {
            return head;
        }
        head = head->next;
    }
    return NULL;
}

void add_symbol(label_entry **head, const char *name, int address, int attributes) {
    label_entry *new_node = (label_entry *)malloc(sizeof(label_entry));
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

void free_symbol_table(label_entry *head) {
    label_entry *tmp;
    while (head) {
        tmp = head;
        head = head->next;
        free(tmp);
    }
}
