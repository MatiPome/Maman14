/* table.h - Symbol table header */
#ifndef TABLE_H
#define TABLE_H

#define CODE_ATTRIBUTE 1
#define DATA_ATTRIBUTE 2
#define EXTERN_ATTRIBUTE 4
#define ENTRY_ATTRIBUTE 8

#define MAX_LABEL_LENGTH 32

typedef struct symbol_node {
    char name[MAX_LABEL_LENGTH];
    int address;
    int attributes;
    struct symbol_node *next;
} symbol_node;

/* Global head of the symbol table */
extern symbol_node *symbol_table;

symbol_node *find_symbol(symbol_node *head, const char *name);
void add_symbol(symbol_node **head, const char *name, int address, int attributes);
void free_symbol_table(symbol_node *head);

#endif
