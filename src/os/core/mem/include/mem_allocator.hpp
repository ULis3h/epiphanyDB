#ifndef CORE_MEM_MEM_ALLOCATOR_HPP_
#define CORE_MEM_MEM_ALLOCATOR_HPP_

#include <stdlib.h>

class IMemAllocator 
{
public:
    virtual void* allocate(size_t size) = 0;
    virtual void dellocate(void* ptr) = 0;
    virtual ~IMemAllocator() {}
};

#endif //! CORE_MEM_MEM_ALLOCATOR_HPP_