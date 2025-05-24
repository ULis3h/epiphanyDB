#ifndef EPIPHANY_COMMON_CACHE_LRU_POLICY_HPP_
#define EPIPHANY_COMMON_CACHE_LRU_POLICY_HPP_

// includes.
#include <cstddef>
#include <map>
#include <iostream>

#include "cache.hpp"
#include "allocator.hpp"

namespace epiphany {
namespace common {
namespace cache {

using epiphany::common::Allocator;

/**
 * @brief 基于双向链表实现的LRU缓存策略.
 */
template <typename Key, typename Value>
class LRUCache : public ICachePolicy<Key, Value> {
private:
    struct ListNode {
        Key key;
        Value value;
        ListNode *prev;
        ListNode *next;

        ListNode(const Key &k, const Value &v) : key(k), value(v), prev(NULL), 
        next(NULL) {}
    };

    typedef std::map<Key, ListNode*, 
    Allocator<std::pair<Key, ListNode*> > > HashMap;

public:
    LRUCache(size_t capacity);

    ~LRUCache();

public: // overrides.

    /// @brief 将某个元素标记为最近使用
    /// @param key 
    virtual void touch(const Key &key);

    /// @brief 添加或更新缓存元素
    /// @param key 
    /// @param value 
    virtual void add(const Key &key, const Value &value);

    /// @brief 删除某个元素
    /// @param key 
    virtual void remove(const Key &key);

    /// @brief 移除最久未使用的元素
    virtual void evict();

    /// @brief 清空缓存
    virtual void clear();

    /// @brief 返回缓存大小
    /// @return 
    virtual size_t size() const;

    /// @brief 打印缓存内容（调试用）
    void printCache() const;

private:

    /// @brief 将节点移到链表头部
    /// @param node 
    void moveToHead(ListNode *node);

    /// @brief 移除链表尾部节点
    void removeTail();
private:
    size_t capacity_; ///< 缓存容量
    size_t size_;     ///< 当前缓存大小
    HashMap cache_;   ///< 哈希表，用于快速查找
    ListNode *head_;  ///< 双向链表头部（最近使用）
    ListNode *tail_;  ///< 双向链表尾部（最久未使用）
    Allocator<ListNode> node_allocator_; ///< node分配器.
};

template <typename Key, typename Value>
LRUCache<Key, Value>::LRUCache(size_t capacity) : capacity_(capacity), size_(0), 
head_(NULL), tail_(NULL) 
{}

template <typename Key, typename Value>
LRUCache<Key, Value>::~LRUCache() 
{
    clear();
}
template <typename Key, typename Value> void 
LRUCache<Key, Value>::touch(const Key &key)
{
    typename HashMap::iterator it = cache_.find(key);
    if (it != cache_.end())
    {
        moveToHead(it->second);
    }
}

template <typename Key, typename Value> void 
LRUCache<Key, Value>::add(const Key &key, const Value &value)
{
    typename HashMap::iterator it = cache_.find(key);

    if (it != cache_.end())
    {
        // 元素已存在，更新值并移动到头部
        it->second->value = value;
        moveToHead(it->second);
    }
    else
    {
        // 添加新元素
        if (size_ >= capacity_)
        {
            evict(); // 如果缓存已满，移除最久未使用的元素
        }

        // ListNode *newNode = new ListNode(key, value);
        ListNode *newNode = node_allocator_.allocate();

        cache_[key] = newNode;
        moveToHead(newNode);
        ++size_;
    }
}

template <typename Key, typename Value> void 
LRUCache<Key, Value>::remove(const Key &key)
{
    typename HashMap::iterator it = cache_.find(key);
    if (it != cache_.end())
    {
        ListNode *node = it->second;

        // 从链表中移除
        if (node->prev)
        {
            node->prev->next = node->next;
        }
        if (node->next)
        {
            node->next->prev = node->prev;
        }
        if (node == head_)
        {
            head_ = node->next;
        }
        if (node == tail_)
        {
            tail_ = node->prev;
        }

        // 从哈希表中移除
        cache_.erase(it);
        // delete node;
        node_allocator_.deallocate(node);

        --size_;
    }
}

template <typename Key, typename Value> void 
LRUCache<Key, Value>::evict() 
{
    removeTail();
}

template <typename Key, typename Value> void 
LRUCache<Key, Value>::clear() 
{
    while (head_ != NULL) {
        ListNode *node = head_;
        head_ = head_->next;
        // delete node;
        node_allocator_.deallocate(node);
    }

    cache_.clear();
    head_ = tail_ = NULL;
    size_ = 0;
}

template <typename Key, typename Value> size_t 
LRUCache<Key, Value>::size() const 
{
    return size_;
}

template <typename Key, typename Value> void 
LRUCache<Key, Value>::printCache() const 
{
    ListNode *node = head_;
    while (node != NULL) {
        std::cout << "(" << node->key << ", " << node->value << ") ";
        node = node->next;
    }
    std::cout << std::endl;
}


template <typename Key, typename Value> void 
LRUCache<Key, Value>::moveToHead(ListNode *node) 
{
    if (node == head_)
    {
        return; // 如果已经是头节点，无需移动
    }

    // 从当前位置移除节点
    if (node->prev)
    {
        node->prev->next = node->next;
    }
    if (node->next)
    {
        node->next->prev = node->prev;
    }

    if (node == tail_)
    {
        tail_ = node->prev; // 更新尾节点
    }

    // 插入到链表头部
    node->prev = NULL;
    node->next = head_;
    if (head_)
    {
        head_->prev = node;
    }
    head_ = node;

    if (tail_ == NULL)
    {
        tail_ = head_; // 如果链表之前为空，尾节点也指向当前节点
    }
}

template <typename Key, typename Value> void 
LRUCache<Key, Value>::removeTail() 
{
    if (tail_ == NULL)
    {
        return; // 链表为空
    }

    ListNode *oldTail = tail_;
    tail_ = tail_->prev;

    if (tail_)
    {
        tail_->next = NULL;
    }
    else
    {
        head_ = NULL; // 如果链表为空，头节点也置为 NULL
    }

    cache_.erase(oldTail->key);
    // delete oldTail;
    node_allocator_.deallocate(oldTail);
    --size_;
}

} /// cache.
} /// common.
} /// epiphany.
#endif //! EPIPHANY_COMMON_CACHE_LRU_POLICY_HPP_