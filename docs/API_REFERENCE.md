# EpiphanyDB API 参考文档

## 概述

本文档提供了 EpiphanyDB 的完整 API 参考，包括所有可用的函数、数据结构、枚举类型和使用示例。

## 头文件

```cpp
#include "epiphany_v2.h"
```

## 数据类型和枚举

### 结果代码 (EPIPHANY_RESULT_CODE)

```cpp
typedef enum {
    EPIPHANY_SUCCESS = 0,           // 操作成功
    EPIPHANY_ERROR_INVALID_PARAM,   // 无效参数
    EPIPHANY_ERROR_OUT_OF_MEMORY,   // 内存不足
    EPIPHANY_ERROR_CONNECTION,      // 连接错误
    EPIPHANY_ERROR_SQL_SYNTAX,      // SQL 语法错误
    EPIPHANY_ERROR_TABLE_NOT_FOUND, // 表不存在
    EPIPHANY_ERROR_COLUMN_NOT_FOUND,// 列不存在
    EPIPHANY_ERROR_CONSTRAINT,      // 约束违反
    EPIPHANY_ERROR_TRANSACTION,     // 事务错误
    EPIPHANY_ERROR_TIMEOUT,         // 操作超时
    EPIPHANY_ERROR_PERMISSION,      // 权限不足
    EPIPHANY_ERROR_INTERNAL         // 内部错误
} EPIPHANY_RESULT_CODE;
```

### 存储模式 (EPIPHANY_STORAGE_MODE)

```cpp
typedef enum {
    EPIPHANY_STORAGE_ROW = 0,       // 行存储模式
    EPIPHANY_STORAGE_COLUMN,        // 列存储模式
    EPIPHANY_STORAGE_VECTOR,        // 向量存储模式
    EPIPHANY_STORAGE_TIMESERIES,    // 时序存储模式
    EPIPHANY_STORAGE_AUTO           // 自动选择模式
} EPIPHANY_STORAGE_MODE;
```

### 数据类型 (EPIPHANY_TYPE)

```cpp
typedef enum {
    EPIPHANY_TYPE_NULL = 0,         // 空值
    EPIPHANY_TYPE_INTEGER,          // 整数
    EPIPHANY_TYPE_FLOAT,            // 浮点数
    EPIPHANY_TYPE_DOUBLE,           // 双精度浮点数
    EPIPHANY_TYPE_STRING,           // 字符串
    EPIPHANY_TYPE_BLOB,             // 二进制数据
    EPIPHANY_TYPE_VECTOR,           // 向量
    EPIPHANY_TYPE_TIMESTAMP,        // 时间戳
    EPIPHANY_TYPE_DATE,             // 日期
    EPIPHANY_TYPE_TIME,             // 时间
    EPIPHANY_TYPE_BOOLEAN           // 布尔值
} EPIPHANY_TYPE;
```

### 索引类型 (EPIPHANY_INDEX_TYPE)

```cpp
typedef enum {
    EPIPHANY_INDEX_BTREE = 0,       // B树索引
    EPIPHANY_INDEX_HASH,            // 哈希索引
    EPIPHANY_INDEX_VECTOR_HNSW,     // HNSW向量索引
    EPIPHANY_INDEX_VECTOR_IVF,      // IVF向量索引
    EPIPHANY_INDEX_FULLTEXT         // 全文索引
} EPIPHANY_INDEX_TYPE;
```

### 句柄类型

```cpp
typedef struct EPIPHANY_INSTANCE_IMPL* EPIPHANY_INSTANCE;
typedef struct EPIPHANY_CONNECTION_IMPL* EPIPHANY_CONNECTION;
typedef struct EPIPHANY_SESSION_IMPL* EPIPHANY_SESSION;
typedef struct EPIPHANY_STATEMENT_IMPL* EPIPHANY_STATEMENT;
typedef struct EPIPHANY_RESULT_IMPL* EPIPHANY_RESULT;
```

## 核心 API 函数

### 实例管理

#### epiphany_create_instance

创建数据库实例。

```cpp
EPIPHANY_RESULT_CODE epiphany_create_instance(EPIPHANY_INSTANCE* instance);
```

**参数:**
- `instance`: 输出参数，返回创建的实例句柄

**返回值:**
- `EPIPHANY_SUCCESS`: 创建成功
- `EPIPHANY_ERROR_OUT_OF_MEMORY`: 内存不足
- `EPIPHANY_ERROR_INTERNAL`: 内部错误

**示例:**
```cpp
EPIPHANY_INSTANCE instance;
EPIPHANY_RESULT_CODE result = epiphany_create_instance(&instance);
if (result == EPIPHANY_SUCCESS) {
    // 实例创建成功
}
```

#### epiphany_destroy_instance

销毁数据库实例。

```cpp
EPIPHANY_RESULT_CODE epiphany_destroy_instance(EPIPHANY_INSTANCE instance);
```

**参数:**
- `instance`: 要销毁的实例句柄

**返回值:**
- `EPIPHANY_SUCCESS`: 销毁成功
- `EPIPHANY_ERROR_INVALID_PARAM`: 无效参数

### 连接管理

#### epiphany_connect

建立数据库连接。

```cpp
EPIPHANY_RESULT_CODE epiphany_connect(
    EPIPHANY_INSTANCE instance,
    const char* connection_string,
    EPIPHANY_CONNECTION* connection
);
```

**参数:**
- `instance`: 数据库实例句柄
- `connection_string`: 连接字符串
- `connection`: 输出参数，返回连接句柄

**连接字符串格式:**
```
"host=localhost;port=5432;database=mydb;user=username;password=password"
```

**返回值:**
- `EPIPHANY_SUCCESS`: 连接成功
- `EPIPHANY_ERROR_CONNECTION`: 连接失败
- `EPIPHANY_ERROR_INVALID_PARAM`: 无效参数

**示例:**
```cpp
EPIPHANY_CONNECTION connection;
const char* conn_str = "host=localhost;database=testdb";
EPIPHANY_RESULT_CODE result = epiphany_connect(instance, conn_str, &connection);
```

#### epiphany_disconnect

断开数据库连接。

```cpp
EPIPHANY_RESULT_CODE epiphany_disconnect(EPIPHANY_CONNECTION connection);
```

### 会话管理

#### epiphany_create_session

创建数据库会话。

```cpp
EPIPHANY_RESULT_CODE epiphany_create_session(
    EPIPHANY_CONNECTION connection,
    EPIPHANY_SESSION* session
);
```

#### epiphany_close_session

关闭数据库会话。

```cpp
EPIPHANY_RESULT_CODE epiphany_close_session(EPIPHANY_SESSION session);
```

### SQL 执行

#### epiphany_execute_sql

执行 SQL 语句。

```cpp
EPIPHANY_RESULT_CODE epiphany_execute_sql(
    EPIPHANY_SESSION session,
    const char* sql,
    EPIPHANY_RESULT* result
);
```

**参数:**
- `session`: 会话句柄
- `sql`: SQL 语句字符串
- `result`: 输出参数，返回结果集句柄

**返回值:**
- `EPIPHANY_SUCCESS`: 执行成功
- `EPIPHANY_ERROR_SQL_SYNTAX`: SQL 语法错误
- `EPIPHANY_ERROR_TABLE_NOT_FOUND`: 表不存在

**示例:**
```cpp
EPIPHANY_RESULT result;
const char* sql = "SELECT * FROM users WHERE age > 25";
EPIPHANY_RESULT_CODE code = epiphany_execute_sql(session, sql, &result);
```

#### epiphany_prepare_statement

预编译 SQL 语句。

```cpp
EPIPHANY_RESULT_CODE epiphany_prepare_statement(
    EPIPHANY_SESSION session,
    const char* sql,
    EPIPHANY_STATEMENT* statement
);
```

#### epiphany_execute_statement

执行预编译语句。

```cpp
EPIPHANY_RESULT_CODE epiphany_execute_statement(
    EPIPHANY_STATEMENT statement,
    EPIPHANY_RESULT* result
);
```

### 参数绑定

#### epiphany_bind_integer

绑定整数参数。

```cpp
EPIPHANY_RESULT_CODE epiphany_bind_integer(
    EPIPHANY_STATEMENT statement,
    int param_index,
    int value
);
```

#### epiphany_bind_string

绑定字符串参数。

```cpp
EPIPHANY_RESULT_CODE epiphany_bind_string(
    EPIPHANY_STATEMENT statement,
    int param_index,
    const char* value
);
```

#### epiphany_bind_vector

绑定向量参数。

```cpp
EPIPHANY_RESULT_CODE epiphany_bind_vector(
    EPIPHANY_STATEMENT statement,
    int param_index,
    const float* vector,
    int dimension
);
```

### 结果集处理

#### epiphany_fetch_row

获取下一行数据。

```cpp
EPIPHANY_RESULT_CODE epiphany_fetch_row(EPIPHANY_RESULT result);
```

#### epiphany_get_column_count

获取列数。

```cpp
int epiphany_get_column_count(EPIPHANY_RESULT result);
```

#### epiphany_get_column_name

获取列名。

```cpp
const char* epiphany_get_column_name(EPIPHANY_RESULT result, int column_index);
```

#### epiphany_get_column_type

获取列类型。

```cpp
EPIPHANY_TYPE epiphany_get_column_type(EPIPHANY_RESULT result, int column_index);
```

#### epiphany_get_integer

获取整数值。

```cpp
EPIPHANY_RESULT_CODE epiphany_get_integer(
    EPIPHANY_RESULT result,
    int column_index,
    int* value
);
```

#### epiphany_get_string

获取字符串值。

```cpp
EPIPHANY_RESULT_CODE epiphany_get_string(
    EPIPHANY_RESULT result,
    int column_index,
    const char** value
);
```

#### epiphany_get_vector

获取向量值。

```cpp
EPIPHANY_RESULT_CODE epiphany_get_vector(
    EPIPHANY_RESULT result,
    int column_index,
    const float** vector,
    int* dimension
);
```

### 事务管理

#### epiphany_begin_transaction

开始事务。

```cpp
EPIPHANY_RESULT_CODE epiphany_begin_transaction(EPIPHANY_SESSION session);
```

#### epiphany_commit_transaction

提交事务。

```cpp
EPIPHANY_RESULT_CODE epiphany_commit_transaction(EPIPHANY_SESSION session);
```

#### epiphany_rollback_transaction

回滚事务。

```cpp
EPIPHANY_RESULT_CODE epiphany_rollback_transaction(EPIPHANY_SESSION session);
```

### 表管理

#### epiphany_create_table

创建表。

```cpp
EPIPHANY_RESULT_CODE epiphany_create_table(
    EPIPHANY_SESSION session,
    const char* table_name,
    const char* schema,
    EPIPHANY_STORAGE_MODE storage_mode
);
```

**示例:**
```cpp
const char* schema = 
    "id INTEGER PRIMARY KEY, "
    "name VARCHAR(100), "
    "vector VECTOR(512)";
    
epiphany_create_table(session, "embeddings", schema, EPIPHANY_STORAGE_VECTOR);
```

#### epiphany_drop_table

删除表。

```cpp
EPIPHANY_RESULT_CODE epiphany_drop_table(
    EPIPHANY_SESSION session,
    const char* table_name
);
```

### 索引管理

#### epiphany_create_index

创建索引。

```cpp
EPIPHANY_RESULT_CODE epiphany_create_index(
    EPIPHANY_SESSION session,
    const char* index_name,
    const char* table_name,
    const char* column_name,
    EPIPHANY_INDEX_TYPE index_type
);
```

#### epiphany_drop_index

删除索引。

```cpp
EPIPHANY_RESULT_CODE epiphany_drop_index(
    EPIPHANY_SESSION session,
    const char* index_name
);
```

### 配置管理

#### epiphany_set_config

设置配置参数。

```cpp
EPIPHANY_RESULT_CODE epiphany_set_config(
    EPIPHANY_INSTANCE instance,
    const char* key,
    const char* value
);
```

**常用配置参数:**
- `buffer_pool_size`: 缓冲池大小 (MB)
- `query_cache_size`: 查询缓存大小 (MB)
- `max_connections`: 最大连接数
- `log_level`: 日志级别 (DEBUG, INFO, WARN, ERROR)

#### epiphany_get_config

获取配置参数。

```cpp
EPIPHANY_RESULT_CODE epiphany_get_config(
    EPIPHANY_INSTANCE instance,
    const char* key,
    char* value,
    int value_size
);
```

### 统计信息

#### epiphany_get_table_stats

获取表统计信息。

```cpp
typedef struct {
    long long row_count;        // 行数
    long long size_bytes;       // 大小(字节)
    double compression_ratio;   // 压缩比
    long long index_count;      // 索引数量
} EPIPHANY_TABLE_STATS;

EPIPHANY_RESULT_CODE epiphany_get_table_stats(
    EPIPHANY_SESSION session,
    const char* table_name,
    EPIPHANY_TABLE_STATS* stats
);
```

#### epiphany_get_query_stats

获取查询统计信息。

```cpp
typedef struct {
    long long execution_count;  // 执行次数
    double avg_execution_time;  // 平均执行时间(ms)
    double total_execution_time;// 总执行时间(ms)
    long long rows_examined;    // 扫描行数
    long long rows_returned;    // 返回行数
} EPIPHANY_QUERY_STATS;

EPIPHANY_RESULT_CODE epiphany_get_query_stats(
    EPIPHANY_SESSION session,
    const char* sql_hash,
    EPIPHANY_QUERY_STATS* stats
);
```

## 高级功能 API

### 向量操作

#### epiphany_vector_similarity_search

向量相似性搜索。

```cpp
EPIPHANY_RESULT_CODE epiphany_vector_similarity_search(
    EPIPHANY_SESSION session,
    const char* table_name,
    const char* vector_column,
    const float* query_vector,
    int dimension,
    int top_k,
    EPIPHANY_RESULT* result
);
```

#### epiphany_vector_clustering

向量聚类。

```cpp
EPIPHANY_RESULT_CODE epiphany_vector_clustering(
    EPIPHANY_SESSION session,
    const char* table_name,
    const char* vector_column,
    int cluster_count,
    EPIPHANY_RESULT* result
);
```

### 时序数据操作

#### epiphany_timeseries_aggregate

时序数据聚合。

```cpp
EPIPHANY_RESULT_CODE epiphany_timeseries_aggregate(
    EPIPHANY_SESSION session,
    const char* table_name,
    const char* time_column,
    const char* value_column,
    const char* aggregate_function,
    const char* time_interval,
    EPIPHANY_RESULT* result
);
```

### 批量操作

#### epiphany_batch_insert

批量插入数据。

```cpp
EPIPHANY_RESULT_CODE epiphany_batch_insert(
    EPIPHANY_SESSION session,
    const char* table_name,
    const char** columns,
    int column_count,
    void** data,
    EPIPHANY_TYPE* types,
    int row_count
);
```

## 错误处理

### epiphany_get_error_message

获取错误消息。

```cpp
const char* epiphany_get_error_message(EPIPHANY_RESULT_CODE error_code);
```

### epiphany_get_last_error

获取最后一个错误。

```cpp
EPIPHANY_RESULT_CODE epiphany_get_last_error(
    EPIPHANY_SESSION session,
    char* error_message,
    int message_size
);
```

## 完整使用示例

### 基本查询示例

```cpp
#include "epiphany_v2.h"
#include <stdio.h>

int main() {
    EPIPHANY_INSTANCE instance;
    EPIPHANY_CONNECTION connection;
    EPIPHANY_SESSION session;
    EPIPHANY_RESULT result;
    
    // 创建实例
    if (epiphany_create_instance(&instance) != EPIPHANY_SUCCESS) {
        printf("Failed to create instance\\n");
        return 1;
    }
    
    // 建立连接
    if (epiphany_connect(instance, "host=localhost;database=testdb", 
                        &connection) != EPIPHANY_SUCCESS) {
        printf("Failed to connect\\n");
        return 1;
    }
    
    // 创建会话
    if (epiphany_create_session(connection, &session) != EPIPHANY_SUCCESS) {
        printf("Failed to create session\\n");
        return 1;
    }
    
    // 执行查询
    if (epiphany_execute_sql(session, "SELECT * FROM users", 
                            &result) == EPIPHANY_SUCCESS) {
        // 处理结果
        while (epiphany_fetch_row(result) == EPIPHANY_SUCCESS) {
            int id;
            const char* name;
            
            epiphany_get_integer(result, 0, &id);
            epiphany_get_string(result, 1, &name);
            
            printf("ID: %d, Name: %s\\n", id, name);
        }
        
        epiphany_free_result(result);
    }
    
    // 清理资源
    epiphany_close_session(session);
    epiphany_disconnect(connection);
    epiphany_destroy_instance(instance);
    
    return 0;
}
```

### 向量搜索示例

```cpp
#include "epiphany_v2.h"
#include <stdio.h>

int main() {
    EPIPHANY_INSTANCE instance;
    EPIPHANY_CONNECTION connection;
    EPIPHANY_SESSION session;
    EPIPHANY_RESULT result;
    
    // 初始化连接 (省略错误检查)
    epiphany_create_instance(&instance);
    epiphany_connect(instance, "host=localhost;database=vectordb", &connection);
    epiphany_create_session(connection, &session);
    
    // 创建向量表
    const char* create_table_sql = 
        "CREATE TABLE embeddings ("
        "id INTEGER PRIMARY KEY, "
        "vector VECTOR(128), "
        "metadata TEXT"
        ") STORAGE_MODE = VECTOR";
    
    epiphany_execute_sql(session, create_table_sql, NULL);
    
    // 创建向量索引
    epiphany_create_index(session, "idx_vector", "embeddings", 
                         "vector", EPIPHANY_INDEX_VECTOR_HNSW);
    
    // 向量相似性搜索
    float query_vector[128] = {0.1, 0.2, 0.3, /* ... */};
    
    if (epiphany_vector_similarity_search(session, "embeddings", "vector",
                                         query_vector, 128, 10, 
                                         &result) == EPIPHANY_SUCCESS) {
        printf("Top 10 similar vectors:\\n");
        while (epiphany_fetch_row(result) == EPIPHANY_SUCCESS) {
            int id;
            const char* metadata;
            
            epiphany_get_integer(result, 0, &id);
            epiphany_get_string(result, 2, &metadata);
            
            printf("ID: %d, Metadata: %s\\n", id, metadata);
        }
        
        epiphany_free_result(result);
    }
    
    // 清理资源
    epiphany_close_session(session);
    epiphany_disconnect(connection);
    epiphany_destroy_instance(instance);
    
    return 0;
}
```

## 注意事项

1. **内存管理**: 所有通过 API 分配的资源都需要显式释放
2. **线程安全**: 连接和会话对象不是线程安全的，需要在应用层进行同步
3. **错误处理**: 始终检查函数返回值，并适当处理错误情况
4. **参数验证**: API 会验证输入参数，无效参数会返回相应错误码
5. **资源限制**: 注意连接数、内存使用等资源限制

---

*本 API 参考文档涵盖了 EpiphanyDB 的主要功能，更多详细信息请参考源代码和示例程序。*