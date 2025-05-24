#include <cstddef>
#include <cstring>
#include <cstdlib>
#include <cerrno>

#include "config.hpp"
#include "macros.hpp"
#include "epiphany_internal.hpp"


const char* 
__epiphany_confdfl_epiphany_open = 
"cache_size=100MB,create=false,error_prefix="",eviction_target=80,"
"eviction_trigger=95,extensions=(),hazard_max=30,home_environment=false,"
"home_environment_priv=false,logging=false,multiprocess=false,"
"session_max=50,transaction=false,verbose=()";



#define PUSH(i, t)                         \
    do                                     \
    {                                      \
        if (this->top_ == -1)              \
            this->top_ = this->depth_;     \
        if (this->top_ == this->depth_)    \
        {                                  \
            if (out->len > 0)              \
                return -1;                 \
            out->type = (t);               \
            out->str = (this->cur_ + (i)); \
        }                                  \
    } while (0)

#define CAP(i)                                                      \
    do                                                              \
    {                                                               \
        if (this->depth_ == this->top_)                             \
        {                                                           \
            out->len = (size_t)((this->cur_ + (i) + 1) - out->str); \
        }                                                           \
    } while (0)

typedef enum {
	A_LOOP, A_BAD, A_DOWN, A_UP, A_VALUE, A_NEXT, A_QDOWN, A_QUP,
	A_ESC, A_UNESC, A_BARE, A_NUMBARE, A_UNBARE, A_UTF8_2,
	A_UTF8_3, A_UTF8_4, A_UTF_CONTINUE
} CONFIG_ACTION;

/**
 * 待解析配置字符序列示例:
 *  cache_size = 100M # 配置缓存为100M.
 */


/** 
 * Dec  Hex  Char   EPIPHANY配置中的处理
 * --- ----- ------ ------------------------
 * 000  00   NUL    A_BAD
 * 001  01   SOH    A_BAD
 * 002  02   STX    A_BAD
 * 003  03   ETX    A_BAD
 * 004  04   EOT    A_BAD
 * 005  05   ENQ    A_BAD
 * 006  06   ACK    A_BAD
 * 007  07   BEL    A_BAD
 * 008  08   BS     A_BAD
 * 009  09   HT     A_LOOP  (Tab, 允许用于格式化)
 * 010  0A   LF     A_LOOP  (换行, 允许用于格式化)
 * 011  0B   VT     A_BAD
 * 012  0C   FF     A_BAD
 * 013  0D   CR     A_LOOP  (回车, 允许用于格式化)
 * 014  0E   SO     A_BAD
 * 015  0F   SI     A_BAD
 * 016  10   DLE    A_BAD
 * 017  11   DC1    A_BAD
 * 018  12   DC2    A_BAD
 * 019  13   DC3    A_BAD
 * 020  14   DC4    A_BAD
 * 021  15   NAK    A_BAD
 * 022  16   SYN    A_BAD
 * 023  17   ETB    A_BAD
 * 024  18   CAN    A_BAD
 * 025  19   EM     A_BAD
 * 026  1A   SUB    A_BAD
 * 027  1B   ESC    A_BAD
 * 028  1C   FS     A_BAD
 * 029  1D   GS     A_BAD
 * 030  1E   RS     A_BAD
 * 031  1F   US     A_BAD
 * 032  20   Space  A_LOOP  (空格, 允许用于格式化)
 * 033  21   !      A_BAD
 * 034  22   "      A_QUP   (字符串开始/结束)
 * 035  23   #      A_BAD
 * 036  24   $      A_BAD
 * 037  25   %      A_BAD
 * 038  26   &      A_BAD
 * 039  27   '      A_BAD
 * 040  28   (      A_UP    (嵌套结构开始)
 * 041  29   )      A_DOWN  (嵌套结构结束)
 * 042  2A   *      A_BAD
 * 043  2B   +      A_BAD
 * 044  2C   ,      A_NEXT  (配置项分隔符)
 * 045  2D   -      A_NUMBARE (数字的负号)
 * 046  2E   .      A_BAD
 * 047  2F   /      A_BAD
 * 048  30   0      A_NUMBARE (数字)
 * 049  31   1      A_NUMBARE
 * 050  32   2      A_NUMBARE
 * 051  33   3      A_NUMBARE
 * 052  34   4      A_NUMBARE
 * 053  35   5      A_NUMBARE
 * 054  36   6      A_NUMBARE
 * 055  37   7      A_NUMBARE
 * 056  38   8      A_NUMBARE
 * 057  39   9      A_NUMBARE
 * 058  3A   :      A_VALUE  (值开始标记)
 * 059  3B   ;      A_BAD
 * 060  3C   <      A_BAD
 * 061  3D   =      A_VALUE  (值开始标记)
 * 062  3E   >      A_BAD
 * 063  3F   ?      A_BAD
 * 064  40   @      A_BAD
 * 065  41   A      A_BARE   (标识符字符)
 * ...  ..   ...    A_BARE   (A-Z 都是标识符字符)
 * 090  5A   Z      A_BARE
 * 091  5B   [      A_UP    (嵌套结构开始)
 * 092  5C   \      A_ESC   (转义字符)
 * 093  5D   ]      A_DOWN  (嵌套结构结束)
 * 094  5E   ^      A_BAD
 * 095  5F   _      A_BARE  (标识符字符)
 * 096  60   `      A_BAD
 * 097  61   a      A_BARE  (标识符字符)
 * ...  ..   ...    A_BARE  (a-z 都是标识符字符)
 * 122  7A   z      A_BARE
 * 123  7B   {      A_UP    (嵌套结构开始)
 * 124  7C   |      A_BAD
 * 125  7D   }      A_DOWN  (嵌套结构结束)
 * 126  7E   ~      A_BAD
 * 127  7F   DEL    A_BAD
 * 
 * 128-255: 扩展ASCII字符
 * - UTF-8序列开始字符: A_UTF8_2, A_UTF8_3, A_UTF8_4
 * - UTF-8继续字节: A_UTF_CONTINUE
 * - 其他: A_BAD
 */

 /**
  * 配置解析器设置为以下4个状态表:
  *  - struct 解析整体结构. 
  *  - bare 解析裸标识符.
  *  - string 解析字符串.
  *  - esc 解析转义序列.
  * 
  * <状态转移说明>:
  * 
  * struct状态主要处理:
  *     1. 空白字符(空格\Tab\换行\回车) -> A_LOOP (保持struct状态).
  *     2. 标识符开始(字母\下划线\数字\负号) -> A_BARE/A_NUMBARE (转换到bare状态).
  *     3. 字符串开始(") -> A_QUP (转换到string状态).
  *     4. 结构控制:
  *         1) 等号/冒号(=/:) -> A_VALUE
  *         2) 逗号(,) -> A_NEXT
  *         3) 括号([{() -> A_UP
  *         4) 括号()}]) -> A_DOWN
  * 
  * bare状态主要处理:
  *     1. 标识符字符 -> A_LOOP (保持bare状态)
  *         - 字母
  *         - 数字
  *         - 下划线
  *         - 其他有效字符
  *     2. 分隔符 -> A_UNBARE (回到struct状态)
  *         - 空白字符
  *         - 等号/冒号
  *         - 逗号
  *         - 右括号
  *     3. 其他字符 -> A_BAD (错误)
  * 
  * string状态主要处理:
  *     1. 普通字符 -> A_LOOP (保持string状态)
  *         - 大多数可打印字符
  *     2. 特殊字符
  *         - 引号(") -> A_QDOWN (回到struct状态)
  *         - 反斜杠(\) -> A_ESC (转到esc状态)
  *     3. UTF-8序列开始
  *         - 2字节序列 -> A_UTF8_2
  *         - 3字节序列 -> A_UTF8_3
  *         - 4字节序列 -> A_UTF8_4
  * 
  * esc状态主要处理:
  *     1. 有效转义字符 -> A_UNESC (回到string状态)
  *         - \" (引号)
  *         - \\ (反斜杠)
  *         - \/ (斜杠)
  *         - \b (退格)
  *         - \f (换页)
  *         - \n (换行)
  *         - \r (回车)
  *         - \t (制表符)
  *         - \u (Ucicode)
  *     2. 其他字符 -> A_BAD (错误)
  * 
  */

static epiphany_int8 gostruct[256] = {
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_LOOP, A_LOOP, A_BAD, A_BAD, A_LOOP, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_LOOP, A_BAD, A_QUP,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_UP, A_DOWN, A_BAD, A_BAD,
	A_NEXT, A_NUMBARE, A_BAD, A_BAD, A_NUMBARE, A_NUMBARE,
	A_NUMBARE, A_NUMBARE, A_NUMBARE, A_NUMBARE, A_NUMBARE,
	A_NUMBARE, A_NUMBARE, A_NUMBARE, A_VALUE, A_BAD, A_BAD,
	A_VALUE, A_BAD, A_BAD, A_BAD, A_BARE, A_BARE, A_BARE, A_BARE,
	A_BARE, A_BARE, A_BARE, A_BARE, A_BARE, A_BARE, A_BARE, A_BARE,
	A_BARE, A_BARE, A_BARE, A_BARE, A_BARE, A_BARE, A_BARE, A_BARE,
	A_BARE, A_BARE, A_BARE, A_BARE, A_BARE, A_BARE, A_UP, A_BAD,
	A_DOWN, A_BAD, A_BARE, A_BAD, A_BARE, A_BARE, A_BARE, A_BARE,
	A_BARE, A_BARE, A_BARE, A_BARE, A_BARE, A_BARE, A_BARE, A_BARE,
	A_BARE, A_BARE, A_BARE, A_BARE, A_BARE, A_BARE, A_BARE, A_BARE,
	A_BARE, A_BARE, A_BARE, A_BARE, A_BARE, A_BARE, A_UP, A_BAD,
	A_DOWN, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD
};

static epiphany_int8 gobare[256] = {
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_UNBARE, A_UNBARE, A_BAD, A_BAD, A_UNBARE, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_UNBARE,
	A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP,
	A_UNBARE, A_LOOP, A_LOOP, A_UNBARE, A_LOOP, A_LOOP, A_LOOP,
	A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP,
	A_LOOP, A_LOOP, A_UNBARE, A_LOOP, A_LOOP, A_UNBARE, A_LOOP,
	A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP,
	A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP,
	A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP,
	A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_UNBARE,
	A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP,
	A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP,
	A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP,
	A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP,
	A_UNBARE, A_LOOP, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD
};

static epiphany_int8 gostring[256] = {
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_LOOP, A_LOOP, A_QDOWN,
	A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP,
	A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP,
	A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP,
	A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP,
	A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP,
	A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP,
	A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP,
	A_LOOP, A_ESC, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP,
	A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP,
	A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP,
	A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP,
	A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_UTF8_2,
	A_UTF8_2, A_UTF8_2, A_UTF8_2, A_UTF8_2, A_UTF8_2, A_UTF8_2,
	A_UTF8_2, A_UTF8_2, A_UTF8_2, A_UTF8_2, A_UTF8_2, A_UTF8_2,
	A_UTF8_2, A_UTF8_2, A_UTF8_2, A_UTF8_2, A_UTF8_2, A_UTF8_2,
	A_UTF8_2, A_UTF8_2, A_UTF8_2, A_UTF8_2, A_UTF8_2, A_UTF8_2,
	A_UTF8_2, A_UTF8_2, A_UTF8_2, A_UTF8_2, A_UTF8_2, A_UTF8_2,
	A_UTF8_2, A_UTF8_3, A_UTF8_3, A_UTF8_3, A_UTF8_3, A_UTF8_3,
	A_UTF8_3, A_UTF8_3, A_UTF8_3, A_UTF8_3, A_UTF8_3, A_UTF8_3,
	A_UTF8_3, A_UTF8_3, A_UTF8_3, A_UTF8_3, A_UTF8_3, A_UTF8_4,
	A_UTF8_4, A_UTF8_4, A_UTF8_4, A_UTF8_4, A_UTF8_4, A_UTF8_4,
	A_UTF8_4, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD
};

static epiphany_int8 goutf8_continue[256] = {
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_UTF_CONTINUE, A_UTF_CONTINUE, A_UTF_CONTINUE,
	A_UTF_CONTINUE, A_UTF_CONTINUE, A_UTF_CONTINUE, A_UTF_CONTINUE,
	A_UTF_CONTINUE, A_UTF_CONTINUE, A_UTF_CONTINUE, A_UTF_CONTINUE,
	A_UTF_CONTINUE, A_UTF_CONTINUE, A_UTF_CONTINUE, A_UTF_CONTINUE,
	A_UTF_CONTINUE, A_UTF_CONTINUE, A_UTF_CONTINUE, A_UTF_CONTINUE,
	A_UTF_CONTINUE, A_UTF_CONTINUE, A_UTF_CONTINUE, A_UTF_CONTINUE,
	A_UTF_CONTINUE, A_UTF_CONTINUE, A_UTF_CONTINUE, A_UTF_CONTINUE,
	A_UTF_CONTINUE, A_UTF_CONTINUE, A_UTF_CONTINUE, A_UTF_CONTINUE,
	A_UTF_CONTINUE, A_UTF_CONTINUE, A_UTF_CONTINUE, A_UTF_CONTINUE,
	A_UTF_CONTINUE, A_UTF_CONTINUE, A_UTF_CONTINUE, A_UTF_CONTINUE,
	A_UTF_CONTINUE, A_UTF_CONTINUE, A_UTF_CONTINUE, A_UTF_CONTINUE,
	A_UTF_CONTINUE, A_UTF_CONTINUE, A_UTF_CONTINUE, A_UTF_CONTINUE,
	A_UTF_CONTINUE, A_UTF_CONTINUE, A_UTF_CONTINUE, A_UTF_CONTINUE,
	A_UTF_CONTINUE, A_UTF_CONTINUE, A_UTF_CONTINUE, A_UTF_CONTINUE,
	A_UTF_CONTINUE, A_UTF_CONTINUE, A_UTF_CONTINUE, A_UTF_CONTINUE,
	A_UTF_CONTINUE, A_UTF_CONTINUE, A_UTF_CONTINUE, A_UTF_CONTINUE,
	A_UTF_CONTINUE, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD
};

static epiphany_int8 goesc[256] = {
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_UNESC,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_UNESC, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_UNESC, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_UNESC, A_BAD, A_BAD, A_BAD, A_UNESC, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_UNESC, A_BAD,
	A_BAD, A_BAD, A_UNESC, A_BAD, A_UNESC, A_UNESC, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
	A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD
};

EpiphanyConfig::EpiphanyConfig() 
{
}

EpiphanyConfig::~EpiphanyConfig()
{
}

int 
EpiphanyConfig::init(EpiphanySessionImpl* session, const char* str, size_t len)
{
    EpiphanyCheckRtInt(session);
    len = ( str == NULL ) ? 0 : strlen(str);

    this->session_ = session;
    this->orig_ = str;
    this->end_ = str + len;
    this->depth_ = 0;
    this->top_ = -1;
    this->go_ = NULL;

    return ( 0 );
}


int
EpiphanyConfig::process_value(void* value)
{
    EpiphanyCheckRtInt(value);

    char *endptr = NULL;
    EPIPHANY_CONFIG_ITEM* pValue = (EPIPHANY_CONFIG_ITEM*) value;

    if ( pValue->len == 0 ) {
        return 0;
    } 

    /**
     * 处理标识符.
     */
    if ( pValue->type == EPIPHANY_CONFIG_ITEM::ITEM_ID ) {
        if ( strncasecmp(pValue->str, "true", pValue->len) == 0 ) {
            pValue->type = EPIPHANY_CONFIG_ITEM::ITEM_NUM;
            pValue->val = 1;
        } else if ( strncasecmp(pValue->str, "false", pValue->len) == 0 ) {
            pValue->type = EPIPHANY_CONFIG_ITEM::ITEM_NUM;
            pValue->val = 0;
        }
    } else if ( pValue->type == EPIPHANY_CONFIG_ITEM::ITEM_NUM ) {
        errno = 0;
        pValue->val = strtoll(pValue->str, &endptr, 10);

        if ( errno == ERANGE ) {
            LogErr("Number Out Of Range", ERANGE);
            return ERANGE;
        }

        /** 检查剩余字符. */
        while ( endptr < pValue->str + pValue->len ) {
            switch ( *endptr++ ) {
                case L'b':
                case L'B':
                {
                    break;
                }
                case L'k':
                case L'K':
                {
                    pValue->val <<= 10;
                    break;
                }
                case L'm':
                case L'M':
                {
                    pValue->val <<= 20;
                    break;
                }
                case L'g':
                case L'G':
                {
                    pValue->val <<= 30;
                    break;
                }
                case L't':
                case L'T':
                {
                    pValue->val <<= 40;
                    break;
                }
                case L'p':
                case L'P':
                {
                    pValue->val <<= 50;
                    break;
                }
                default: {
                    pValue->type = EPIPHANY_CONFIG_ITEM::ITEM_ID;
                    break;
                }
            }
        }
    }

    return 0;
}

int 
EpiphanyConfig::next(void* key, void* value) 
{
    EpiphanyCheckNRtInt(key, value);
    
    EPIPHANY_CONFIG_ITEM* out =  (EPIPHANY_CONFIG_ITEM*) key;
    EPIPHANY_CONFIG_ITEM *pKey = (EPIPHANY_CONFIG_ITEM*) key;
    EPIPHANY_CONFIG_ITEM *pValue = (EPIPHANY_CONFIG_ITEM*) value;

    int utf8_remain = 0;
    static EPIPHANY_CONFIG_ITEM default_value = {
        "", 0, 1, EPIPHANY_CONFIG_ITEM::ITEM_NUM
    };

    pKey->len = 0;
    *pValue = default_value;

    if ( this->go_ == NULL ) {
        /** 初始状态表为gostruct. */
        this->go_ = gostruct;
    } 

    while ( this->cur_ < this->end_ ) {
        /** 转移规则见上面的状态转移说明. */
        switch ( this->go_[(int)*this->cur_] ) {
            case A_LOOP:
            {
                break;
            }
            case A_BAD:
            {
                /** 对于当前的状态表, 其cur的值是可预测的, 正常不会出现BAD, 
                 *  若出现,即为错误 
                 */
                LogErr("Unexpected character", 0);
                return -1;
            }
            case A_DOWN: 
            {
                --this->depth_;
                CAP(0);
                break;
            }
            case A_UP:
            {
                if ( this->top_ == -1 ) {
                    this->top_ = 1;
                }
                PUSH(0, EPIPHANY_CONFIG_ITEM::ITEM_STRUCT);
                ++this->depth_;
                break;
            }
            case A_VALUE:
            {
                if ( this->depth_ == this->top_ ) {
                    if ( out == value && *this->cur_ != ':' ) {
                        return -1;
                    }
                    out = (EPIPHANY_CONFIG_ITEM*) value;
                }
                break;
            }
            case A_NEXT:
            {
                if ( this->depth_ == this->top_ && pKey->len > 0 ) {
                    ++this->cur_;
                    goto val;
                } else {
                    break;
                }
            }
            case A_QDOWN:
            {
                CAP(-1);
                this->go_ = gostruct;
                break;
            }
            case A_QUP:
            {
                PUSH(1, EPIPHANY_CONFIG_ITEM::ITEM_STRING);
                this->go_ = gostring;
                break;
            }
            case A_ESC:
            {
                this->go_ = goesc;
                break;
            }
            case A_UNESC:
            {
                this->go_ = gostring;
                break;
            }
            case A_BARE:
            {
                PUSH(0, EPIPHANY_CONFIG_ITEM::ITEM_ID);
                this->go_ = gobare;
                break;
            }
            case A_NUMBARE:
            {
                this->go_ = gobare;
                break;
            }
            case A_UNBARE:
            {
                CAP(-1);
                this->go_ = gostruct;
                continue;
            }
            case A_UTF8_2:
            {
                this->go_ = goutf8_continue;
                utf8_remain = 1;
                break;
            }
            case A_UTF8_3:
            {
                this->go_ = goutf8_continue;
                utf8_remain = 2;
                break;
            }
            case A_UTF8_4:
            {
                this->go_ = goutf8_continue;
                utf8_remain = 3;
                break;
            }
            case A_UTF_CONTINUE: 
            {
                if ( !--utf8_remain ) {
                    this->go_ = gostring;
                }
                break;
            }
        }
        this->cur_++;
    }

    if ( this->depth_ == this->top_ && pKey->len > 0)
val:    return process_value(value);

    if ( this->depth_ == 0 ) {
        return -2;
    }

    return -1;
}

int 
EpiphanyConfig::get_raw(void* key, void* value)
{
    EPIPHANY_CONFIG_ITEM* pKey = (EPIPHANY_CONFIG_ITEM*) key;
    EPIPHANY_CONFIG_ITEM* pValue = (EPIPHANY_CONFIG_ITEM*) value;

    EPIPHANY_CONFIG_ITEM k;
    EPIPHANY_CONFIG_ITEM v;

    int ret = -1;

    while ( (ret = next(&k, &v)) == 0 )
    {
        if ( (k.type == EPIPHANY_CONFIG_ITEM::ITEM_STRING || 
              k.type == EPIPHANY_CONFIG_ITEM::ITEM_ID) && 
              k.len == pKey->len && strncasecmp(pKey->str, k.str, k.len) == 0) {
            *pValue = v;
            return ( 0 );
        } 
    }

    return ret;
}

int 
EpiphanyConfig::get(EpiphanySessionImpl* session, const char** cfg, 
               void* key, void* value)
{
    int found = 0;
    int ret = EPIPHANY_COMMON_GET_CONFIG_KEY_NOTFOUND;

    for ( found = 0; *cfg != NULL; cfg++ ) {
        EpiphanyCheckRtCode((ret = init(session, *cfg )), ret);
        if ( (ret = get_raw(key, value)) == 0 ) {
            found = 1;
        } else {
            return ( ret );
        }
    }

    return ( found ? 0 : ret );
}

int 
EpiphanyConfig::gets(EpiphanySessionImpl* session, const char** cfg, 
               void* key, void* value)
{
    EPIPHANY_CONFIG_ITEM key_item;

    key_item.type = EPIPHANY_CONFIG_ITEM::ITEM_STRING;
    key_item.str = (const char*) key;
    key_item.len = strlen((const char* )key);

    return ( get(session, cfg, &key_item, value) );
}

int 
EpiphanyConfig::getone(EpiphanySessionImpl* session, const char* cfg, void* key, 
           void* value)
{
    const char *cfgs[] = { cfg, NULL};
    return ( get(session, cfgs, key, value) ); 
}

int 
EpiphanyConfig::getones(EpiphanySessionImpl* session, const char* cfg, void* key, 
           void* value)
{
    const char *cfgs[] = { cfg, NULL};
    return ( gets(session, cfgs, key, value) ); 
}

int 
EpiphanyConfig::subgetraw(EpiphanySessionImpl* session, void* cfg, void* key, void* value)
{
    EPIPHANY_CONFIG_ITEM* pKey = (EPIPHANY_CONFIG_ITEM*) key;
    EPIPHANY_CONFIG_ITEM* it = (EPIPHANY_CONFIG_ITEM*) cfg;
    init(session, it->str);
    return ( get_raw(key, value) );
}

int 
EpiphanyConfig::subgets(EpiphanySessionImpl* session, void* cfg, void* key, void* value)
{
    EPIPHANY_CONFIG_ITEM* pKey = (EPIPHANY_CONFIG_ITEM*) key;
    
    pKey->str = (const char*) key;
    pKey->len = strlen((const char*) key);

    return ( subgetraw(session, cfg, key, value) );
}