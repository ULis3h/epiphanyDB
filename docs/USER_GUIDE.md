# EpiphanyDB 用户指南

## 概述

EpiphanyDB 是一个创新的超融合数据库系统，集成了多种数据存储模式和查询能力，为现代应用提供统一的数据管理解决方案。

## 核心特性

### 🔄 多模态存储
- **行存储 (Row Store)**: 适用于 OLTP 事务处理
- **列存储 (Column Store)**: 优化分析查询性能
- **向量存储 (Vector Store)**: 支持 AI/ML 应用的向量相似性搜索
- **时序存储 (Time Series Store)**: 专为时间序列数据优化

### 🚀 统一查询接口
- 标准 SQL 查询支持
- 向量相似性搜索
- 时序数据分析
- 跨模态联合查询

### ⚡ 性能优化
- 自适应存储布局优化
- 智能数据压缩
- 多级缓存系统
- 并行查询执行
- 实时数据处理

## 快速开始

### 1. 编译安装

```bash
# 克隆项目
git clone <repository-url>
cd epiphanyDB

# 创建构建目录
mkdir build && cd build

# 配置和编译
cmake ..
make -j$(nproc)

# 运行测试
make test
```

### 2. 基本使用

#### 初始化数据库连接

```cpp
#include "epiphany_v2.h"

// 创建数据库实例
EPIPHANY_INSTANCE* instance = nullptr;
EPIPHANY_RESULT_CODE result = epiphany_create_instance(&instance);

if (result == EPIPHANY_SUCCESS) {
    // 连接成功
    printf("数据库连接成功\\n");
} else {
    // 处理错误
    printf("连接失败: %d\\n", result);
}
```

#### 创建表

```cpp
// 创建行存储表 (适用于事务处理)
const char* create_users_sql = 
    "CREATE TABLE users ("
    "id INTEGER PRIMARY KEY, "
    "name VARCHAR(100), "
    "email VARCHAR(255), "
    "age INTEGER"
    ") STORAGE_MODE = ROW";

// 创建列存储表 (适用于分析查询)
const char* create_analytics_sql = 
    "CREATE TABLE sales_data ("
    "date DATE, "
    "product_id INTEGER, "
    "revenue DECIMAL(10,2), "
    "quantity INTEGER"
    ") STORAGE_MODE = COLUMN";

// 创建向量表 (适用于 AI/ML)
const char* create_vectors_sql = 
    "CREATE TABLE embeddings ("
    "id INTEGER PRIMARY KEY, "
    "vector VECTOR(512), "
    "metadata TEXT"
    ") STORAGE_MODE = VECTOR";
```

#### 数据操作

```cpp
// 插入数据
const char* insert_sql = 
    "INSERT INTO users (name, email, age) VALUES (?, ?, ?)";

// 查询数据
const char* select_sql = 
    "SELECT * FROM users WHERE age > 25";

// 向量相似性搜索
const char* vector_search_sql = 
    "SELECT id, metadata FROM embeddings "
    "ORDER BY vector <-> ? LIMIT 10";
```

## 高级功能

### 索引管理

```cpp
// 创建 B 树索引
"CREATE INDEX idx_user_id ON users(id) USING BTREE"

// 创建哈希索引
"CREATE INDEX idx_user_email ON users(email) USING HASH"

// 创建向量索引
"CREATE INDEX idx_vector ON embeddings(vector) USING VECTOR"
```

### 性能优化

```cpp
// 优化存储布局
"OPTIMIZE TABLE users STORAGE_LAYOUT"

// 压缩数据
"COMPRESS TABLE sales_data"

// 更新统计信息
"ANALYZE TABLE users"
```

### 实时数据处理

```cpp
// 实时数据插入
"INSERT INTO metrics (timestamp, sensor_id, value) "
"VALUES (NOW(), ?, ?)"

// 实时聚合查询
"SELECT AVG(value) FROM metrics "
"WHERE timestamp > NOW() - INTERVAL 1 MINUTE "
"GROUP BY sensor_id"
```

## API 参考

### 核心函数

#### `epiphany_create_instance`
创建数据库实例

```cpp
EPIPHANY_RESULT_CODE epiphany_create_instance(EPIPHANY_INSTANCE** instance);
```

#### `epiphany_connect`
建立数据库连接

```cpp
EPIPHANY_RESULT_CODE epiphany_connect(
    EPIPHANY_INSTANCE* instance,
    const char* connection_string,
    EPIPHANY_CONNECTION** connection
);
```

#### `epiphany_execute_sql`
执行 SQL 语句

```cpp
EPIPHANY_RESULT_CODE epiphany_execute_sql(
    EPIPHANY_CONNECTION* connection,
    const char* sql,
    EPIPHANY_RESULT** result
);
```

### 存储模式枚举

```cpp
typedef enum {
    EPIPHANY_STORAGE_ROW,        // 行存储
    EPIPHANY_STORAGE_COLUMN,     // 列存储
    EPIPHANY_STORAGE_VECTOR,     // 向量存储
    EPIPHANY_STORAGE_TIMESERIES  // 时序存储
} EPIPHANY_STORAGE_MODE;
```

### 数据类型支持

```cpp
typedef enum {
    EPIPHANY_TYPE_INTEGER,    // 整数
    EPIPHANY_TYPE_FLOAT,      // 浮点数
    EPIPHANY_TYPE_STRING,     // 字符串
    EPIPHANY_TYPE_VECTOR,     // 向量
    EPIPHANY_TYPE_TIMESTAMP   // 时间戳
} EPIPHANY_TYPE;
```

## 最佳实践

### 1. 选择合适的存储模式

- **行存储**: 用于频繁的增删改操作，如用户管理、订单处理
- **列存储**: 用于分析查询，如报表生成、数据挖掘
- **向量存储**: 用于 AI/ML 应用，如推荐系统、相似性搜索
- **时序存储**: 用于时间序列数据，如监控指标、IoT 数据

### 2. 索引策略

- 为经常查询的字段创建索引
- 选择合适的索引类型 (B树、哈希、向量)
- 定期维护和优化索引

### 3. 性能调优

- 使用 `ANALYZE` 更新表统计信息
- 定期运行 `OPTIMIZE` 优化存储布局
- 启用数据压缩以节省存储空间
- 合理配置缓存大小

### 4. 查询优化

- 使用预编译语句提高性能
- 避免全表扫描，合理使用索引
- 对于复杂查询，检查执行计划
- 利用并行查询能力

## 示例程序

### 简单示例

运行基础功能演示：

```bash
cd build
./examples/simple_demo
```

### 完整演示

运行超融合数据库完整功能演示：

```bash
cd build
./examples/hyperconverged_demo
```

## 配置选项

### 内存配置

```cpp
// 设置缓冲池大小 (MB)
epiphany_set_config("buffer_pool_size", "512");

// 设置查询缓存大小 (MB)
epiphany_set_config("query_cache_size", "128");
```

### 并发配置

```cpp
// 设置最大连接数
epiphany_set_config("max_connections", "100");

// 设置查询并行度
epiphany_set_config("query_parallelism", "4");
```

### 存储配置

```cpp
// 启用数据压缩
epiphany_set_config("enable_compression", "true");

// 设置压缩算法
epiphany_set_config("compression_algorithm", "lz4");
```

## 故障排除

### 常见问题

1. **连接失败**
   - 检查数据库服务是否启动
   - 验证连接字符串格式
   - 确认网络连接正常

2. **查询性能慢**
   - 检查是否有合适的索引
   - 分析查询执行计划
   - 考虑优化存储布局

3. **内存不足**
   - 调整缓冲池大小
   - 启用数据压缩
   - 优化查询复杂度

### 日志和调试

```cpp
// 启用详细日志
epiphany_set_config("log_level", "DEBUG");

// 设置日志文件路径
epiphany_set_config("log_file", "/var/log/epiphany.log");
```

## 更多资源

- [架构设计文档](design/hyperconverged_architecture.md)
- [API 完整参考](include/epiphany_v2.h)
- [示例代码](examples/)
- [性能基准测试](docs/benchmarks/)

## 支持和反馈

如果您在使用过程中遇到问题或有改进建议，请：

1. 查看本用户指南和 FAQ
2. 检查示例代码
3. 提交 Issue 或 Pull Request
4. 联系开发团队

---

*EpiphanyDB - 下一代超融合数据库系统*