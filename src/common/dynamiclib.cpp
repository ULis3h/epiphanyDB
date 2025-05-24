#include <string.h>

#include "dlh.hpp"
#include "dynamiclib.hpp"

using epiphany::os::EpiphanyDLH;
using epiphany::DynamicLibrary;
using epiphany::DynamicLibManager;

DynamicLibrary::DynamicLibrary() 
 : handle_(NULL), name_(NULL) 
{
}

DynamicLibrary::DynamicLibrary(void* h, const char* n)
 : handle_(h), name_(n) 
{
}

DynamicLibrary::DynamicLibrary(const DynamicLibrary& other)
 : handle_(other.handle_), name_(other.name_)
{
}

DynamicLibrary::~DynamicLibrary()
{
    if ( handle_ ) {
        EpiphanyDLH::dl_close(handle_);        
        handle_ = NULL;
    }
}

DynamicLibrary& 
DynamicLibrary::operator=(const DynamicLibrary& other)
{
    if ( this != &other )
    {
        if ( handle_ ) {
            EpiphanyDLH::dl_close(handle_);
        }
        handle_ = other.handle_;
        name_ = other.name_;
    }

    return *this;
}

void* 
DynamicLibrary::get_handle() const {
    return handle_;
}

const char*
DynamicLibrary::get_name() const {
    return name_;
}

void
DynamicLibrary::set_handle(void* handle)
{
    handle_ = handle;
}

void 
DynamicLibrary::set_name(const char* name)
{
    name_ = name;
}

void
DynamicLibManager::add_library(const DynamicLibrary& lib)
{
    libs_.push_back(lib);
}

DynamicLibrary*
DynamicLibManager::find_library(const char* name)
{
    vector<DynamicLibrary, Allocator<DynamicLibrary> >::iterator it;
    for ( it = libs_.begin(); it != libs_.end(); ++it ) {
        if ( strcmp((*it).get_name(), name) == 0) {
            return &(*it);
        }
    }
    return NULL;
}

void
DynamicLibManager::remove_library(const char* name)
{
    vector<DynamicLibrary, Allocator<DynamicLibrary> >::iterator it;
    for ( it = libs_.begin(); it != libs_.end(); ++it ) {
        if ( strcmp((*it).get_name(), name) == 0) {
            libs_.erase(it);
            return;
        }
    }
}

size_t
DynamicLibManager::get_library_count() const 
{
    return libs_.size();
}

void 
DynamicLibManager::clear()
{
    libs_.clear();
}