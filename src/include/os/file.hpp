#ifndef EPIPHANY_OS_FILE_HPP_
#define EPIPHANY_OS_FILE_HPP_

#include <cstdio>
#include <cstddef>
#include <stdexcept>

namespace epiphany {
namespace os {

enum OpenMode
{
    /** 只读. */
    READ = 0,

    /** 写入，覆盖. */
    WRITE = 1,

    /** 追加. */
    APPEND = 2,

    /** 读写. */
    READWRITE = 3
};

class FileException : public std::runtime_error
{
public:
    FileException(const std::string& msg) : std::runtime_error(msg) {}
};

/**
 * @brief 文件操作类. 所有需要适配的操作系统都需要实现此类. 
 * 
 */
class File {
public:
    File(const char* path, OpenMode mode = READ);

    ~File();

    /** 基础操作. */
    void close();
    bool isOpen() const;

    size_t read(void* buffer, size_t size);
    size_t write(const void* buffer, size_t size);

    /** 文件定位 */
    void seek(size_t offset, int origin = SEEK_SET);

    /** 返回文件当前位置. */
    size_t tell();

    /** 返回文件大小. */
    size_t size();

private:
    File(const File&);
    File& operator=(const File&);

    /** 平台相关实现. */
    struct Impl;
    Impl* impl_;
};

} /// epiphany
} /// os
#endif //! EPIPHANY_OS_FILE_HPP_