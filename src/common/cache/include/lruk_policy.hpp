#ifndef EPIPHANY_COMMON_CACHE_LRU_K_POLICY_HPP_
#define EPIPHANY_COMMON_CACHE_LRU_K_POLICY_HPP_

// includes.
#include <cstddef>
#include "cache.hpp"

namespace epiphany {
namespace common {
namespace cache {

template <typename K, typename V>
class LruKPolicy : public ICachePolicy<K, V> {
public:
    explicit LruKPolicy(size_t capacity);
    virtual ~LruKPolicy();

public: // overrides.
    virtual void touch(const K& key);
    
    virtual void add(const K& key, const V& value);

    virtual void remove(const K& key);

    virtual K ecvt();

    virtual void clear();

    virtual size_t size() const;
};

}
}
}
#endif //! EPIPHANY_COMMON_CACHE_LRU_K_POLICY_HPP_