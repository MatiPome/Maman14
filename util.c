/* util.c - Utility function implementations */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "globals.h"

/*
 * add_new_file
 * Creates a new file name by replacing the extension of `filename` with `extension`.
 * For example: add_new_file("source.as", ".am") returns "source.am".
 * The returned string must be freed by the caller.
 *
 * Parameters:
 *   filename   - Original file name (may include extension)
 *   extension  - New extension (must include the dot, e.g., ".am")
 *
 * Returns:
 *   Pointer to a newly allocated string with the new name, or NULL on allocation failure.
 */
char *add_new_file(const char *filename, const char *extension) {
    char *dot = strrchr(filename, '.');  /* Find last '.' in filename, or NULL if none */
    int base_len;

    /* Determine length of the base name (excluding existing extension) */
    if (dot)
        base_len = (int)(dot - filename);
    else
        base_len = (int)strlen(filename);

    /* Allocate memory for: base + extension + null terminator */
    /* (C90: malloc returns void*, cast not needed in C90) */
    {
        char *new_name = malloc(base_len + strlen(extension) + 1);
        if (!new_name) return NULL;

        /* Copy the base part of the filename */
        strncpy(new_name, filename, base_len);
        new_name[base_len] = '\0';    /* Ensure null-termination */
        strcat(new_name, extension);  /* Add new extension */

        return new_name;
    }
}
