#ifndef EPIPHANY_BITMAP_HPP_
#define EPIPHANY_BITMAP_HPP_

#include <cstddef>

namespace epiphany    {
namespace util {

struct BitmapInner {
    size_t size_;
    char* bits_;
};

class Bitmap {
public:
    Bitmap();
    ~Bitmap();

public:
    /// @brief 
    /// @param size 
    void resize(size_t size);

    /// @brief 
    /// @param index 
    void set(size_t index);

    /// @brief 
    /// @param index 
    void clear(size_t index);

    /// @brief 
    /// @return 
    size_t size() const;

    /// @brief 
    /// @return 
    size_t bytes() const;
private:
    BitmapInner inner_;
};

} /// util.
} /// epiphany.

#endif //! EPIPHANY_BITMAP_HPP_