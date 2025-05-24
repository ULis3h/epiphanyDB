#include "pager.hpp"
#include "mem_manager.hpp"

using epiphany::bufferpool::Pager;

Pager::Pager() 
{
}

void*
Pager::operator new(size_t size) {
    Pager* rtp = (Pager*) MemPool::instance.alloc(size);
    return rtp;
}

Pager::~Pager()
{   
}

void
Pager::init() 
{
}

Pgno 
Pager::allocate_page() {
    return (Pgno) (0); 
}
