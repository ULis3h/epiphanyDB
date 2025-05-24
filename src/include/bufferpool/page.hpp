#ifndef EPIPHANY_PAGE_HPP_
#define EPIPHANY_PAGE_HPP_

#include "epiphany_types.hpp"

namespace epiphany 
{

namespace bufferpool 
{


/**
 * @brief 无效的页面号. 
 * 
 */
static const Pgno INVALID_PAGENO = -1;

/**
 * @brief 头页面页面号. 
 * 
 */
static const Pgno HEADER_PAGENO = 0;

/**
 * @brief 页面大小. 
 * 
 */
static const size_t PAGE_SIZE = 4096;

/*-------------------------------IPage Define---------------------------------*/
/**
 * Page是文件中某一段空间在内存中的表现形式, 虽然页面的大小是可配置的, 但为了读写效率, 将
 * 页面大小限制为4k的整数倍; 不同的文件组织策略有不同的Page实现, 提取一个基类, 其余的衍生页面
 * 都继承此类, 对于衍生页面, 其具体实现应当放到各自的模块文件中.
 */
class IPage
{
public:
    /// @brief 获取页面类型.
    /// @return 
    virtual char page_type(void) = 0;
    
    /// @brief 获取页面标识.
    /// @return 
    virtual char page_flag(void) = 0;
    
    /// @brief 获取页面状态.
    /// @return 
    virtual char page_state(void) = 0;

    /// @brief 获取页面是否为脏页面
    /// @return 
    virtual char is_dirty(void) = 0;
};
/*----------------------------IPage Define End--------------------------------*/


/*---------------------------Page Factory Define------------------------------*/
class PageFactory
{
public:
    virtual IPage* createPage() = 0;
    virtual ~PageFactory() { }
};
/*----------------------------Page Factory End--------------------------------*/

} //! namespace bufferpool

} //! namespace epiphany

#endif //! EPIPHANY_PAGE_HPP_