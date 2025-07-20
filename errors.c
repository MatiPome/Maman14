#include <stdio.h>
#include "errors.h"

/*
 * Prints a formatted error message to stderr, including the line number.
 * Parameters:
 *   msg      - Description of the error (should be a string literal or valid pointer)
 *   line_num - The line number in the source file where the error occurred
 *
 * Usage:
 *   report_error("Unknown opcode", 7);
 */
void report_error(const char *msg, int line_num)
{
    fprintf(stderr, "Error (line %d): %s\n", line_num, msg);
}
