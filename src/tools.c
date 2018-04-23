#include "tools.h"


long time_ms()
{
    struct timespec spec;
    long ms;
    time_t s;

    clock_gettime(CLOCK_REALTIME, &spec);
    s  = spec.tv_sec;
    ms = spec.tv_nsec / 1.0e6;

    return ms + (s * 1000);
}
