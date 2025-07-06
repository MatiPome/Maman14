#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *add_new_file(const char *filename, const char *extension) {
    char *dot = strrchr(filename, '.'); // Find last '.'
    int base_len = dot ? (dot - filename) : strlen(filename);

    // Allocate memory for new name + null terminator
    char *new_name = malloc(base_len + strlen(extension) + 1);
    if (!new_name) return NULL;

    strncpy(new_name, filename, base_len); // Copy base name (without .ext)
    new_name[base_len] = '\0';             // Null-terminate
    strcat(new_name, extension);           // Add new extension

    return new_name;
}
