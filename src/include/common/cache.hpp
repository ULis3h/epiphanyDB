#ifndef EPIPHANY_CACHE_HPP_
#define EPIPHANY_CACHE_HPP_

#include <cstddef>

/**
 * 扩展组件函数表.
 */
struct epiphany_extended_common_cache_vtavble_t
{
    /** lruk */
    void* (*lruk_create)();
};

namespace epiphany    {
namespace common {


/**
 * @brief 缓存策略接口， 如果需要更新缓存策略， 只需要实现此接口即可.
 */
template <typename K, typename V>
class ICachePolicy {
public:
    virtual ~ICachePolicy() { };

    /**
     * @brief 访问项.
     */
    virtual void touch(const K& key) = 0;

    /**
     * @brief 添加项.
     */
    virtual void add(const K& key, const V& value) = 0;

    /**
     * @brief 删除项.
     */
    virtual void remove(const K& key) = 0;

    /**
     * @brief 获取将要淘汰的项.
     */
    virtual K evict() = 0;

    /**
     * @brief 清空策略.
     */
    virtual void clear() = 0;

    /**
     * @brief 获取当前项的数量.
     */
    virtual size_t size() const = 0;
};

class CachePolicyFactory {
public:
    template <typename K, typename V>
    static ICachePolicy<K, V>* 
    create_policy();
};


struct CacheInner 
{
    unsigned long pages_inmem_;
};


/**
 * @brief 缓存模块.
 *  缓存模块通过将数据存储在内存中,使数据库不必每次都从磁盘中读取数据, 以降低对磁盘的I/O需求,
 * 减少访问磁盘的开销.
 */
template <typename K, typename V>
class Cache {
    
    /**
     * @brief 创建缓存.
     */
    int create();
private:
    ICachePolicy<K, V>* policy_; ///< 缓存策略.
    CacheInner inner_;
};

} /// common.
} /// epiphany.

#endif //! EPIPHANY_CACHE_HPP_