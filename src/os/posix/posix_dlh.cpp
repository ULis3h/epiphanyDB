#include "dlh.hpp"
#include <dlfcn.h>

using epiphany::os::EpiphanyDLH;

void* EpiphanyDLH::dl_open(const char* n, int f) {
    return dlopen(n, f);
}

int EpiphanyDLH::dl_close(void* handle) 
{
    return dlclose(handle);
}