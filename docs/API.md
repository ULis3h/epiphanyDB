# EpiphanyDB API 文档

EpiphanyDB 提供了丰富的 API 接口，支持多种编程语言和访问方式。本文档详细介绍了所有可用的 API 接口、参数说明和使用示例。

## 目录

- [概述](#概述)
- [C++ API](#c-api)
- [REST API](#rest-api)
- [SQL 接口](#sql-接口)
- [向量搜索 API](#向量搜索-api)
- [时序数据 API](#时序数据-api)
- [图数据 API](#图数据-api)
- [错误处理](#错误处理)
- [性能优化](#性能优化)

## 概述

EpiphanyDB 支持多种 API 访问方式：

- **C++ Native API**: 高性能的原生 C++ 接口
- **REST API**: 基于 HTTP 的 RESTful 接口
- **SQL Interface**: 标准 SQL 查询接口
- **Specialized APIs**: 针对向量、时序、图数据的专用接口

### 连接配置

```cpp
// C++ 连接配置
epiphany::ConnectionConfig config;
config.host = "localhost";
config.port = 5432;
config.database = "epiphany";
config.username = "user";
config.password = "password";
config.ssl_mode = epiphany::SSLMode::Prefer;
```

```bash
# REST API 基础 URL
BASE_URL="http://localhost:8080/api/v1"
```

## C++ API

### 核心类和接口

#### Database 类

```cpp
namespace epiphany {

class Database {
public:
    // 构造函数
    explicit Database(const ConnectionConfig& config);
    
    // 连接管理
    bool connect();
    void disconnect();
    bool is_connected() const;
    
    // 会话管理
    std::unique_ptr<Session> create_session();
    
    // 数据库操作
    bool create_database(const std::string& name);
    bool drop_database(const std::string& name);
    std::vector<std::string> list_databases();
    
    // 事务管理
    std::unique_ptr<Transaction> begin_transaction(
        TransactionIsolation isolation = TransactionIsolation::ReadCommitted
    );
};

} // namespace epiphany
```

#### Session 类

```cpp
namespace epiphany {

class Session {
public:
    // 表管理
    bool create_table(const TableSchema& schema);
    bool drop_table(const std::string& table_name);
    bool alter_table(const std::string& table_name, const AlterTableSpec& spec);
    
    // 数据操作
    bool insert(const std::string& table_name, const Record& record);
    bool insert_batch(const std::string& table_name, const std::vector<Record>& records);
    bool update(const std::string& table_name, const Record& record, const Condition& where);
    bool delete_records(const std::string& table_name, const Condition& where);
    
    // 查询操作
    std::unique_ptr<ResultSet> select(const Query& query);
    std::unique_ptr<ResultSet> execute_sql(const std::string& sql);
    
    // 索引管理
    bool create_index(const IndexSpec& spec);
    bool drop_index(const std::string& index_name);
    
    // 向量操作
    std::unique_ptr<VectorIndex> create_vector_index(const VectorIndexSpec& spec);
    std::vector<VectorSearchResult> vector_search(
        const std::string& table_name,
        const std::vector<float>& query_vector,
        int k = 10,
        const VectorSearchOptions& options = {}
    );
};

} // namespace epiphany
```

### 数据类型

#### Record 类

```cpp
namespace epiphany {

class Record {
public:
    // 构造函数
    Record() = default;
    explicit Record(const std::map<std::string, Value>& data);
    
    // 字段操作
    void set(const std::string& field, const Value& value);
    Value get(const std::string& field) const;
    bool has_field(const std::string& field) const;
    
    // 类型安全的访问方法
    void set_int(const std::string& field, int64_t value);
    void set_double(const std::string& field, double value);
    void set_string(const std::string& field, const std::string& value);
    void set_vector(const std::string& field, const std::vector<float>& value);
    void set_timestamp(const std::string& field, const Timestamp& value);
    
    int64_t get_int(const std::string& field) const;
    double get_double(const std::string& field) const;
    std::string get_string(const std::string& field) const;
    std::vector<float> get_vector(const std::string& field) const;
    Timestamp get_timestamp(const std::string& field) const;
    
    // 序列化
    std::string to_json() const;
    static Record from_json(const std::string& json);
};

} // namespace epiphany
```

### 使用示例

#### 基本数据操作

```cpp
#include <epiphany/database.h>
#include <epiphany/session.h>

int main() {
    // 创建数据库连接
    epiphany::ConnectionConfig config;
    config.host = "localhost";
    config.port = 5432;
    config.database = "test_db";
    
    epiphany::Database db(config);
    if (!db.connect()) {
        std::cerr << "Failed to connect to database" << std::endl;
        return 1;
    }
    
    // 创建会话
    auto session = db.create_session();
    
    // 创建表
    epiphany::TableSchema schema;
    schema.name = "users";
    schema.add_column("id", epiphany::DataType::Integer, true); // primary key
    schema.add_column("name", epiphany::DataType::String);
    schema.add_column("email", epiphany::DataType::String);
    schema.add_column("created_at", epiphany::DataType::Timestamp);
    
    if (!session->create_table(schema)) {
        std::cerr << "Failed to create table" << std::endl;
        return 1;
    }
    
    // 插入数据
    epiphany::Record record;
    record.set_int("id", 1);
    record.set_string("name", "John Doe");
    record.set_string("email", "john@example.com");
    record.set_timestamp("created_at", epiphany::Timestamp::now());
    
    if (!session->insert("users", record)) {
        std::cerr << "Failed to insert record" << std::endl;
        return 1;
    }
    
    // 查询数据
    epiphany::Query query;
    query.from("users")
         .select({"id", "name", "email"})
         .where("name = 'John Doe'");
    
    auto result = session->select(query);
    while (result->next()) {
        std::cout << "ID: " << result->get_int("id") << std::endl;
        std::cout << "Name: " << result->get_string("name") << std::endl;
        std::cout << "Email: " << result->get_string("email") << std::endl;
    }
    
    return 0;
}
```

#### 向量搜索示例

```cpp
#include <epiphany/vector_search.h>

void vector_search_example() {
    auto session = db.create_session();
    
    // 创建向量表
    epiphany::TableSchema schema;
    schema.name = "embeddings";
    schema.add_column("id", epiphany::DataType::Integer, true);
    schema.add_column("text", epiphany::DataType::String);
    schema.add_column("vector", epiphany::DataType::Vector, false, 768); // 768维向量
    
    session->create_table(schema);
    
    // 创建向量索引
    epiphany::VectorIndexSpec index_spec;
    index_spec.table_name = "embeddings";
    index_spec.column_name = "vector";
    index_spec.index_type = epiphany::VectorIndexType::HNSW;
    index_spec.distance_metric = epiphany::DistanceMetric::Cosine;
    index_spec.parameters["M"] = "16";
    index_spec.parameters["efConstruction"] = "200";
    
    auto vector_index = session->create_vector_index(index_spec);
    
    // 插入向量数据
    std::vector<float> embedding(768);
    // ... 填充 embedding 数据
    
    epiphany::Record record;
    record.set_int("id", 1);
    record.set_string("text", "Sample text");
    record.set_vector("vector", embedding);
    
    session->insert("embeddings", record);
    
    // 向量搜索
    std::vector<float> query_vector(768);
    // ... 填充查询向量
    
    epiphany::VectorSearchOptions options;
    options.ef_search = 100;
    options.include_metadata = true;
    
    auto results = session->vector_search("embeddings", query_vector, 10, options);
    
    for (const auto& result : results) {
        std::cout << "ID: " << result.id << std::endl;
        std::cout << "Distance: " << result.distance << std::endl;
        std::cout << "Text: " << result.metadata["text"] << std::endl;
    }
}
```

## REST API

### 认证

```bash
# 获取访问令牌
curl -X POST "${BASE_URL}/auth/login" \
  -H "Content-Type: application/json" \
  -d '{
    "username": "user",
    "password": "password"
  }'

# 响应
{
  "access_token": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9...",
  "token_type": "Bearer",
  "expires_in": 3600
}
```

### 数据库操作

#### 创建数据库

```bash
curl -X POST "${BASE_URL}/databases" \
  -H "Authorization: Bearer ${ACCESS_TOKEN}" \
  -H "Content-Type: application/json" \
  -d '{
    "name": "test_db",
    "options": {
      "encoding": "UTF8",
      "locale": "en_US.UTF-8"
    }
  }'
```

#### 列出数据库

```bash
curl -X GET "${BASE_URL}/databases" \
  -H "Authorization: Bearer ${ACCESS_TOKEN}"

# 响应
{
  "databases": [
    {
      "name": "test_db",
      "size": "15MB",
      "created_at": "2024-01-15T10:30:00Z"
    }
  ]
}
```

### 表操作

#### 创建表

```bash
curl -X POST "${BASE_URL}/databases/test_db/tables" \
  -H "Authorization: Bearer ${ACCESS_TOKEN}" \
  -H "Content-Type: application/json" \
  -d '{
    "name": "users",
    "columns": [
      {
        "name": "id",
        "type": "integer",
        "primary_key": true,
        "auto_increment": true
      },
      {
        "name": "name",
        "type": "string",
        "max_length": 255,
        "nullable": false
      },
      {
        "name": "email",
        "type": "string",
        "max_length": 255,
        "unique": true
      },
      {
        "name": "created_at",
        "type": "timestamp",
        "default": "CURRENT_TIMESTAMP"
      }
    ]
  }'
```

#### 查询表结构

```bash
curl -X GET "${BASE_URL}/databases/test_db/tables/users/schema" \
  -H "Authorization: Bearer ${ACCESS_TOKEN}"

# 响应
{
  "table_name": "users",
  "columns": [
    {
      "name": "id",
      "type": "integer",
      "nullable": false,
      "primary_key": true,
      "auto_increment": true
    },
    {
      "name": "name",
      "type": "string",
      "max_length": 255,
      "nullable": false
    }
  ],
  "indexes": [
    {
      "name": "users_pkey",
      "type": "btree",
      "columns": ["id"],
      "unique": true
    }
  ]
}
```

### 数据操作

#### 插入数据

```bash
curl -X POST "${BASE_URL}/databases/test_db/tables/users/records" \
  -H "Authorization: Bearer ${ACCESS_TOKEN}" \
  -H "Content-Type: application/json" \
  -d '{
    "records": [
      {
        "name": "John Doe",
        "email": "john@example.com"
      },
      {
        "name": "Jane Smith",
        "email": "jane@example.com"
      }
    ]
  }'

# 响应
{
  "inserted_count": 2,
  "inserted_ids": [1, 2]
}
```

#### 查询数据

```bash
curl -X GET "${BASE_URL}/databases/test_db/tables/users/records?limit=10&offset=0" \
  -H "Authorization: Bearer ${ACCESS_TOKEN}"

# 带条件查询
curl -X GET "${BASE_URL}/databases/test_db/tables/users/records?where=name='John Doe'" \
  -H "Authorization: Bearer ${ACCESS_TOKEN}"

# 响应
{
  "records": [
    {
      "id": 1,
      "name": "John Doe",
      "email": "john@example.com",
      "created_at": "2024-01-15T10:30:00Z"
    }
  ],
  "total_count": 1,
  "has_more": false
}
```

#### 更新数据

```bash
curl -X PUT "${BASE_URL}/databases/test_db/tables/users/records/1" \
  -H "Authorization: Bearer ${ACCESS_TOKEN}" \
  -H "Content-Type: application/json" \
  -d '{
    "name": "John Smith",
    "email": "johnsmith@example.com"
  }'
```

#### 删除数据

```bash
curl -X DELETE "${BASE_URL}/databases/test_db/tables/users/records/1" \
  -H "Authorization: Bearer ${ACCESS_TOKEN}"

# 批量删除
curl -X DELETE "${BASE_URL}/databases/test_db/tables/users/records?where=created_at<'2024-01-01'" \
  -H "Authorization: Bearer ${ACCESS_TOKEN}"
```

### SQL 查询

```bash
curl -X POST "${BASE_URL}/databases/test_db/query" \
  -H "Authorization: Bearer ${ACCESS_TOKEN}" \
  -H "Content-Type: application/json" \
  -d '{
    "sql": "SELECT u.name, u.email, COUNT(o.id) as order_count FROM users u LEFT JOIN orders o ON u.id = o.user_id GROUP BY u.id, u.name, u.email",
    "parameters": {}
  }'

# 响应
{
  "columns": [
    {"name": "name", "type": "string"},
    {"name": "email", "type": "string"},
    {"name": "order_count", "type": "integer"}
  ],
  "rows": [
    ["John Doe", "john@example.com", 5],
    ["Jane Smith", "jane@example.com", 3]
  ],
  "execution_time_ms": 15.2
}
```

## 向量搜索 API

### 创建向量索引

```bash
curl -X POST "${BASE_URL}/databases/test_db/tables/embeddings/vector_indexes" \
  -H "Authorization: Bearer ${ACCESS_TOKEN}" \
  -H "Content-Type: application/json" \
  -d '{
    "name": "embedding_index",
    "column": "vector",
    "index_type": "hnsw",
    "distance_metric": "cosine",
    "parameters": {
      "M": 16,
      "efConstruction": 200,
      "efSearch": 100
    }
  }'
```

### 向量搜索

```bash
curl -X POST "${BASE_URL}/databases/test_db/tables/embeddings/vector_search" \
  -H "Authorization: Bearer ${ACCESS_TOKEN}" \
  -H "Content-Type: application/json" \
  -d '{
    "query_vector": [0.1, 0.2, 0.3, ...],
    "k": 10,
    "distance_metric": "cosine",
    "include_metadata": true,
    "filter": {
      "category": "technology"
    }
  }'

# 响应
{
  "results": [
    {
      "id": 123,
      "distance": 0.15,
      "metadata": {
        "title": "Machine Learning Basics",
        "category": "technology"
      }
    }
  ],
  "execution_time_ms": 5.2
}
```

## 时序数据 API

### 创建时序表

```bash
curl -X POST "${BASE_URL}/databases/test_db/timeseries_tables" \
  -H "Authorization: Bearer ${ACCESS_TOKEN}" \
  -H "Content-Type: application/json" \
  -d '{
    "name": "sensor_data",
    "time_column": "timestamp",
    "partition_interval": "1 day",
    "retention_policy": "30 days",
    "columns": [
      {"name": "sensor_id", "type": "string"},
      {"name": "temperature", "type": "double"},
      {"name": "humidity", "type": "double"},
      {"name": "timestamp", "type": "timestamp"}
    ]
  }'
```

### 插入时序数据

```bash
curl -X POST "${BASE_URL}/databases/test_db/timeseries_tables/sensor_data/data" \
  -H "Authorization: Bearer ${ACCESS_TOKEN}" \
  -H "Content-Type: application/json" \
  -d '{
    "data_points": [
      {
        "sensor_id": "sensor_001",
        "temperature": 23.5,
        "humidity": 65.2,
        "timestamp": "2024-01-15T10:30:00Z"
      },
      {
        "sensor_id": "sensor_001",
        "temperature": 23.7,
        "humidity": 64.8,
        "timestamp": "2024-01-15T10:31:00Z"
      }
    ]
  }'
```

### 时序查询

```bash
# 时间范围查询
curl -X GET "${BASE_URL}/databases/test_db/timeseries_tables/sensor_data/data?start_time=2024-01-15T00:00:00Z&end_time=2024-01-15T23:59:59Z&sensor_id=sensor_001" \
  -H "Authorization: Bearer ${ACCESS_TOKEN}"

# 聚合查询
curl -X POST "${BASE_URL}/databases/test_db/timeseries_tables/sensor_data/aggregate" \
  -H "Authorization: Bearer ${ACCESS_TOKEN}" \
  -H "Content-Type: application/json" \
  -d '{
    "start_time": "2024-01-15T00:00:00Z",
    "end_time": "2024-01-15T23:59:59Z",
    "interval": "1 hour",
    "aggregations": [
      {"column": "temperature", "function": "avg"},
      {"column": "humidity", "function": "max"}
    ],
    "group_by": ["sensor_id"]
  }'
```

## 图数据 API

### 创建图

```bash
curl -X POST "${BASE_URL}/databases/test_db/graphs" \
  -H "Authorization: Bearer ${ACCESS_TOKEN}" \
  -H "Content-Type: application/json" \
  -d '{
    "name": "social_network",
    "node_types": [
      {
        "name": "User",
        "properties": [
          {"name": "name", "type": "string"},
          {"name": "age", "type": "integer"}
        ]
      }
    ],
    "edge_types": [
      {
        "name": "FOLLOWS",
        "from_node_type": "User",
        "to_node_type": "User",
        "properties": [
          {"name": "since", "type": "timestamp"}
        ]
      }
    ]
  }'
```

### 添加节点和边

```bash
# 添加节点
curl -X POST "${BASE_URL}/databases/test_db/graphs/social_network/nodes" \
  -H "Authorization: Bearer ${ACCESS_TOKEN}" \
  -H "Content-Type: application/json" \
  -d '{
    "nodes": [
      {
        "type": "User",
        "properties": {
          "name": "Alice",
          "age": 30
        }
      },
      {
        "type": "User",
        "properties": {
          "name": "Bob",
          "age": 25
        }
      }
    ]
  }'

# 添加边
curl -X POST "${BASE_URL}/databases/test_db/graphs/social_network/edges" \
  -H "Authorization: Bearer ${ACCESS_TOKEN}" \
  -H "Content-Type: application/json" \
  -d '{
    "edges": [
      {
        "type": "FOLLOWS",
        "from_node_id": "user_1",
        "to_node_id": "user_2",
        "properties": {
          "since": "2024-01-15T10:30:00Z"
        }
      }
    ]
  }'
```

### 图查询

```bash
# 路径查询
curl -X POST "${BASE_URL}/databases/test_db/graphs/social_network/query" \
  -H "Authorization: Bearer ${ACCESS_TOKEN}" \
  -H "Content-Type: application/json" \
  -d '{
    "query_type": "path",
    "start_node_id": "user_1",
    "end_node_id": "user_2",
    "max_depth": 3,
    "edge_types": ["FOLLOWS"]
  }'

# 邻居查询
curl -X POST "${BASE_URL}/databases/test_db/graphs/social_network/query" \
  -H "Authorization: Bearer ${ACCESS_TOKEN}" \
  -H "Content-Type: application/json" \
  -d '{
    "query_type": "neighbors",
    "node_id": "user_1",
    "depth": 2,
    "direction": "outgoing"
  }'
```

## 错误处理

### 错误响应格式

```json
{
  "error": {
    "code": "INVALID_REQUEST",
    "message": "Invalid table name format",
    "details": {
      "field": "table_name",
      "value": "invalid-name",
      "expected_format": "alphanumeric_underscore"
    },
    "request_id": "req_123456789"
  }
}
```

### 常见错误代码

| 错误代码 | HTTP状态码 | 描述 |
|----------|------------|------|
| `INVALID_REQUEST` | 400 | 请求参数无效 |
| `UNAUTHORIZED` | 401 | 认证失败 |
| `FORBIDDEN` | 403 | 权限不足 |
| `NOT_FOUND` | 404 | 资源不存在 |
| `CONFLICT` | 409 | 资源冲突 |
| `RATE_LIMIT_EXCEEDED` | 429 | 请求频率超限 |
| `INTERNAL_ERROR` | 500 | 服务器内部错误 |
| `SERVICE_UNAVAILABLE` | 503 | 服务不可用 |

### C++ 异常处理

```cpp
try {
    auto result = session->select(query);
    // 处理结果
} catch (const epiphany::ConnectionException& e) {
    std::cerr << "Connection error: " << e.what() << std::endl;
} catch (const epiphany::QueryException& e) {
    std::cerr << "Query error: " << e.what() << std::endl;
    std::cerr << "SQL State: " << e.sql_state() << std::endl;
} catch (const epiphany::DataException& e) {
    std::cerr << "Data error: " << e.what() << std::endl;
} catch (const std::exception& e) {
    std::cerr << "Unexpected error: " << e.what() << std::endl;
}
```

## 性能优化

### 批量操作

```cpp
// C++ 批量插入
std::vector<epiphany::Record> records;
for (int i = 0; i < 1000; ++i) {
    epiphany::Record record;
    record.set_int("id", i);
    record.set_string("data", "batch_data_" + std::to_string(i));
    records.push_back(record);
}

session->insert_batch("test_table", records);
```

```bash
# REST API 批量插入
curl -X POST "${BASE_URL}/databases/test_db/tables/test_table/records/batch" \
  -H "Authorization: Bearer ${ACCESS_TOKEN}" \
  -H "Content-Type: application/json" \
  -d '{
    "records": [
      {"id": 1, "data": "batch_data_1"},
      {"id": 2, "data": "batch_data_2"}
    ],
    "batch_size": 1000
  }'
```

### 连接池配置

```cpp
epiphany::ConnectionPoolConfig pool_config;
pool_config.min_connections = 5;
pool_config.max_connections = 20;
pool_config.connection_timeout = std::chrono::seconds(30);
pool_config.idle_timeout = std::chrono::minutes(10);

auto pool = epiphany::ConnectionPool::create(config, pool_config);
auto connection = pool->get_connection();
```

### 查询优化

```sql
-- 使用索引提示
SELECT /*+ INDEX(users, idx_email) */ * FROM users WHERE email = 'john@example.com';

-- 使用查询计划缓存
PREPARE user_query AS SELECT * FROM users WHERE id = $1;
EXECUTE user_query(123);
```

## SDK 和客户端库

### Python SDK

```python
import epiphanydb

# 连接数据库
client = epiphanydb.Client(
    host='localhost',
    port=5432,
    database='test_db',
    username='user',
    password='password'
)

# 查询数据
result = client.query("SELECT * FROM users WHERE age > ?", [25])
for row in result:
    print(f"Name: {row['name']}, Age: {row['age']}")

# 向量搜索
results = client.vector_search(
    table='embeddings',
    query_vector=[0.1, 0.2, 0.3],
    k=10
)
```

### JavaScript SDK

```javascript
const EpiphanyDB = require('epiphanydb');

const client = new EpiphanyDB.Client({
  host: 'localhost',
  port: 5432,
  database: 'test_db',
  username: 'user',
  password: 'password'
});

// 异步查询
async function queryUsers() {
  const result = await client.query('SELECT * FROM users WHERE age > $1', [25]);
  return result.rows;
}

// 向量搜索
async function vectorSearch() {
  const results = await client.vectorSearch({
    table: 'embeddings',
    queryVector: [0.1, 0.2, 0.3],
    k: 10
  });
  return results;
}
```

### Go SDK

```go
package main

import (
    "github.com/epiphanydb/go-client"
)

func main() {
    client, err := epiphanydb.Connect(&epiphanydb.Config{
        Host:     "localhost",
        Port:     5432,
        Database: "test_db",
        Username: "user",
        Password: "password",
    })
    if err != nil {
        panic(err)
    }
    defer client.Close()

    // 查询数据
    rows, err := client.Query("SELECT * FROM users WHERE age > $1", 25)
    if err != nil {
        panic(err)
    }
    defer rows.Close()

    for rows.Next() {
        var name string
        var age int
        err := rows.Scan(&name, &age)
        if err != nil {
            panic(err)
        }
        fmt.Printf("Name: %s, Age: %d\n", name, age)
    }
}
```

## 获取帮助

- **API 参考**: [https://epiphanydb.io/api](https://epiphanydb.io/api)
- **示例代码**: [https://github.com/your-org/epiphanyDB/tree/main/examples](https://github.com/your-org/epiphanyDB/tree/main/examples)
- **社区论坛**: [https://github.com/your-org/epiphanyDB/discussions](https://github.com/your-org/epiphanyDB/discussions)
- **问题报告**: [https://github.com/your-org/epiphanyDB/issues](https://github.com/your-org/epiphanyDB/issues)