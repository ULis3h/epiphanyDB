#ifndef EPIPHANY_COMMON_MEM_MANAGER_HPP_
#define EPIPHANY_COMMON_MEM_MANAGER_HPP_

// includes.

namespace epiphany {
namespace common {

/**
 * @brief 内存管理策略类型.
 */
enum MemManagerPolicyType {
    EPIPHANY_MEM_MANAGER,
    NO_MEM_MANAGER,
};

/**
 * @brief 内存管理策略.
 */
class IMemManagerPolicy {
public:
    virtual ~IMemManagerPolicy() { };

public:
    virtual void* alloc(size_t _size) = 0;

    virtual void free(void* _ptr) = 0;
};

/**
 * @brief 内存池.
 */
class MemPool {
public:
    MemPool();
    ~MemPool();

public:
    void init(void* _base_addr, size_t _size,  MemManagerPolicyType _type =
              EPIPHANY_MEM_MANAGER);

    static MemPool instance;

    void* alloc(size_t _size);
    void free(void* _ptr);

private:
    IMemManagerPolicy* policy_;
};


} /// namespace common
} /// namespace epiphany

using epiphany::common::MemPool;

#undef MEM_POOL_GLOBAL

#endif //! EPIPHANY_COMMON_MEM_MANAGER_HPP_