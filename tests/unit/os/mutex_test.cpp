#include "mutex.hpp"

/** run on windows. */
#ifdef _WIN32


#endif

/** run on linux. */
#ifdef __linux__


#endif 

#ifdef __APPLE__
#include <pthread>



int os_mutex_test() {
    pthread_t  thread;
    pthread_attr_t thread_attr;

    return 0;
}

#endif