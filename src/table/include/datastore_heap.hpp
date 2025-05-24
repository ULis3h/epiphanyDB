#ifndef EPIPHANY_HEAP_PAGE_HPP_
#define EPIPHANY_HEAP_PAGE_HPP_

#include <cstddef>

#include "page.hpp"
#include "epiphany_types.hpp"
#include "record.hpp"
#include "datastore.hpp"

namespace epiphany    {
namespace table {

/**
 * 使用堆表存储时页面设计.
 */
class HeapPage : public IPage {
public:

private:
    Pgno next_page_;    ///< 下一个页面的页号.
    epiphany_size_t *lower_; ///< 页面lower指针.
    epiphany_size_t *upper_; ///< 页面upper指针.
};

class DataStoreHeap : public DataStore 
{
public:
    

};


} /// table.
} /// epiphany.

#endif //! EPIPHANY_HEAP_PAGE_HPP_