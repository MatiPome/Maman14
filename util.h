#ifndef UTIL_H
#define UTIL_H

/*
 * add_new_file
 * Returns a new string with the file extension replaced.
 * Example: add_new_file("foo.as", ".am") → "foo.am"
 * The returned string is dynamically allocated and must be freed by the caller.
 *
 * Parameters:
 *   filename   - original file name (may contain an extension)
 *   extension  - new extension, including dot (e.g., ".am")
 *
 * Returns:
 *   Pointer to new string, or NULL if memory allocation fails.
 */
char *add_new_file(const char *filename, const char *extension);

#endif /* UTIL_H */
