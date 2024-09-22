#ifndef UTIL_H
#define UTIL_H

#include <stdbool.h>

#define serverPort 48763
#define serverIP "127.0.0.1"
#define MAX_PATH_LEN 250
#define SEP_LINE "========================================================================================\n"
#define ACK_MSG " complete couting!"

// if debug == true, 
// (1) Serial counting occurrence will also run and compare with parallel regarding time consuming
// (2) More information will be print
static bool debug = false;

void print_debug(const char *format, ... );

#endif