
// includes.
#include "no_mem_manager.hpp"


using epiphany::common::NoMemManager;

NoMemManager::NoMemManager() 
{
}

NoMemManager::~NoMemManager()
{
}

void*
NoMemManager::alloc(size_t _size)
{
    return epiphany_malloc(_size);
}

void
NoMemManager::free(void* _ptr)
{
    epiphany_free(_ptr);
}