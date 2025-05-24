/// Linux system implementation.

#include "macros.hpp"
#include "time.hpp"

#if defined(__linux__) || defined(__APPLE__)
#include <sys/time.h>
#include <time.h>

using epiphany::EpiphanyNow;

long long EpiphanyNow::sec()
{
    struct timeval tv;
    gettimeofday(&tv, 0);
    time_t ret_sec = tv.tv_sec;
    if (tv.tv_usec > 500 * 1000)
        ret_sec++;
    return ret_sec;
}

long long EpiphanyNow::usec()
{
    struct timeval tv;
    gettimeofday(&tv, 0);
    return (long long)tv.tv_sec * 1000000 + tv.tv_usec;
}

long long EpiphanyNow::msec()
{
    struct timeval tv;
    gettimeofday(&tv, 0);
    long long ret_msec = 
    EpiphanyCast(long long, (tv.tv_sec * 1000 + tv.tv_usec / 1000));
    if (tv.tv_usec % 1000 > 500) 
        ret_msec++;
    return ret_msec;
}

#endif //! __linix__