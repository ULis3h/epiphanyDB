#ifndef UTIL_MACROS_HPP_
#define UTIL_MACROS_HPP_
#include <stddef.h>
#include <stdint.h>
#include <cstdio>
#include <cassert>
#include <stdarg.h>

#include "config.hpp"
#include "log.hpp"

#ifndef DB_DEBUG
#define DB_DEBUG 0x04
#endif

/** 检测操作系统位数 */
#if defined(_WIN64) || defined(__x86_64__) || defined(__aarch64__) || \
    defined(__ppc64__) || defined(__powerpc64__) || defined(__sparc64__) || \
    defined(__mips64) || defined(__ia64__) || defined(__alpha__) || \
    defined(__s390x__) || defined(__riscv) && __riscv_xlen == 64
    #define EPIPHANY_64BIT 1
#else
    #define EPIPHANY_32BIT 1
#endif

/** 根据操作系统位数定义指针大小 */
#ifdef EPIPHANY_64BIT
    typedef uint64_t epiphany_ptr_t;
    #define EPIPHANY_PTR_SIZE 8
#else
    typedef uint32_t epiphany_ptr_t;
    #define EPIPHANY_PTR_SIZE 4
#endif

#ifndef EpiphanyAssert
#define EpiphanyAssert(expr)        \
    do                         \
    {                          \
        if (!!(expr))          \
            ;                  \
        else                   \
        {                      \
            printf("[error]"); \
            assert(0);         \
        }                      \
    } while (0)
#endif //! EpiphanyAssert

#ifndef EpiphanyCheckAndReturn
#define EpiphanyCheckAndReturn(cond, retval, fmt, ...) \
    if (!(cond))                                   \
    {                                              \
        LogInfo(" " fmt "", ##__VA_ARGS__);        \
        return (retval);                           \
    }
#endif //! EpiphanyCheckAndReturn

#ifndef Tzdb_Array_len
#define Tzdb_Array_len(array) (sizeof(array) / sizeof((array)[0]))
#endif //! Tzdb_Array_len(array)

#define TZ_ISNULL(p) (EpiphanyNull == (p))

#define EpiphanyCheckRtVoid(expr) \
    do                       \
    {                        \
        assert(expr);        \
        if (TZ_ISNULL(expr)) \
        {                    \
            return;          \
        }                    \
    } while (0)

#define EpiphanyCheckRtPointer(expr) \
    do                          \
    {                           \
        if (TZ_ISNULL(expr))    \
        {                       \
            return NULL;        \
        }                       \
    } while (0)

#define EpiphanyCheckRtInt(expr)  \
    do                       \
    {                        \
        if (TZ_ISNULL(expr)) \
        {                    \
            return -1;       \
        }                    \
    } while (0)

#define EpiphanyCheckRtCode(expr, ret) \
    do                            \
    {                             \
        if (TZ_ISNULL((expr)))    \
        {                         \
            return ret;           \
        }                         \
    } while (0)

#define EpiphanyCheckNRtCode(...)                                               \
    do                                                                     \
    {                                                                      \
        if (check_null_args(__VA_ARGS__, NULL))                            \
        {                                                                  \
            printf("One or more arguments are NULL, exiting function.\n"); \
            return EpiphanyRet(100); /* EpiphanyRet Operate nullptr return code. */  \
        }                                                                  \
    } while (0)

#define EpiphanyCheckNRtInt(...)                                                \
    do                                                                     \
    {                                                                      \
        if (check_null_args(__VA_ARGS__, NULL))                            \
        {                                                                  \
            printf("One or more arguments are NULL, exiting function.\n"); \
            return int(-1);                                             \
        }                                                                  \
    } while (0)

#define EpiphanyCheckNRtVoid(...)                                               \
    do                                                                     \
    {                                                                      \
        if (check_null_args(__VA_ARGS__, NULL))                            \
        {                                                                  \
            printf("One or more arguments are NULL, exiting function.\n"); \
            return;                                                        \
        }                                                                  \
    } while (0)

#define EpiphanyCheckNRtPointer(...)                                       \
    do                                                                \
    {                                                                 \
        if (check_null_args(__VA_ARGS__, NULL))                       \
        {                                                             \
            printf("Operate a nullptr.\n");                           \
            return EpiphanyNull; /* EpiphanyRet Operate nullptr return code. */ \
        }                                                             \
    } while (0)

#ifndef EpiphanyAssertAndCheckRtCode
#define EpiphanyAssertAndCheckRtCode(expr, code) \
    do                                      \
    {                                       \
        EpiphanyAssert((expr));                  \
        EpiphanyCheckRtCode(expr, code);         \
    } while (0)
#endif //! EpiphanyAssertAndCheckRtCode

inline int check_null_args(void *first, ...)
{
    va_list args;
    void *arg = first;
    int is_null = 0;

    va_start(args, first);
    while (arg != NULL)
    {
        if (arg == NULL)
        {
            is_null = 1;
            break;
        }
        arg = va_arg(args, void *);
    }
    va_end(args);
    return is_null;
}

/// 调试与输出
#define BT_LOG(fmt, ...)                                                    \
    do                                                                      \
    {                                                                       \
        if (debug_switch)                                                   \
        {                                                                   \
            fflush(stdout);                                                 \
            printf(" [\033[32mbtree info\033[0m]: " fmt "", ##__VA_ARGS__); \
            fflush(stdout);                                                 \
        }                                                                   \
    } while (0)

/// 释放一个指针所指向的内存空间
#define EpiphanyDelete(p) do{ if(p != NULL) delete p; p = NULL;} while(0)

#ifndef NEED_ALIGN
#ifdef SPACEOS2
#define NEED_ALIGN DOALIGN
#else
#define NEED_ALIGN(x, b) x
#endif //! SPACEOS2
#endif //! NEED_ALIGN

/// 将一个内存地址按照n字节对齐.
#ifndef EpiphanyAlignToN
#define EpiphanyAlignToN(address, size) \
((char*)((((uintptr_t)address) + (size - 1)) & ~(uintptr_t)(size - 1)))
#endif //! EpiphanyAlignToN


/// 对一个值进行n字节对齐.
#define EpiphanyDoAlign(x,b)   (((x) + (b) - 1U) & ~((b) - 1U))

#if HSL_BYTE_ALIGMENT == 8
#define BYTE_ALIGMENT_MASK ( 0x001f )
#endif //! HSL_BYTE_ALIGMENT

#define EpiphanyNull 0U

#define EpiphanyDefine(var_type, var_name)            \
    var_type var_name;                           \
    var_type d_##var_name() { return var_name; } \
    var_type &r_##var_name() { return var_name; }

#define EpiphanyCast(t, exp) ((t) (exp))

#define EpiphanyMax(a, b) ((a) < (b) ? (a) : (b))

#define EpiphanyMin(a, b) ((a) > (b) ? (a) : (b))

#define EpiphanyAarrayCopy(dest, src, n) \
    do {\
        for (size_t i = 0; i < (n); ++i)\
        {\
            (dest)[i] = (src)[i];\
        }\
    } while(0)


#define EpiphanyStrAppendZero(str, len) str[len] = '\0'

#define EpiphanyAllocStrByCopyInCodeFunc(dst, src, n) \
    do                                           \
    {                                            \
        EpiphanyAssert(dst);                          \
        EpiphanyCheckRtCode(dst, Epiphany_OpNullPtr);      \
        EpiphanyAssert(src);                          \
        EpiphanyCheckRtCode(src, Epiphany_OpNullPtr);      \
        EpiphanyAssert(n > 0);                        \
        EpiphanyCheckRtCode(n > 0, Epiphany_AllocZero);    \
        *dst = (char *)ndbMalloc(n);             \
        EpiphanyAssert(*dst);                         \
        EpiphanyCheckRtCode(*dst, Epiphany_OpNullPtr);     \
        ndbMemCpy(*dst, src, n);                 \
    } while (0)



#ifndef EPIPHANY_EPSILON
#define EPIPHANY_EPSILON (1E-6)
#endif //! EPIPHANY_EPSILON


#ifndef epiphany_min
#define epiphany_min(a, b) (((a) < (b)) ? (a) : (b))
#endif //! epiphany_min

#ifndef epiphany_safe_strlen_and_def
#define epiphany_safe_strlen_and_def(str, name)           \
    size_t name;   \
    do                                  \
    {                                   \
        name = strlen(str);   \
        EpiphanyAssert(name > 0);         \
        EpiphanyCheckRtPointer(name > 0); \
    } while (0)

#endif //! epiphany_safe_strlen_and_def


#ifdef _MSC_VER
    #define epiphany_likely(x) (x)
    #define epiphany_unlikely(x) (x)
#else // GNU 编译器(2.96及以上版本).
    #define epiphany_likely(x) __builtin_expect(!!(x), 1)
    #define epiphany_unlikely(x) __builtin_expect(!!(x), 0)
#endif //! 

/**
 * 花指令实现，用于代码混淆.
 * 注意: 仅在需要保护的发布版本中使用.
 */
#if defined(CODE_ENCRYPT)
    #if defined(_MSC_VER)
        #define EPIPHANY_OBFUSCATE __asm {               \
            _emit 0xEB;  /* jmp +1 */               \
            _emit 0xFF;  /* invalid */              \
            _emit 0xEB;  /* jmp +1 */               \
            _emit 0xFF;  /* invalid */              \
        }
    #elif defined(__GNUC__) || defined(__clang__)
        #define EPIPHANY_OBFUSCATE                       \
            __asm__ volatile(                       \
                ".byte 0xeb, 0x00\n\t"    /* jmp +0 */ \
                ".byte 0xeb, 0x00\n\t"    /* jmp +0 */ \
                ".byte 0xeb, 0x00\n\t"    /* jmp +0 */ \
            )
    #endif
#else
    #define EPIPHANY_OBFUSCATE
#endif

/**
 * 给定一个数, 计算存储这个数所需要的bit. 
 */
#define BITS_REQUIRED(n) ((n) == 0 ? 1 : ( \
    (n) & (1U << 31) ? 32 : \
    (n) & (1U << 30) ? 31 : \
    (n) & (1U << 29) ? 30 : \
    (n) & (1U << 28) ? 29 : \
    (n) & (1U << 27) ? 28 : \
    (n) & (1U << 26) ? 27 : \
    (n) & (1U << 25) ? 26 : \
    (n) & (1U << 24) ? 25 : \
    (n) & (1U << 23) ? 24 : \
    (n) & (1U << 22) ? 23 : \
    (n) & (1U << 21) ? 22 : \
    (n) & (1U << 20) ? 21 : \
    (n) & (1U << 19) ? 20 : \
    (n) & (1U << 18) ? 19 : \
    (n) & (1U << 17) ? 18 : \
    (n) & (1U << 16) ? 17 : \
    (n) & (1U << 15) ? 16 : \
    (n) & (1U << 14) ? 15 : \
    (n) & (1U << 13) ? 14 : \
    (n) & (1U << 12) ? 13 : \
    (n) & (1U << 11) ? 12 : \
    (n) & (1U << 10) ? 11 : \
    (n) & (1U << 9) ? 10 : \
    (n) & (1U << 8) ? 9 : \
    (n) & (1U << 7) ? 8 : \
    (n) & (1U << 6) ? 7 : \
    (n) & (1U << 5) ? 6 : \
    (n) & (1U << 4) ? 5 : \
    (n) & (1U << 3) ? 4 : \
    (n) & (1U << 2) ? 3 : \
    (n) & (1U << 1) ? 2 : 1))

    
/* copy from linux kernel.
 * 获取一个结构体成员相对于结构体首地址的偏移. 
 * 谨慎在国产操作系统中使用, 其0地址可能是不可访问的.
 */
#ifndef offsetof
#define	offsetof(type, member)	((size_t)(u_long)(&((type *)0)->member))
#endif

#define EPIPHANY_CLEAR(s) \
    memset(&(s), 0, sizeof(s))


#endif //UTIL_MACROS_HPP_
