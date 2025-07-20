/* table.h - Symbol table header */
#ifndef TABLE_H
#define TABLE_H

/* Symbol attributes: each attribute uses a unique bit flag */
#define CODE_ATTRIBUTE   1   /* Symbol belongs to code section */
#define DATA_ATTRIBUTE   2   /* Symbol belongs to data section */
#define EXTERN_ATTRIBUTE 4   /* Symbol is external */
#define ENTRY_ATTRIBUTE  8   /* Symbol is entry */

#define MAX_LABEL_LENGTH 32  /* Maximum length for label names */

/*
 * Node structure for the symbol table linked list.
 * - name:        Symbol/label name.
 * - address:     Memory address associated with the symbol.
 * - attributes:  Bit flags indicating symbol type (see above).
 * - next:        Pointer to the next symbol in the list.
 */
typedef struct label_entry {
    char name[MAX_LABEL_LENGTH];
    int address;
    int attributes;
    struct label_entry *next;
} label_entry;

/* Global pointer to the head of the symbol table. */
extern label_entry *symbol_table;

/* Search for a symbol by name. Returns pointer to node, or NULL if not found. */
label_entry *find_symbol(label_entry *head, const char *name);

/* Add a symbol to the symbol table. Inserts at the head of the list. */
void add_symbol(label_entry **head, const char *name, int address, int attributes);

/* Free all memory used by the symbol table. */
void free_symbol_table(label_entry *head);

#endif /* TABLE_H */
