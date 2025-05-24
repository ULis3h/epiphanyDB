# 配置模块设计
配置模块提供配置整个存储引擎的功能.

## 配置信息格式
配置信息可以是一个文件或者一个字符串, 使用等式的关系来进行配置, 配置字符串的格式如下:
```json
"cache_size=100MB,create=false,error_prefix="",eviction_target=80,"
"eviction_trigger=95,extensions=(),hazard_max=30,home_environment=false,"
"home_environment_priv=false,logging=false,multiprocess=false,"
"session_max=50,transaction=false,verbose=()";
```

## 配置模块设计思路
配置模块能够解析形如`cache_size=100MB`这样的等式, 一个最简单的方法就是使用状态机, 假设最开始为初始状态, 然后读取到`c`
字符, 进入另一个状态,这个状态专门来处理裸字符(不被引号包裹的字符序列), 当在这个处理字符的状态读取到`=`时,将进行状态转换, 回到初始状态, 继续读取, 当读取到数字又进入了处理裸字符的状态. 这样,只需要关注状态改变的契机, 就可以建立状态表来创建状态机去处理配置信息.


### 状态表
配置模块设计了5个状态表来处理配置信息.

#### struct
struct状态主要处理:  
    1. 空白字符(`空格\Tab\换行\回车`) -> `A_LOOP` (保持struct状态).  
    2. 标识符开始(`字母\下划线\数字\负号`) -> `A_BARE`/`A_NUMBARE`(转换到`bare`状态).  
    3. 字符串开始(`"`) -> `A_QUP` (转换到`string`状态).  
    4. 结构控制:  
        1) 等号/冒号(`=/:`) -> `A_VALUE`  
        2) 逗号(`,`) -> `A_NEXT`  
        3) 括号(`[{(`) -> `A_UP`  
        4) 括号(`)}]`) -> `A_DOWN`  
#### bare
`bare`状态主要处理:  
    1. 标识符字符 -> `A_LOOP` (保持`bare`状态)  
        - 字母  
        - 数字  
        - 下划线  
        - 其他有效字符  
    2. 分隔符 -> `A_UNBARE` (回到`struct`状态)  
        - 空白字符  
        - 等号/冒号  
        - 逗号  
        - 右括号  
    3. 其他字符 -> `A_BAD` (错误)  

#### string
`string`状态主要处理:  
    1. 普通字符 -> `A_LOOP` (保持`string`状态)  
        - 大多数可打印字符  
    2. 特殊字符  
        - 引号(`"`) -> `A_QDOWN` (回到`struct`状态)  
        - 反斜杠(`\`) -> `A_ESC` (转到`esc`状态)  
    3. `UTF-8`序列开始  
        - 2字节序列 -> `A_UTF8_2`  
        - 3字节序列 -> `A_UTF8_3`  
        - 4字节序列 -> `A_UTF8_4`  

#### esc
1. 有效转义字符 -> `A_UNESC` (回到`string`状态)  
    - \" (引号)  
    - \\ (反斜杠)  
    - \/ (斜杠)  
    - \b (退格)  
    - \f (换页)  
    - \n (换行)  
    - \r (回车)  
    - \t (制表符)  
    - \u (`Ucicode`)  
2. 其他字符 -> `A_BAD` (错误)  