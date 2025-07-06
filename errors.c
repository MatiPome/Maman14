#include <stdio.h>
#include "errors.h"

void report_error(const char *msg, int line_num)
{
    fprintf(stderr, "Error (line %d): %s\n", line_num, msg);
}
