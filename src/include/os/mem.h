#ifndef EPIPHANY_OS_MEM_H_
#define EPIPHANY_OS_MEM_H_

#ifdef __cplusplus
    extern "C"
    {
#endif 

#include <stddef.h>


/// @brief alloc mem.
/// @param _size [in] Requested size.
/// @return 
void* epiphany_malloc(size_t _size);


/// @brief free mem.
/// @param _ptr [in] Pointer to free.
void epiphany_free(void* _ptr);


/// @brief Copy a string to a specified string and terminate with \0.
/// @param str 
/// @param len 
/// @param retp 
/// @return
int epiphany_strndup(const char* str, size_t len, void* retp);


#ifdef __cplusplus
    }
#endif 

#endif //! EPIPHANY_OS_MEM_H_