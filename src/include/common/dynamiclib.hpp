#ifndef EPIPHANY_DYNAMICLIB_HPP_
#define EPIPHANY_DYNAMICLIB_HPP_

#include <stdlib.h>
#include <vector>

#include "allocator.hpp"

using std::vector;
using epiphany::common::Allocator;

namespace epiphany {

/**
 * @brief dynamic library type.
 */
class DynamicLibrary {
public:
    DynamicLibrary();
    DynamicLibrary(void* h = NULL, const char* n = "");
    DynamicLibrary(const DynamicLibrary& other);

    ~DynamicLibrary();

    DynamicLibrary& operator=(const DynamicLibrary& other);

    /// @brief 
    /// @return 
    void* get_handle() const;

    /// @brief 
    /// @return 
    const char* get_name() const;

    /// @brief 
    /// @param handle 
    void set_handle(void* handle);

    /// @brief 
    /// @param name 
    void set_name(const char* name);

private:
    void* handle_; ///< dlopen return's handle.
    const char* name_; ///< dynamic's name.
};

class DynamicLibManager {
public:

    /// @brief 
    /// @param lib 
    void add_library(const DynamicLibrary& lib);

    /// @brief 
    /// @param name 
    /// @return 
    DynamicLibrary* find_library(const char* name);

    /// @brief remove library by name.
    /// @param name 
    void remove_library(const char* name);

    /// @brief get library count.
    /// @return 
    size_t get_library_count() const;

    /// @brief clear all libraries.
    void clear();

private:
    std::vector<DynamicLibrary, Allocator<DynamicLibrary> > libs_;
};


} /// epiphany.

#endif //! EPIPHANY_DYNAMICLIB_HPP_