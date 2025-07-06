#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#define MAX_LABEL_LEN 31

/* Symbol table node structure */
typedef struct symbol_node {
    char name[MAX_LABEL_LEN + 1];
    int address;          /* IC or DC */
    int attributes;       /* Flags: code=1, data=2, extern=4, entry=8 */
    struct symbol_node *next;
} symbol_node;

/* Adds a new symbol to the symbol table */
void add_symbol(symbol_node **head, const char *name, int address, int attributes);

/* Finds a symbol by name */
symbol_node* find_symbol(symbol_node *head, const char *name);

/* Frees the entire symbol table */
void free_symbol_table(symbol_node *head);

#endif /* SYMBOL_TABLE_H */
