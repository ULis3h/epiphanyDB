#ifndef EPIPHANY_OS_TIME_HPP_
#define EPIPHANY_OS_TIME_HPP_

#include "config.hpp"

namespace epiphany {

/**
 * @brief 获取当前时间类.
 */
class EpiphanyNow
{
public:
    /// @brief get second.
    /// @return 
    static long long sec();

    /// @brief
    /// @return 
    static long long usec();

    /// @brief 
    /// @return 
    static long long msec();
};

}

#if defined(ENABLE_TIMING)
#define TimeStart(label)\
    long long start_##label = \
    (TIMIMG_PER == TIMING_PER_MSEC) ? epiphany::EpiphanyNow::msec() : epiphany::EpiphanyNow::usec();

#define TimeEnd(label)\
    long long end_##label = \
    (TIMIMG_PER == TIMING_PER_MSEC) ? epiphany::EpiphanyNow::msec() : epiphany::EpiphanyNow::usec();\
    double elapsed_##label = (double)(end_##label - start_##label);\
    LogInfo("elapsed: %f%ss", elapsed_##label, \
    (TIMIMG_PER == TIMING_PER_MSEC) ? "m" : "u");
    
#else
#define TimeStart(label)
#define TimeEnd(label)
#endif //! ENABLE_TIMING


#endif //! EPIPHANY_OS_TIME_HPP_