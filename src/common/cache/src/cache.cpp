
#include "lru_policy.hpp"
#include "cache.hpp"

using epiphany::common::ICachePolicy;


epiphany_extended_common_cache_vtavble_t epiphany_extended_common_cache_vtavble;

/**
 * LruK 组件实现.
 */
#if defined(NOT_LOAD_COMMON_CACHE_LRU_K)
#define lruk_component_init()
#else
extern void lruk_component_init();
#endif //! NOT_LOAD_COMMON_CACHE_LRU_K

#ifndef common_cache_lruk_create_wrapper
#define common_cache_lruk_create_wrapper(cfg) \
(epiphany_extended_common_cache_vtavble.lruk_create ? \
epiphany_extended_common_cache_vtavble.lruk_create(cfg) : 0)
#endif //! common_cache_lruk_create_wrapper
/**----------------------------lruk impl end----------------------------------*/

void epiphany_common_cache_component_init()
{
    lruk_component_init();
}
