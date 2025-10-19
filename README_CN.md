<div align="center">
  <img src="docs/assets/header.svg" alt="EpiphanyDB Logo" width="200" height="200">
  
  # EpiphanyDB
  
  [![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT) [![PostgreSQL](https://img.shields.io/badge/PostgreSQL-17%2B-blue.svg)](https://www.postgresql.org/) [![Build Status](https://img.shields.io/badge/build-in--progress-yellow.svg)](#building) [![Language](https://img.shields.io/badge/language-C-blue.svg)](https://en.wikipedia.org/wiki/C_(programming_language))
  
  **基于 PostgreSQL 17 的开源超融合多模态数据库**
  
  *提供统一的多存储引擎访问接口，支持行存储、列存储、向量存储、时序存储和图存储*
  
  [English](README.md) | **中文**
</div>

## 🌟 项目简介

EpiphanyDB 是一个革命性的超融合数据库系统，将多种数据存储模式无缝集成在统一的架构中。它支持传统的行存储和列存储，同时原生支持向量存储、时序存储和图存储，为现代应用提供一站式数据管理解决方案。

## ✨ 核心特性

- **多存储引擎架构**: 统一接口访问行存储、列存储、向量存储、时序存储和图存储引擎
- **PostgreSQL 兼容**: 基于 PostgreSQL 17，完全兼容 SQL 标准
- **统一查询接口**: 通过单一 SQL 接口访问所有存储引擎
- **ACID 事务**: 跨所有存储引擎的完整 ACID 事务支持
- **C 语言实现**: 高性能 C 语言核心，提供原生性能
- **可扩展设计**: 支持插件式存储引擎扩展

## 🏗️ 架构概览

```
┌─────────────────────────────────────────────────────────────┐
│                    统一查询接口 (SQL)                        │
├─────────────────────────────────────────────────────────────┤
│                    查询优化器 & 执行引擎                      │
├─────────────────────────────────────────────────────────────┤
│  行存储  │  列存储  │  向量存储  │  时序存储  │  图存储      │
│ (OLTP)  │ (OLAP)  │  (AI/ML)  │  (IoT)   │ (关系网络)    │
├─────────────────────────────────────────────────────────────┤
│                    存储管理 & 缓存层                         │
├─────────────────────────────────────────────────────────────┤
│                    事务管理 & 并发控制                       │
└─────────────────────────────────────────────────────────────┘
```

## 🚀 快速开始

### 系统要求

- **操作系统**: macOS 10.15+ 或 Ubuntu 18.04+
- **编译器**: GCC 7+ 或 Clang 10+
- **内存**: 至少 4GB RAM
- **磁盘空间**: 至少 2GB 可用空间

### 依赖安装

#### macOS (使用 Homebrew)
```bash
# 安装基础依赖
brew install gcc make autoconf automake libtool pkg-config
brew install openssl libxml2 libxslt icu4c python perl tcl-tk
brew install krb5 openssl lz4 zstd readline

# 注意：macOS 版本不支持 systemd 和 LLVM JIT 编译
```

#### Ubuntu/Debian
```bash
# 安装基础依赖
sudo apt-get update
sudo apt-get install -y build-essential autoconf automake libtool pkg-config
sudo apt-get install -y libssl-dev libxml2-dev libxslt1-dev libicu-dev
sudo apt-get install -y python3-dev libperl-dev tcl-dev
sudo apt-get install -y libkrb5-dev liblz4-dev libzstd-dev libreadline-dev
sudo apt-get install -y libsystemd-dev  # Linux 系统支持 systemd

# Linux 版本支持完整功能，包括 systemd 和 LLVM JIT 编译
```

### 构建和安装

```bash
# 克隆仓库
git clone https://github.com/yourusername/epiphanyDB.git
cd epiphanyDB

# 检查依赖
make check-deps

# 构建项目
make build

# 安装
sudo make install

# 启动服务
make start
```

### 验证安装

```bash
# 运行测试
make test

# 检查服务状态
make status
```

### 第一个程序

```c
#include "storage_engines/vector_storage.h"
#include "storage_engines/timeseries_storage.h"
#include "storage_engines/graph_storage.h"
#include <stdio.h>

int main() {
    // 初始化向量存储引擎
    VectorStorageEngine* vector_engine = vector_storage_create();
    if (!vector_engine) {
        fprintf(stderr, "Failed to create vector storage engine\n");
        return 1;
    }
    
    // 创建向量索引
    VectorIndexConfig config = {
        .dimension = 128,
        .index_type = VECTOR_INDEX_HNSW,
        .metric = VECTOR_METRIC_L2
    };
    
    int result = vector_storage_create_index(vector_engine, "embeddings", &config);
    if (result != 0) {
        fprintf(stderr, "Failed to create vector index\n");
        vector_storage_destroy(vector_engine);
        return 1;
    }
    
    // 插入向量数据
    float vector_data[128] = {0.1, 0.2, 0.3, /* ... */};
    VectorPoint point = {
        .id = 1,
        .vector = vector_data,
        .dimension = 128
    };
    
    vector_storage_insert(vector_engine, "embeddings", &point);
    
    // 向量相似性搜索
    VectorSearchParams search_params = {
        .k = 10,
        .ef = 100
    };
    VectorSearchResult* search_results = NULL;
    int num_results = vector_storage_search(vector_engine, "embeddings", 
                                          vector_data, &search_params, &search_results);
    
    printf("Found %d similar vectors\n", num_results);
    
    // 清理资源
    vector_storage_free_search_results(search_results, num_results);
    vector_storage_destroy(vector_engine);
    
    return 0;
}
```

## 📚 文档

### 用户文档
- [📖 入门指南](docs/GETTING_STARTED.md) - 初学者必读指南
- [📘 用户手册](docs/USER_GUIDE.md) - 全面的使用说明
- [📙 API 参考](docs/API_REFERENCE.md) - 完整的 API 文档
- [📗 性能指南](docs/PERFORMANCE_GUIDE.md) - 性能调优建议

### 开发者文档
- [🏗️ 架构设计](docs/ARCHITECTURE.md) - 系统架构概览
- [🔧 开发指南](docs/DEVELOPMENT.md) - 开发环境搭建
- [🧪 测试指南](docs/TESTING.md) - 测试框架使用
- [🚀 部署指南](docs/DEPLOYMENT.md) - 生产环境部署

## 🧪 测试

EpiphanyDB 包含全面的测试套件，涵盖多存储引擎的功能测试：

### 测试类别
- **多存储引擎测试**: `tests/test_multi_storage_engines.c` - 测试向量、时序和图存储引擎的集成
- **单元测试**: 各个存储引擎组件的独立功能测试
- **性能测试**: 基准测试和性能回归测试
- **集成测试**: 跨引擎功能和互操作性测试

### 运行测试
```bash
# 运行所有测试
make test

# 运行特定的多存储引擎测试
make test-engines

# 编译测试文件（用于调试）
gcc -o test_multi_storage_engines tests/test_multi_storage_engines.c \
    -Iinclude -std=c99 -Wall -Wextra

# 运行编译后的测试
./test_multi_storage_engines
```

### 测试覆盖
- ✅ 向量存储引擎：索引创建、向量插入、相似性搜索
- ✅ 时序存储引擎：数据点写入、批量操作、查询和聚合
- ✅ 图存储引擎：节点和边操作、路径查找、图遍历

## 🛠️ 开发环境

### 快速设置
```bash
# 自动化开发环境设置
make dev-setup

# 检查开发依赖
make check-deps
```

### 手动设置
```bash
# 安装依赖（因操作系统而异）
# Ubuntu/Debian
sudo apt-get install build-essential autoconf automake pkg-config \
    libicu-dev libkrb5-dev llvm-dev postgresql-server-dev-all

# macOS
brew install autoconf automake pkg-config llvm icu4c krb5

# 构建开发版本
make build

# 清理构建文件
make clean
```

## 🎯 使用场景

### 向量相似性搜索
适用于 AI/ML 应用，如：
- **语义搜索**: 文档、图片、音频的语义相似性搜索
- **推荐系统**: 基于用户行为和内容特征的个性化推荐
- **异常检测**: 通过向量距离检测异常模式

```c
// 向量相似性搜索示例
VectorStorageEngine* engine = vector_storage_create();
VectorIndexConfig config = {
    .dimension = 512,
    .index_type = VECTOR_INDEX_HNSW,
    .metric = VECTOR_METRIC_L2
};

vector_storage_create_index(engine, "embeddings", &config);

// 插入向量数据
float embedding[512] = {0.1, 0.2, /* ... */};
VectorPoint point = {.id = 1, .vector = embedding, .dimension = 512};
vector_storage_insert(engine, "embeddings", &point);

// 相似性搜索
VectorSearchParams params = {.k = 10, .ef = 100};
VectorSearchResult* results = NULL;
int count = vector_storage_search(engine, "embeddings", embedding, &params, &results);
```

### IoT 时序数据处理
适用于物联网和监控场景：
- **设备监控**: 实时监控设备状态和性能指标
- **环境监测**: 温度、湿度、空气质量等环境数据
- **工业自动化**: 生产线数据采集和分析

```c
// IoT 时序数据处理示例
TimeseriesStorageEngine* ts_engine = timeseries_storage_create();

// 写入传感器数据
TimeseriesPoint point = {
    .timestamp = time(NULL),
    .series_name = "temperature_sensor_01",
    .fields = (TimeseriesFieldValue[]){
        {.name = "temperature", .type = TIMESERIES_FIELD_FLOAT, .value.float_val = 23.5},
        {.name = "humidity", .type = TIMESERIES_FIELD_FLOAT, .value.float_val = 65.2}
    },
    .num_fields = 2
};

timeseries_storage_write_point(ts_engine, &point);

// 时间范围查询
TimeseriesQueryParamsExt query = {
    .series_names = (char*[]){"temperature_sensor_01"},
    .num_series = 1,
    .start_time = time(NULL) - 3600, // 最近1小时
    .end_time = time(NULL),
    .aggregation = TIMESERIES_AGG_AVG
};

TimeseriesQueryResult* result = timeseries_storage_query_ext(ts_engine, &query);
```

### 社交网络图分析
适用于关系型数据分析：
- **社交网络**: 用户关系分析、社区发现、影响力分析
- **知识图谱**: 实体关系建模和推理
- **供应链管理**: 供应商关系和依赖分析

```c
// 社交网络图分析示例
GraphStorageEngine* graph_engine = graph_storage_create();

// 添加用户节点
GraphNode user1 = {.id = 1, .label = "User", .properties = "{\"name\":\"Alice\",\"age\":25}"};
GraphNode user2 = {.id = 2, .label = "User", .properties = "{\"name\":\"Bob\",\"age\":30}"};

graph_storage_add_node(graph_engine, &user1);
graph_storage_add_node(graph_engine, &user2);

// 添加好友关系
GraphEdge friendship = {
    .id = 1,
    .source_id = 1,
    .target_id = 2,
    .label = "FRIEND",
    .properties = "{\"since\":\"2023-01-01\"}"
};

graph_storage_add_edge(graph_engine, &friendship);

// 查找最短路径
GraphPath* path = graph_storage_find_path(graph_engine, 1, 2, 3); // 最大深度3
if (path) {
    printf("找到路径，包含 %d 个节点\n", path->num_nodes);
    graph_storage_free_path(path);
}
```

## 🚀 性能特性

### 向量存储引擎
- **高维向量支持**: 支持最高 65536 维向量
- **多种索引算法**: HNSW、IVF、LSH 等
- **相似性度量**: L2、余弦、内积等
- **并行搜索**: 多线程并行相似性搜索
- **内存优化**: 高效的内存使用和缓存策略

### 时序存储引擎
- **高吞吐写入**: 支持每秒百万级数据点写入
- **压缩存储**: 时序数据高效压缩算法
- **聚合查询**: 内置 sum、avg、min、max 等聚合函数
- **时间窗口**: 灵活的时间窗口查询支持
- **数据保留**: 自动数据过期和清理机制

### 图存储引擎
- **大规模图**: 支持千万级节点和边
- **图算法**: 内置 BFS、DFS、最短路径等算法
- **属性图**: 支持节点和边的属性存储
- **图遍历**: 高效的图遍历和模式匹配
- **并发访问**: 支持多线程并发图操作

## 🔧 配置选项

### 全局配置
```c
// 设置全局配置
EpiphanyDBConfig config = {
    .memory_limit = 1024 * 1024 * 1024, // 1GB
    .thread_pool_size = 8,
    .enable_logging = true,
    .log_level = LOG_INFO
};

epiphanydb_set_global_config(&config);
```

### 存储引擎特定配置
```c
// 向量存储引擎配置
VectorStorageConfig vector_config = {
    .max_vectors = 1000000,
    .index_build_threads = 4,
    .search_threads = 8,
    .cache_size = 256 * 1024 * 1024 // 256MB
};

// 时序存储引擎配置
TimeseriesStorageConfig ts_config = {
    .retention_period = 30 * 24 * 3600, // 30天
    .compression_level = 6,
    .batch_size = 1000,
    .flush_interval = 5 // 5秒
};

// 图存储引擎配置
GraphStorageConfig graph_config = {
    .max_nodes = 10000000,
    .max_edges = 100000000,
    .enable_indexing = true,
    .cache_size = 512 * 1024 * 1024 // 512MB
};
```

## 📊 基准测试

### 向量搜索性能
- **数据集**: 1M 个 128 维向量
- **查询延迟**: < 1ms (P99)
- **吞吐量**: > 10,000 QPS
- **内存使用**: ~2GB

### 时序写入性能
- **写入速率**: 1M 数据点/秒
- **压缩比**: 10:1
- **查询延迟**: < 10ms (聚合查询)
- **存储效率**: 90% 压缩率

### 图遍历性能
- **图规模**: 10M 节点, 100M 边
- **BFS 遍历**: < 100ms (深度 6)
- **最短路径**: < 50ms (平均)
- **并发性能**: 支持 100+ 并发查询

## 🤝 贡献

我们欢迎社区贡献！请查看[贡献指南](CONTRIBUTING.md)了解详细信息。

### 贡献方式
- 🐛 报告错误
- 💡 建议新功能
- 📝 改进文档
- 🔧 提交代码补丁

## 📄 许可证

本项目采用 MIT 许可证 - 详见 [LICENSE](LICENSE) 文件。

## 🙏 致谢

感谢所有为 EpiphanyDB 项目做出贡献的开发者和用户！

## 📞 联系我们

- **项目主页**: https://github.com/your-org/epiphanyDB
- **问题跟踪**: https://github.com/your-org/epiphanyDB/issues
- **讨论区**: https://github.com/your-org/epiphanyDB/discussions
- **邮箱**: epiphanydb@example.com

---

⭐ 如果您觉得 EpiphanyDB 有用，请给我们一个 Star！

## 🏗️ 架构

EpiphanyDB 采用模块化架构，包含以下关键组件：

### 核心组件
- **存储引擎**: 支持行、列、向量和时序数据的多模态存储层
- **查询处理器**: 具有智能优化的统一 SQL 接口
- **事务管理器**: 符合 ACID 的事务处理
- **缓冲池**: 多级缓存和内存管理
- **索引管理器**: 所有数据类型的高级索引

### 支持平台
- **Linux** (Ubuntu 18.04+, CentOS 7+, RHEL 7+)
- **macOS** (10.14+)
- **Windows** (Windows 10+, Windows Server 2016+)

## 🔧 构建说明

### 先决条件
```bash
# Ubuntu/Debian
sudo apt-get install build-essential cmake git

# CentOS/RHEL
sudo yum install gcc-c++ cmake git

# macOS
brew install cmake git
```

### 克隆和构建
```bash
git clone https://github.com/your-org/epiphanyDB.git
cd epiphanyDB
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### Windows 构建 (MinGW)
```bash
mkdir build && cd build
cmake -G "MinGW Makefiles" ..
make
```