#include "cache.hpp"
#include "lruk_policy.hpp"

extern epiphany_extended_common_cache_vtavble_t epiphany_extended_common_cache_vtavble;

void 
lruk_component_init()
{
    epiphany_extended_common_cache_vtavble.lruk_create();
}