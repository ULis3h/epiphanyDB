#ifndef EPIPHANY_OS_THREAD_HPP_
#define EPIPHANY_OS_THREAD_HPP_

#include "mutex.hpp"

using epiphany::os::EpiphanyMutex;

class EpiphanyThread {
public:

    /// @brief 
    /// @param p 
    /// @return 
    static int thread_once(void (*p)(void));

    /// @brief 
    void spin_init();

    /// @brief 
    void spin_lock();

    /// @brief 
    void spin_unlock();
private:
    EpiphanyMutex spin_lock_;
};


#endif //! EPIPHANY_OS_THREAD_HPP_