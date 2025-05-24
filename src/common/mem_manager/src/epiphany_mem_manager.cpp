// includes.
#include <string.h>
#include "epiphany_mem_manager.hpp"

using epiphany::common::EpiphanyMemManager;

EpiphanyMemManager::EpiphanyMemManager(void* _base_addr, size_t _size)
    : size_(_size), free_list_(NULL) 
{
    memset(_base_addr, 0, _size);
    base_addr_ = static_cast<char*>(_base_addr);
    if (base_addr_ == NULL) {
        /** Logs. */
        // throw std::invalid_argument("base_addr_ is nullptr");
    }

    /** Initialize the first block. */
    free_list_ = reinterpret_cast<MemBlock*>(base_addr_);
    free_list_->size_ = size_ - sizeof(MemBlock);
    free_list_->is_free_ = true;
    free_list_->next_ = NULL;

    /** TODO: Initialize lock. */
}

EpiphanyMemManager::~EpiphanyMemManager() {
    /** Memory release operation is completed by the user (whoever allocates is responsible for releasing). */
}

void* 
EpiphanyMemManager::alloc(size_t _size) {
    /** Lock. */
    mutex_.lock();

    /** Traverse the free list. */
    MemBlock* curr = free_list_;

    while (curr != NULL) {
        if ( curr->size_ >= (_size + sizeof(MemBlock)) && curr->is_free_ ) {
           
            /** Found suitable memory block. Split memory block. */
            split_block(curr, _size);
            curr->is_free_ = false;

            /** Unlock. */
            mutex_.unlock();

            return ( 
                reinterpret_cast<void*>
                (reinterpret_cast<char*>(curr) + sizeof(MemBlock)) );
        }

        curr = curr->next_;
    }

    /** 解锁. */
    mutex_.unlock();

    /** No suitable memory block found. */    
    return (NULL);
}

void 
EpiphanyMemManager::free(void* _ptr) {
    if ( _ptr ) {
        mutex_.lock();
        MemBlock* block = reinterpret_cast<MemBlock*>
        (reinterpret_cast<char*>(_ptr) - sizeof(MemBlock));

        block->is_free_ = true;

        merge_blocks();

        mutex_.unlock();
    }
}

void 
EpiphanyMemManager::split_block(EpiphanyMemManager::MemBlock* block, size_t size)
{
    if ( block && (block->size_ >= sizeof(MemBlock)) ) {
        MemBlock* new_block = reinterpret_cast<MemBlock*>(
            reinterpret_cast<char*>(block) + sizeof(MemBlock) + size);

        new_block->size_ = block->size_ - size - sizeof(MemBlock);
        new_block->is_free_ = true;
        new_block->next_ = block->next_;

        block->size_ = size;
        block->next_ = new_block;
    }
}

void
EpiphanyMemManager::merge_blocks() 
{
    MemBlock* current = free_list_;

    while ( current && current->next_ ) {
        MemBlock* next = current->next_;

        if ( current->is_free_ && next->is_free_ ) {
            current->size_ += sizeof(MemBlock) + next->size_;
            current->next_ = next->next_;
        } else {
            current = current->next_;
        }
    }
}