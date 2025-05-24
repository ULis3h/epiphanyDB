#include "mem.h"
#include "file.hpp"

#include <string>
#include <cstdio>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

namespace epiphany 
{

namespace os 
{

struct File::Impl
{
    int fd; ///< File descriptor.

    OpenMode mode_; ///< Open mode.
    const char* path_; ///< File path.

    void* operator new(size_t size) 
    {
        return epiphany_malloc(size);
    }

    void operator delete(void* ptr) 
    {
        epiphany_free(ptr);
    }
};

File::File(const char* path, OpenMode mode)
{
    impl_ = new Impl;
    impl_->path_ = path;
    impl_->mode_ = mode;

    int flags = 0;

    if ( mode & READ )
        flags |= O_RDONLY;
    if ( mode & WRITE )
        flags |= O_WRONLY | O_CREAT | O_TRUNC;
    if ( mode & APPEND )
        flags |= O_WRONLY | O_CREAT | O_APPEND;

    impl_->fd = open(path, flags);

    if ( impl_->fd == -1 ) {
        delete impl_;
        impl_ = NULL;
        throw FileException(std::string("open file failed"));
    }
}

File::~File()
{
    close();
    delete impl_;
    impl_ = NULL;
}

void File::close()
{
    if ( impl_->fd != -1 ) {
        ::close(impl_->fd);
        impl_->fd = -1;
    }
}

bool File::isOpen() const
{
    return impl_->fd != -1;
}

size_t File::read(void* buffer, size_t size)
{
    if ( !isOpen() )
        throw FileException(std::string("file is not open"));
    
    return ::read(impl_->fd, buffer, size);
}

size_t File::write(const void* buffer, size_t size)
{
    if ( !isOpen() )
        throw FileException(std::string("file is not open"));
    
    return ::write(impl_->fd, buffer, size);
}

void File::seek(size_t offset, int origin)
{
    if ( !isOpen() )
        throw FileException(std::string("file is not open"));
    
    ::lseek(impl_->fd, offset, origin);
}

size_t File::tell() 
{
    if ( !isOpen() )
        throw FileException(std::string("file is not open"));
    
    return ::lseek(impl_->fd, 0, SEEK_CUR);
}

size_t File::size() 
{
    if ( !isOpen() )
        throw FileException(std::string("file is not open"));

    struct stat st;
    if ( ::fstat(impl_->fd, &st) == -1 )
        throw FileException(std::string("fstat failed"));
    
    return st.st_size;
}

} //! namespace os
} //! namespace epiphany