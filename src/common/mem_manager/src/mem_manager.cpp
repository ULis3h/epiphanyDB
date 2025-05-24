#include "no_mem_manager.hpp"
#include "epiphany_mem_manager.hpp"

namespace epiphany {
namespace common {

MemPool::MemPool()
    : policy_(NULL)
{
}

MemPool::~MemPool() {
    delete policy_;
}

void
MemPool::init(void* _base_addr, size_t _size,  MemManagerPolicyType _type)
{
    /** Use memory pool for management. */
    if (_type == EPIPHANY_MEM_MANAGER)
    {
        policy_ = new EpiphanyMemManager(_base_addr, _size);
    }
}

void* MemPool::alloc(size_t _size) {
    return (policy_->alloc(_size));
}

void MemPool::free(void* _ptr) {
    policy_->free(_ptr);
}

} /// namespace common
} /// namespace epiphany