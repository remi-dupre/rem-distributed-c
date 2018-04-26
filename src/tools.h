/**
 * Some general functions that do not have any place elsewhere.
 */
#ifndef TOOLS
#define TOOLS

#ifndef _POSIX_C_SOURCE
    #define _POSIX_C_SOURCE 200809L
#endif

#include <inttypes.h>
#include <time.h>


/**
 * Return the timestamp, in milliseconds.
 */
long time_ms();

#endif
