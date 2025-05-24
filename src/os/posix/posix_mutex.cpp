#include "mutex.hpp"

namespace epiphany {
namespace os {

EpiphanyMutex::EpiphanyMutex() : initialized_() { 
    int rc = pthread_mutex_init(&cs_, NULL);
    if (rc == 0) {
        initialized_ = true;
    }
}

EpiphanyMutex::~EpiphanyMutex()
{
    pthread_mutex_destroy(&cs_);
    initialized_ = false;
}

bool
EpiphanyMutex::is_initialized() 
{
    return initialized_;
}

void
EpiphanyMutex::lock()
{
    if (initialized_) {
        int rc = pthread_mutex_lock(&cs_);
    }
}

void 
EpiphanyMutex::unlock()
{
    if (initialized_) {
        int rc = pthread_mutex_unlock(&cs_);
    }
}

} /// epiphany.
} /// os.
