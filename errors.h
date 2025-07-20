#ifndef ERRORS_H
#define ERRORS_H

/*
 * Prints a formatted error message with line number to stderr.
 * Intended to standardize error output throughout the assembler project.
 *
 * Parameters:
 *   msg      - Null-terminated string describing the error
 *   line_num - Line number in the input source file (for context)
 */
void report_error(const char *msg, int line_num);

#endif /* ERRORS_H */
