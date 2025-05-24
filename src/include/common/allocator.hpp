#ifndef EPIPHANY_COMMON_ALLOCATOR_HPP_
#define EPIPHANY_COMMON_ALLOCATOR_HPP_

#include <new>
#include <stddef.h>
#include <climits>

#include "mem_manager.hpp"

namespace epiphany {
namespace common {

template <class T>
inline T* _allocate(ptrdiff_t size, T*) {
    // set_new_handle(0);
    T* tmp = (T*) (MemPool::instance.alloc(sizeof(T)));
    
    // TODO: 容错处理.
    return tmp;
}

template <class T>
inline void _deallocate(T* buffer) {
    MemPool::instance.free(buffer);
}

template <class T1, class T2>
inline void _construct(T1* p, const T2 &value) {
    new (p) T1(value);
}

template <class T>
inline void _destroy(T* ptr) {
    ptr->~T();
}

template <class T>
class Allocator : public std::allocator<T> {
public:
    typedef T value_type;
    typedef T* pointer;
    typedef const T* const_pointer;
    typedef T& reference;
    typedef const T& const_reference;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;

    // rebind allocator of type U
    template <class U>
    class rebind {
    public:
        typedef Allocator<U> other;
    };

    pointer allocate (size_type n, const void* hint = 0) {
        return _allocate((difference_type)n, (pointer)0);
    }

    void deallocate(pointer p, size_type n) {
        _deallocate(p);
    }

    void construct(pointer p, const T&  value) {
        _construct(p, value);
    }

    void destroy(pointer p) {
        _destroy(p);
    }

    pointer address(reference x) {
        return (pointer)&x;
    }

    const_pointer const_address(const_reference x) {
        return (const_pointer)&x;
    }

    size_type max_size() const {
        return size_type(UINT_MAX / sizeof(T));
    }
};

} /// namespace common
} /// namespace epiphany

#endif //! EPIPHANY_COMMON_ALLOCATOR_HPP_