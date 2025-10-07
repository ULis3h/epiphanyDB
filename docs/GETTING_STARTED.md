# EpiphanyDB 快速入门指南

欢迎使用 EpiphanyDB！本指南将帮助您快速了解和使用这个超融合数据库系统。

## 什么是 EpiphanyDB？

EpiphanyDB 是一个创新的超融合数据库系统，它将多种数据存储模式（行存储、列存储、向量存储、时序存储）统一在一个系统中，提供统一的查询接口和优异的性能表现。

### 核心特性

- **多模态存储**：支持行存储、列存储、向量存储和时序存储
- **统一查询接口**：使用标准 SQL 语法访问所有数据类型
- **高性能**：智能查询优化、自适应存储、数据压缩
- **易于使用**：简单的 API 设计，丰富的文档和示例

## 系统要求

### 最低要求
- **操作系统**：Linux、macOS 或 Windows
- **内存**：至少 2GB RAM
- **存储**：至少 1GB 可用磁盘空间
- **编译器**：支持 C++11 的编译器（GCC 4.8+、Clang 3.3+、MSVC 2015+）

### 推荐配置
- **内存**：8GB+ RAM
- **存储**：SSD 存储，10GB+ 可用空间
- **CPU**：多核处理器

## 安装步骤

### 1. 获取源代码

```bash
# 克隆仓库
git clone https://github.com/your-org/epiphanyDB.git
cd epiphanyDB
```

### 2. 安装依赖

#### Ubuntu/Debian
```bash
sudo apt-get update
sudo apt-get install build-essential cmake git
```

#### macOS
```bash
# 使用 Homebrew
brew install cmake git

# 或使用 Xcode Command Line Tools
xcode-select --install
```

#### CentOS/RHEL
```bash
sudo yum groupinstall "Development Tools"
sudo yum install cmake git
```

### 3. 编译安装

```bash
# 创建构建目录
mkdir build && cd build

# 配置构建
cmake ..

# 编译
make -j$(nproc)

# 可选：运行测试
make test

# 安装（可选）
sudo make install
```

## 第一个程序

让我们创建一个简单的程序来体验 EpiphanyDB 的基本功能。

### 1. 创建源文件

创建文件 `hello_epiphany.cpp`：

```cpp
#include <iostream>
#include <epiphany/epiphany.h>

int main() {
    // 初始化 EpiphanyDB 实例
    EPIPHANY_INSTANCE* instance = nullptr;
    EPIPHANY_RESULT result = epiphany_create_instance(&instance);
    
    if (result != EPIPHANY_SUCCESS) {
        std::cerr << "Failed to create EpiphanyDB instance" << std::endl;
        return 1;
    }
    
    std::cout << "EpiphanyDB instance created successfully!" << std::endl;
    
    // 创建会话
    EPIPHANY_SESSION* session = nullptr;
    result = epiphany_create_session(instance, &session);
    
    if (result != EPIPHANY_SUCCESS) {
        std::cerr << "Failed to create session" << std::endl;
        epiphany_destroy_instance(instance);
        return 1;
    }
    
    std::cout << "Session created successfully!" << std::endl;
    
    // 创建一个简单的表
    const char* create_table_sql = 
        "CREATE TABLE users ("
        "id INTEGER PRIMARY KEY, "
        "name VARCHAR(100), "
        "email VARCHAR(255)"
        ") STORAGE_MODE = ROW";
    
    EPIPHANY_RESULT* query_result = nullptr;
    result = epiphany_execute_sql(session, create_table_sql, &query_result);
    
    if (result == EPIPHANY_SUCCESS) {
        std::cout << "Table 'users' created successfully!" << std::endl;
        epiphany_free_result(query_result);
    } else {
        std::cout << "Failed to create table" << std::endl;
    }
    
    // 插入数据
    const char* insert_sql = 
        "INSERT INTO users (id, name, email) VALUES "
        "(1, 'Alice', 'alice@example.com'), "
        "(2, 'Bob', 'bob@example.com')";
    
    result = epiphany_execute_sql(session, insert_sql, &query_result);
    
    if (result == EPIPHANY_SUCCESS) {
        std::cout << "Data inserted successfully!" << std::endl;
        epiphany_free_result(query_result);
    }
    
    // 查询数据
    const char* select_sql = "SELECT * FROM users";
    result = epiphany_execute_sql(session, select_sql, &query_result);
    
    if (result == EPIPHANY_SUCCESS) {
        std::cout << "Query results:" << std::endl;
        
        // 获取结果集信息
        int row_count = epiphany_get_row_count(query_result);
        int col_count = epiphany_get_column_count(query_result);
        
        std::cout << "Found " << row_count << " rows, " << col_count << " columns" << std::endl;
        
        // 打印列名
        for (int i = 0; i < col_count; i++) {
            const char* col_name = epiphany_get_column_name(query_result, i);
            std::cout << col_name << "\t";
        }
        std::cout << std::endl;
        
        // 打印数据
        for (int row = 0; row < row_count; row++) {
            for (int col = 0; col < col_count; col++) {
                const char* value = epiphany_get_string_value(query_result, row, col);
                std::cout << (value ? value : "NULL") << "\t";
            }
            std::cout << std::endl;
        }
        
        epiphany_free_result(query_result);
    }
    
    // 清理资源
    epiphany_destroy_session(session);
    epiphany_destroy_instance(instance);
    
    std::cout << "EpiphanyDB demo completed!" << std::endl;
    return 0;
}
```

### 2. 编译程序

```bash
# 在项目根目录下
g++ -I./include -L./build/src -o hello_epiphany hello_epiphany.cpp -lepiphany
```

### 3. 运行程序

```bash
./hello_epiphany
```

预期输出：
```
EpiphanyDB instance created successfully!
Session created successfully!
Table 'users' created successfully!
Data inserted successfully!
Query results:
Found 2 rows, 3 columns
id      name    email
1       Alice   alice@example.com
2       Bob     bob@example.com
EpiphanyDB demo completed!
```

## 基本概念

### 1. 实例 (Instance)
EpiphanyDB 实例是数据库系统的顶层对象，管理所有资源和配置。

```cpp
EPIPHANY_INSTANCE* instance = nullptr;
epiphany_create_instance(&instance);
```

### 2. 会话 (Session)
会话代表一个数据库连接，用于执行查询和事务。

```cpp
EPIPHANY_SESSION* session = nullptr;
epiphany_create_session(instance, &session);
```

### 3. 存储模式
EpiphanyDB 支持四种存储模式：

- **ROW**：行存储，适合 OLTP 场景
- **COLUMN**：列存储，适合 OLAP 场景
- **VECTOR**：向量存储，适合 AI/ML 场景
- **TIMESERIES**：时序存储，适合 IoT 场景

```sql
-- 创建不同存储模式的表
CREATE TABLE transactions (...) STORAGE_MODE = ROW;
CREATE TABLE analytics (...) STORAGE_MODE = COLUMN;
CREATE TABLE embeddings (...) STORAGE_MODE = VECTOR;
CREATE TABLE metrics (...) STORAGE_MODE = TIMESERIES;
```

## 常用操作示例

### 1. 表操作

```cpp
// 创建表
const char* create_sql = 
    "CREATE TABLE products ("
    "id INTEGER PRIMARY KEY, "
    "name VARCHAR(200), "
    "price DECIMAL(10,2), "
    "category VARCHAR(100)"
    ") STORAGE_MODE = ROW";

epiphany_execute_sql(session, create_sql, &result);

// 删除表
epiphany_execute_sql(session, "DROP TABLE products", &result);
```

### 2. 数据操作

```cpp
// 插入数据
const char* insert_sql = 
    "INSERT INTO products (name, price, category) VALUES "
    "('Laptop', 999.99, 'Electronics'), "
    "('Book', 29.99, 'Education')";

epiphany_execute_sql(session, insert_sql, &result);

// 更新数据
const char* update_sql = 
    "UPDATE products SET price = 899.99 WHERE name = 'Laptop'";

epiphany_execute_sql(session, update_sql, &result);

// 删除数据
const char* delete_sql = 
    "DELETE FROM products WHERE category = 'Education'";

epiphany_execute_sql(session, delete_sql, &result);
```

### 3. 查询操作

```cpp
// 简单查询
const char* select_sql = "SELECT * FROM products WHERE price > 500";
epiphany_execute_sql(session, select_sql, &result);

// 聚合查询
const char* agg_sql = 
    "SELECT category, COUNT(*), AVG(price) "
    "FROM products GROUP BY category";

epiphany_execute_sql(session, agg_sql, &result);
```

### 4. 索引操作

```cpp
// 创建索引
const char* index_sql = 
    "CREATE INDEX idx_product_category ON products(category)";

epiphany_execute_sql(session, index_sql, &result);

// 创建向量索引
const char* vector_index_sql = 
    "CREATE INDEX idx_embedding ON embeddings(vector) USING VECTOR_HNSW";

epiphany_execute_sql(session, vector_index_sql, &result);
```

## 性能优化建议

### 1. 选择合适的存储模式
- **OLTP 应用**：使用 ROW 存储
- **分析查询**：使用 COLUMN 存储
- **AI/ML 应用**：使用 VECTOR 存储
- **时序数据**：使用 TIMESERIES 存储

### 2. 创建合适的索引
```cpp
// 为经常查询的列创建索引
epiphany_execute_sql(session, 
    "CREATE INDEX idx_user_email ON users(email)", &result);

// 为范围查询创建索引
epiphany_execute_sql(session, 
    "CREATE INDEX idx_order_date ON orders(order_date)", &result);
```

### 3. 使用批量操作
```cpp
// 批量插入
epiphany_begin_transaction(session);

for (int i = 0; i < batch_size; i++) {
    epiphany_execute_statement(insert_stmt, NULL);
}

epiphany_commit_transaction(session);
```

### 4. 配置缓冲池
```cpp
// 设置缓冲池大小（以MB为单位）
epiphany_set_config(instance, "buffer_pool_size", "1024");

// 启用查询缓存
epiphany_set_config(instance, "enable_query_cache", "true");
```

## 下一步

现在您已经了解了 EpiphanyDB 的基础知识，可以：

1. **阅读详细文档**：查看 [用户指南](USER_GUIDE.md) 了解更多功能
2. **学习架构设计**：阅读 [架构文档](ARCHITECTURE.md) 了解系统设计
3. **查看 API 参考**：参考 [API 文档](API_REFERENCE.md) 了解完整 API
4. **运行示例程序**：查看 `examples/` 目录中的示例代码
5. **性能测试**：运行 `benchmarks/` 目录中的基准测试

## 获取帮助

如果您遇到问题或需要帮助：

1. **查看文档**：首先查看相关文档
2. **运行示例**：参考示例代码
3. **检查日志**：查看错误日志获取详细信息
4. **社区支持**：访问项目主页获取社区支持

---

*祝您使用 EpiphanyDB 愉快！*