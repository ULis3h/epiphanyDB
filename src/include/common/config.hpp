#include "epiphany_types.hpp"

#ifndef EPIPHANY_CONFIG_HPP_
#define EPIPHANY_CONFIG_HPP_

#ifndef EPIPHANY_TEST
#define EPIPHANY_TEST 1
#endif //! EPIPHANY_TEST

#ifndef EPIPHANY_UNIX
#define EPIPHANY_UNIX 1
#endif //! EPIPHANY_UNIX

#ifndef DEFAULT_NPAGES
#define DEFAULT_NPAGES 256
#endif //! DEFAULT_NPAGES

#ifndef PAGE_HEADER_SIZE
#define PAGE_HEADER_SIZE 512
#endif //! PAGE_HEADER_SIZE

#ifndef INIT_ROOT_NO
#define INIT_ROOT_NO 9
#endif //! INIT_ROOT_NO

#ifndef PGNOS_PERPARGE
#define PGNOS_PERPARGE 254 
#define PGNOS_PERPARGE 254 
#endif //! PGNOS_PERPARGE

#ifndef PAGES_PERNODE
#define PAGES_PERNODE 8 
#endif //! PAGES_PERNODE

#if defined(HASH_DEBUG)
#ifndef HASHS_PERPARGE
#define HASHS_PERPARGE 8 // 8 for test
#endif  //! HASHS_PERPARGE

#else

#ifndef HASHS_PERPARGE
#define HASHS_PERPARGE 248
#endif  //! HASHS_PERPARGE

#endif

#ifndef HASHS_OFFS
#define HASHS_OFFS 8
#endif //! HASHS_OFFS

#ifndef HASH_INIT_SIZE
#define HASH_INIT_SIZE 17
#endif //! HASH_INIT_SIZE

#ifndef BUF_SIZE
#define BUF_SIZE 4096
#endif //! BUF_SIZE


/**
 * 默认的malloc实现,需要操作系统实现libc.
 */
#ifndef MEM_MANAGER_USE_LIBC
#define MEM_MANAGER_USE_LIBC (1<<0)
#endif //! MEM_MANAGER_USE_GLIBC

/** 
 * 操作系统自己的内存申请函数.如spaceos的MemAlloc.
 */
#ifndef MEM_MANAGER_USE_OS
#define MEM_MANAGER_USE_OS (1<<1)
#endif //! MEM_MANAGER_USE_OS


/**
 * 字节对齐数. 单位: 字节.
 * 默认: 8字节对齐.
 */
#ifndef HSL_BYTE_ALIGMENT
#define HSL_BYTE_ALIGMENT 8 
#endif //! HSL_BYTE_ALIGMENT

/**
 * ndb提供内存管理.
 */
#ifndef MEM_MANAGER_USE_SELF_OS
#define MEM_MANAGER_USE_SELF_OS (1<<2)
#endif //! MEM_MANAGER_USE_SELF_OS

#ifndef MEM_MANAGER_USE_SELF_HSL
#define MEM_MANAGER_USE_SELF_HSL (1<<3)
#endif //! MEM_MANAGER_USE_SELF_HSL

/** 
 * 内存泄露检查.
 */
#ifndef MEM_LEAK_CHECK
// #define MEM_LEAK_CHECK 1
#endif //! MEM_LEAK_CHECK

/**
 * MEM_MANAGER_USE_OS.      // 使用操作系统的malloc.
 * MEM_MANAGER_USE_SELF_OS. // 使用操作系统提供的一快内存空间.
 * MEM_MANAGER_USE_SELF_HSL // 使用HSL进行堆管理.
 */
#ifndef EPIPHANY_HEAP_MAMAGER
#define EPIPHANY_HEAP_MAMAGER MEM_MANAGER_USE_OS
#endif //! EPIPHANY_HEAP_MAMAGER

#if EPIPHANY_HEAP_MAMAGER == MEM_MANAGER_USE_SELF_HSL
#ifndef HEAP_TOTAL_SIZE
/** 这个需要根据不同的环境需求进行配置. */
#define HEAP_TOTAL_SIZE 1024 * 1024 
#endif //! HEAP_TOTAL_SIZE
#endif //! EPIPHANY_HEAP_MAMAGER == MEM_MANAGER_USE_SELF_HSL

// 开启计时
#ifndef ENABLE_TIMING
#define ENABLE_TIMING 1

// 计时精度 毫秒.
#ifndef TIMING_PER_MSEC
#define TIMING_PER_MSEC 1
#endif //!TIMING_PER_MSEC

// 计时精度 微秒.
#ifndef TIMING_PER_USEC
#define TIMING_PER_USEC 2
#endif //! TIMING_PER_USEC

#ifndef TIMING_PER
#define TIMIMG_PER TIMING_PER_USEC
#endif //! TIMIMG_PER
#endif //! ENABLE_TIMING

class EpiphanySessionImpl;
struct __epiphany_config_item;
/**
 * @brief 配置模块.
 */
class EpiphanyConfig {
public:

    /// @brief 
    EpiphanyConfig();

    /// @brief 
    ~EpiphanyConfig();

    /// @brief 初始化配置句柄.
    /// @param session 
    /// @param str 
    /// @param len 
    /// @return 
    int init(EpiphanySessionImpl* session, const char* str, size_t len = 0);

    /// @brief 处理特殊的配置值.
    /// @param value 
    /// @return 
    int process_value(void* value);

    /// @brief 获取下一个配置项.
    /// @param key 
    /// @param value 
    /// @return 
    int next(void* key, void* value);

    /// @brief 查找指定key的值.
    /// @param key 
    /// @param value 
    /// @return 
    int get_raw(void* key, void* value);

    /// @brief 给定一个字符串配置, 查询指定key的最终值.
    /// @param session [in] 会话指针.
    /// @param cfg [in] 字符串形式的配置.
    /// @param key [in] 查找的key.
    /// @param value [out] 查询结果.
    /// @return 
    int gets(EpiphanySessionImpl* session, const char** cfg, void* key, void* value);

    /// @brief 给定一个字符串配置, 查询指定key的最终值.
    /// @param session [in] 会话指针.
    /// @param cfg [in] 字符串形式的配置.
    /// @param key [in] 查找的key.
    /// @param value [out] 查询结果.
    /// @return 
    int getone(EpiphanySessionImpl* session, const char* cfg, void* key, 
               void* value);

    /// @brief 给定一个字符串配置, 查询指定key的最终值.
    /// @param session [in] 会话指针.
    /// @param cfg [in] 字符串形式的配置.
    /// @param key [in] 查找的key.
    /// @param value [out] 查询结果.
    /// @return 
    int getones(EpiphanySessionImpl* session, const char* cfg, void* key, 
                void* value);

    /// @brief 获取给定项的值.
    /// @param session 
    /// @param cfg 
    /// @param key 
    /// @param value 
    /// @return 
    int subgets(EpiphanySessionImpl* session, void* cfg, void* key, void* value);

private:
    /// @brief 给定一个字符串配置, 查询指定key的最终值.
    /// @param session [in] 会话指针.
    /// @param cfg [in] 字符串形式的配置.
    /// @param key [in] 查找的key.
    /// @param value [out] 查询结果.
    /// @return 
    int get(EpiphanySessionImpl* session, const char** cfg, void* key, void* value);


    /// @brief 获取给定项的值.
    /// @param session 
    /// @param cfg 
    /// @param key 
    /// @param value 
    /// @return 
    int subgetraw(EpiphanySessionImpl* session, void* cfg, void* key, void* value);

private:
    EpiphanySessionImpl *session_; ///< 会话实例.
    const char *orig_;        ///< 原始配置字符串.
    const char *end_;         ///< 字符串结束位置.
    const char *cur_;         ///< 当前解析位置.

    int depth_;    ///< 嵌套深度.
    int top_;      ///< 顶层标记.
    epiphany_int8 *go_; ///< 当前状态机.
};

struct __epiphany_config_item 
{
    const char* str;
    size_t len;
    epiphany_int64 val;
    enum {
        ITEM_STRING,
        ITEM_ID,
        ITEM_NUM,
        ITEM_STRUCT,
    } type;
};

#endif //! EPIPHANY_CONFIG_HPP_