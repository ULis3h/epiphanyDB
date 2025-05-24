#include <cstring>

#include "epiphany.h"
#include "api.hpp"
#include "include/os/mem.h"
#include "include/os/thread.hpp"
#include "include/common/macros.hpp"
#include "include/common/mem_manager.hpp"

MemPool MemPool::instance = MemPool();

extern const char* __epiphany_confdfl_epiphany_open;
extern int __epiphany_library_init();
extern EpiphanyThread __epiphany_process;

EPIPHANY_RET epiphany_open_by_cfg(EPIPHANY_CONFIG* config, EPIPHANY_CONNECTION** _connection)
{  
    return EPIPHANY_OK;
}

EPIPHANY_RET epiphany_open(const char* home, 
                 const char* config, 
                 EPIPHANY_CONNECTION** _connection)
{
    EPIPHANY_RET ret = EPIPHANY_OK;

    EPIPHANY_ITEM* cbuf = NULL;
    
    EPIPHANY_ITEM expath;
    EPIPHANY_CLEAR(expath);
    
    EPIPHANY_ITEM exconfig;
    EPIPHANY_CLEAR(exconfig);
    
    EpiphanyConnectionImpl* connect = NULL;

    EpiphanySessionImpl* session = NULL;

    const char* cfg[] = {
        __epiphany_confdfl_epiphany_open, config, NULL, NULL
    };

    *_connection = NULL;
    
    /** Check if this is the first execution of the current thread. */
    EpiphanyCheckRtCode(((ret = EPIPHANY_RET(__epiphany_library_init())) == 0), ret);

    connect = new EpiphanyConnectionImpl();

    __epiphany_process.spin_lock();

    /** manager process list. */

    __epiphany_process.spin_unlock();

    session = &connect->default_session_;
    session->connection_ = (__epiphany_connection*)connect;
    session->name_ = "epiphany_open";

    connect->init();
    connect->home(home, cfg);

    /** Create cache. */


    return ( EPIPHANY_OK );
}
