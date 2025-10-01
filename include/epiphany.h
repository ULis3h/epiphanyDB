#ifndef _EPIPHANY_H_
#define _EPIPHANY_H_

#ifdef __cplusplus
    extern "C"
    {
#endif 

/***************************required includes**********************************/
#include <stddef.h>
#include <stdint.h>
/**************************required includes end*******************************/

/***************************epiphany version***********************************/
#define EPIPHANY_VERSION_MAJOR 0
#define EPIPHANY_VERSION_MINOR 0
#define EPIPHANY_VERSION_PATCH 1
#define EPIPHANY_VERSION_STRING "epiphany 0.0.0.1: (September 23, 2024)"
/***************************epiphany version end*******************************/

/***************************epiphany structs define****************************/
struct __epiphany_config; typedef struct __epiphany_config EPIPHANY_CONFIG;
struct __epiphany_connection; typedef struct __epiphany_connection EPIPHANY_CONNECTION;
struct __epiphany_session; typedef struct __epiphany_session EPIPHANY_SESSION;
struct __epiphany_item; typedef struct __epiphany_item EPIPHANY_ITEM;

/// 返回码定义:
typedef enum __epiphany_ret {
    /** 调用epiphany接口正常执行时的函数返回值. */
    EPIPHANY_OK = 0, 

    /** OS模块返回值定义: */
    EPIPHANY_OS_RUNTIME_INIT_FAILED = 100,

    /** COMMON模块返回值定义. */

    /** 配置模块没有查询到指定Key的值. */
    EPIPHANY_COMMON_GET_CONFIG_KEY_NOTFOUND = 300,
} EPIPHANY_RET;

/**
 * @brief 存储引擎的存储模式.
 */
typedef enum __epiphany_storage_mode
{
    /** 支持行列混合存储. */
    MODE_DEFAULT = 0,

    /** mco的行存储.  */
    MODE_MCO,
} EPIPHANY_STORAGE_MODE;

/**
 * @brief 配置结果.
 * 
 */
struct __epiphany_config {

    /** 设置存储引擎的存储模式 */
    int (*storage_mode)(EPIPHANY_STORAGE_MODE mode);

    void* inner_;
}; 

struct __epiphany_item {
    const void* data_;
    uint32_t size_;
};

struct __epiphany_connection {

    int (*load_extension)(EPIPHANY_CONNECTION *connection, const char* path, 
                          const char* config);

    int (*add_cursor_type)(EPIPHANY_CONNECTION *connection); 
};

struct __epiphany_session {

    EPIPHANY_CONNECTION* connection_;

    int (*close)(EPIPHANY_SESSION* session, const char* config);

};

/*************************epiphany strusts define end**************************/

/// @brief 存储引擎配置创建函数.
/// @param home [in] 数据库主目录.
/// @param mode [in] 打开模式.
/// @param cache_base [in] 缓存基地址. 
/// @param cache_size [in] 缓存大小.
/// @return success: 配置结构指针 / fail: NULL.
EPIPHANY_CONFIG* epiphany_config_create(const char* home, EPIPHANY_STORAGE_MODE mode, 
                              void* cache_base, size_t cache_size);


/// @brief 销毁配置结构内存.
/// @param config 
void epiphany_config_destroy(EPIPHANY_CONFIG* config);

/// @brief create epiphany by path.
///        在第一版存储引擎的实现中，一个运行环境中只有一个存储引擎，且外部使用存储引擎时使用
///        宏去调用，如使用storage_open宏去开启存储引擎，这样使得在操作多个数据库时，
///        存储引擎内部数据库切换错乱，为解决这个问题，在这一版本的存储引擎实现中，重构为一个
///        独立的存储引擎实例，每个数据库都有一个独立的存储引擎实例，并且添加了连接的概念，
///        使得存储引擎可以支持多个数据库同时操作.
/// @param home [in] 数据库文件目录. 
/// @param config [in] 传入字符串类型的配置信息.
/// @param connection [out] 传入指针类型的连接指针.
/// @return success: EPIPHANY_OK / fail: 具体返回值.
EPIPHANY_RET epiphany_open(const char* home, const char* config, 
                 EPIPHANY_CONNECTION** connection);

/// @brief create epiphany by config.
/// @param config [in] 传入epiphany_config_t类型的配置信息.
/// @param connection [out] 传入指针类型的连接指针.
/// @return success: EPIPHANY_OK / fail: 具体返回值.             
EPIPHANY_RET epiphany_open_by_cfg(EPIPHANY_CONFIG* config, EPIPHANY_CONNECTION** connection);




/****************************epiphany api define end********************************/

#ifdef __cplusplus
    }
#endif 

#endif //! _EPIPHANY_H_