#include <pthread.h>
#include "thread.hpp"


int 
EpiphanyThread::thread_once(void (*p)(void) ) {
    static pthread_once_t once_control = PTHREAD_ONCE_INIT; 
    return pthread_once(&once_control, p);
}

void 
EpiphanyThread::spin_init() {
   /** No need to initialize, already called during creation. */ 
}

void 
EpiphanyThread::spin_lock() {
    spin_lock_.lock();
}

void 
EpiphanyThread::spin_unlock() {
    spin_lock_.unlock();
}