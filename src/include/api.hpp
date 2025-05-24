#ifndef EPIPHANY_API_HPP_
#define EPIPHANY_API_HPP_

#include <list>

#include "epiphany.h"
#include "mutex.hpp"
#include "epiphany_types.hpp"
#include "epiphany_internal.hpp"


using epiphany::os::EpiphanyMutex;


class EpiphanySessionImpl : public __epiphany_session {
public:
    EpiphanySessionImpl();
    ~EpiphanySessionImpl();

public:
    const char* name_;
    DS_BTREE* data_store_btree_;
};


/**
 * @brief impl fot __epiphany_connection.
 */
class EpiphanyConnectionImpl : __epiphany_connection
{
public: 
    EpiphanyConnectionImpl();

    void* operator new(size_t size);

    /// @brief 
    int init();

    /// @brief 设置数据库主目录.
    /// @param _home 
    /// @param cfg 
    /// @return 
    int home(const char* _home, const char **cfg);

    /// @brief 打开一个连接.
    /// @param cfg 
    /// @return 
    int open(const char* cfg []);

public:
    EpiphanySessionImpl default_session_; ///< 默认会话.    

    // EpiphanySessionImpl default_session_; ///< 默认会话.
    const char* home_; ///< Database home dir.
    bool is_new_;

    // std::list, btree list.


    EpiphanyMutex spinlock_;
    epiphany_size_t store_locked_cnt_; ///< Locked: store count.
    
};


#endif //! EPIPHANY_API_HPP_