#ifndef CORE_MEM_TLSF_HPP_
#define CORE_MEM_TLSF_HPP_

#include "mem_allocator.hpp"

typedef struct block_header_t;

/**
 * @brief TLSF算法.
 * 
 * 两级结构:
 *  第一级(FL-Free List): TLSF将整个内存空间的空闲块按照大小范围划分为多个组,这构成了第一级
 *      结构,这些组的划分是基于内存块大小的二进制对数,例如,内存块大小范围在2^n到2^(n+1) - 1
 *      字节的空闲块被划分到一组中.第一级结构实际上是一个数组,每个元素指向第二级中对应的空闲块
 *      链表.
 *  第二级(SL-Sub List): 在每个第一级组中,有一个或多个空闲块链表(取决于具体实现).这些链表中
 *      的节点代表空闲的内存块,每个内存块节点包含了该内存块的大小\起始地址等信息.这种两级结构
 *      使得在分配内存时,可以快速定位到合适大小范围的空闲块链表,减少了搜索时间.
 * 
 * 时间复杂度: O(1).
 * 
 * 注意: TLSF在32位系统时按照4byte进行对齐,在64位系统时使用8byte对齐, 这不满足posix对
 *      malloc具有2 * sizeof(size_t)的要求, 在外部组件使用时需要注意.
 */
class MemAllocatorTlsf : IMemAllocator
{
public:
    MemAllocatorTlsf(void* mem, size_t size);
    ~MemAllocatorTlsf();

public: // overriedes.
    virtual void* allocate(size_t size);
    virtual void dellocate(void* ptr);

private:
    void mapping_insert(size_t size, int* fli, int* sli);


    /// @brief 初始化tlsf.
    /// @param mem 
    /// @param size 
    void initialise_tlsf(void* mem, size_t size);

private:
    block_header_t null;
    void* fl_;
    void* fl_map_;

    void* sl_;
    void* sl_map_;
    
};

#endif //! CORE_MEM_TLSF_HPP_