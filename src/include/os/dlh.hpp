#ifndef EPIPHANY_OS_DLH_HPP_
#define EPIPHANY_OS_DLH_HPP_

namespace epiphany {
namespace os {


class EpiphanyDLH {
public:

    /// @brief open a dynamic library.
    /// @param n dynamic name.
    /// @param f open flag.
    /// @return handle or NULL.
    static void* dl_open(const char* n, int f);

    /// @brief close a dynamic library.
    /// @param handle 
    /// @return 
    static int dl_close(void* handle);
};


} /// os.
} /// epiphany.

#endif //! EPIPHANY_OS_DLH_HPP_