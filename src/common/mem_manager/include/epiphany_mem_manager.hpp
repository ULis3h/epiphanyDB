#ifndef EPIPHANY_COMMON_EPIPHANY_MEM_MANAGER_HPP_
#define EPIPHANY_COMMON_EPIPHANY_MEM_MANAGER_HPP_

// includes.
#include <cstddef>

#include "mutex.hpp"
#include "mem_manager.hpp"

namespace epiphany {
namespace common {

using epiphany::os::EpiphanyMutex;


class EpiphanyMemManager : public IMemManagerPolicy {  
public:
    EpiphanyMemManager(void* _base_addr, size_t _size);
    virtual ~EpiphanyMemManager();

public: // overrides.
    virtual void* alloc(size_t _size);
    virtual void free(void* _ptr);

private:
    struct MemBlock
    {
        /** 内存块大小. */
        size_t size_;
        /** 指向下一个内存块. */
        MemBlock *next_;
        /** 是否是空闲块. */
        bool is_free_;
    };

private:

    /// @brief 分割内存块.
    /// @param _block [in] 内存块.
    /// @param _size [in] 内存块大小.
    void split_block(MemBlock* _block, size_t _size);

    /// @brief 合并内存块.  
    void merge_blocks();

private:
    char *base_addr_;     ///< 内存池基地址.
    size_t size_;         ///< 内存池大小.
    MemBlock *free_list_; ///< 空闲内存块链表.

    EpiphanyMutex mutex_; ///< 互斥锁, 暂时使用 void* 来表示.
};

} /// namespace common
} /// namespace epiphany
#endif //! EPIPHANY_COMMON_EPIPHANY_MEM_MANAGER_HPP_