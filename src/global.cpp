#include "thread.hpp"

EpiphanyThread __epiphany_process;
static int __epiphany_thread_once_failed;

static void 
epiphany_thread_once() {
    __epiphany_process.spin_init();
}

int 
__epiphany_library_init(void) {
    static int first = 1;
    int ret;

    if ( first ) {
        if ( (ret = EpiphanyThread::thread_once(epiphany_thread_once)) != 0 ) {
            __epiphany_thread_once_failed = ret;
            first = 0;
        } 
    }
    return ( __epiphany_thread_once_failed );
}