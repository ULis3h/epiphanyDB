#ifndef EPIPHANY_OS_MUTEX_HPP_
#define EPIPHANY_OS_MUTEX_HPP_

namespace epiphany {
namespace os {

#if defined (EPIPHANY_ON_POSIX)
#include <pthread.h>
#define EpiphanyMutexT pthread_mutex_t
#endif


/**
 * 锁类型的定义, 只根据平台进行区分, 由于一个时间只能运行在一个平台上, 锁的定义不使用抽象类型
 * 进行定义, 而使用宏进行控制.
 */
class EpiphanyMutex 
{
public:
    EpiphanyMutex();
    ~EpiphanyMutex();

    /// @brief 检查是否已经完成初始化
    /// @return 
    bool is_initialized();

    /// @brief 加锁.
    void lock();

    /// @brief 释放锁.
    void unlock();
private:
    bool initialized_;
    EpiphanyMutexT cs_;
};


} /// epiphany
} /// os
#endif //! EPIPHANY_OS_MUTEX_HPP_