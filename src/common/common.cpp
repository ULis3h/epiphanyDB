#include "common.h"

struct epiphany_extended_common_vtable_t
{
    void (*cache_componet_init)();
};

epiphany_extended_common_vtable_t epiphany_common_componet_vtable;

void 
epiphany_common_componet_init()
{

}