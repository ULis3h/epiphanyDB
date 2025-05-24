#ifndef EPIPHANY_COMMON_NO_MEM_MANAGER_HPP_
#define EPIPHANY_COMMON_NO_MEM_MANAGER_HPP_

// includes.
#include <cstddef>

#include "mem.h"
#include "mem_manager.hpp"

namespace epiphany {
namespace common {

class NoMemManager : public IMemManagerPolicy {  
public:
    NoMemManager();
    virtual ~NoMemManager();

public: // overrides.
    virtual void* alloc(size_t _size);
    virtual void free(void* _ptr);
};

} /// namespace common
} /// namespace epiphany
#endif //! EPIPHANY_COMMON_NO_MEM_MANAGER_HPP_