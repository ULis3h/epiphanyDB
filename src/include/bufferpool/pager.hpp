#ifndef EPIPHANY_PAGER_PAGER_HPP_
#define EPIPHANY_PAGER_PAGER_HPP_

#include <map>
#include <vector>

#include "page.hpp"
#include "epiphany_types.hpp"
#include "cache.hpp"
#include "allocator.hpp"

using std::map;
using std::vector;
using epiphany::common::Cache;
using epiphany::common::Allocator;

namespace epiphany 
{

namespace bufferpool 
{

/**
 * @brief 缓冲页面管理, buffer pool.
 * 一个数据库对应一组数据文件, 这组数据文件的创建
 */
class Pager {
public:

    /// @brief 
    Pager();

    /// @brief 重载new.
    /// @param size 
    /// @return 
    void* operator new(size_t size);

    /// @brief 
    ~Pager();
public:

    /// @brief 初始化Pager.
    void init();

    /// @brief 申请一个页面.
    /// @return 页面号, 大于0为合法页号.
    Pgno allocate_page();
private:
    Cache<Pgno, size_t> cache_;
    
    /** 页面缓冲区. */
    vector<IPage*, Allocator<IPage*> > buffer_;

    ///< 页号到缓冲区偏移的映射.
    map<Pgno, size_t, Allocator<std::pair<Pgno, size_t> > > map_;
};

} //! epiphany::bufferpool

} //! epiphany


#endif //! EPIPHANY_PAGER_PAGER_HPP_