#include <stdlib.h>
#include <cstring>

#include "macros.hpp"

#include "mem.h"
#include "mem_manager.hpp"

using epiphany::common::MemPool;

void* 
epiphany_malloc(size_t _size)
{
    return malloc(_size);
}

void
epiphany_free(void* _ptr)
{
    return free(_ptr);
}


int 
epiphany_strndup(const char* str, size_t len, void* retp)
{
    void* p = NULL;

    if ( str == NULL ) {
        *(void**)retp = NULL;
        return ( 0 );
    } 

    p = MemPool::instance.alloc(len + 1);
    
    EpiphanyCheckRtInt(p);

    memcpy(p, str, len);

    *(void **)retp = p;    

    return 0;
}

