#include <cstring>

#include "common/macros.hpp"
#include "api.hpp"

#include "mem.h"
#include "mem_manager.hpp"
#include "epiphany_internal.hpp"
#include "bufferpool/pager.hpp"


using epiphany::common::MemPool;

static int
__connect_load_extebsion (
    EPIPHANY_CONNECTION *epiphany_conn, 
    const char* path, 
    const char* config)
{
    // EPIPHANY_CONFIG_ITEM cval;
    EpiphanyConnectionImpl* conn;

    return 0;
}

EpiphanySessionImpl::EpiphanySessionImpl() 
{
}

EpiphanySessionImpl::~EpiphanySessionImpl() 
{
}

EpiphanyConnectionImpl::EpiphanyConnectionImpl() {
    load_extension = __connect_load_extebsion;
}

int
EpiphanyConnectionImpl::init() 
{
    return 0;
}

int 
EpiphanyConnectionImpl::home(const char* _home, const char **cfg)
{
    EPIPHANY_CONFIG_ITEM cval;
    EpiphanySessionImpl *session;

    session = &this->default_session_;

    if ( _home == NULL ) {
    }
    epiphany_strndup(_home, ( _home == NULL ) ? 0 : strlen(_home), &home_);
    return 0;
}

int 
EpiphanyConnectionImpl::open(const char* cfg [])
{
    /** Create cache. */

    return 0;
}

void* 
EpiphanyConnectionImpl::operator new(size_t size) {
    EpiphanyConnectionImpl* tmp = (EpiphanyConnectionImpl*)
    (MemPool::instance.alloc(sizeof(EpiphanyConnectionImpl)));

    ::new ( tmp ) EpiphanyConnectionImpl();
    
    return tmp;
}