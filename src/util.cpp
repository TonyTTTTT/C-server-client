#include <stdarg.h>
#include <cstdio>
#include "util.h"

void print_debug(const char *format, ... ) {
    if (debug) {
        va_list arg_list;
        va_start(arg_list, format);
        vprintf(format, arg_list);
        va_end(arg_list);
    }
}