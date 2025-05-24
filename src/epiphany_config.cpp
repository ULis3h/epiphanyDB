#include "epiphany_config.hpp"
#include "epiphany.h"

namespace epiphany {
// CPPS.
ConfigImpl::ConfigImpl(const char* home, int mode)
    : home_(home), mode_(mode),
      cache_size_(0), page_size_(4096), read_only_(false) 
{ }

ConfigImpl::~ConfigImpl() 
{ }

}

extern "C" {

EPIPHANY_CONFIG* epiphany_config_create(const char* home, EPIPHANY_STORAGE_MODE mode, 
                              void* cache_base, size_t cache_size) 
{
    EPIPHANY_CONFIG* config = new EPIPHANY_CONFIG();
    config->inner_ = new epiphany::ConfigImpl(home, mode);
    return config;
}

void 
epiphany_config_destroy(EPIPHANY_CONFIG* config) 
{
    if (config) {
        delete reinterpret_cast<epiphany::ConfigImpl*>(config->inner_);
        config->inner_ = NULL;
        delete config;
        config = NULL;
    }
}

} // extern "C"
