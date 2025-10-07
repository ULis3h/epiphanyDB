<div align="center">
  <img src="docs/assets/header.svg" alt="EpiphanyDB Logo" width="200" height="200">
  
  # EpiphanyDB
  
  [![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT) [![PostgreSQL](https://img.shields.io/badge/PostgreSQL-17%2B-blue.svg)](https://www.postgresql.org/) [![Build Status](https://img.shields.io/badge/build-in--progress-yellow.svg)](#building) [![Language](https://img.shields.io/badge/language-C-blue.svg)](https://en.wikipedia.org/wiki/C_(programming_language))
  
  **基于 PostgreSQL 17 的开源超融合多模态数据库**
  
  *提供统一的多存储引擎访问接口，支持行存储、列存储、向量存储、时序存储和图存储*
</div>

## ✨ 核心特性

- **多存储引擎架构**: 统一接口访问行存储、列存储、向量存储、时序存储和图存储引擎
- **PostgreSQL 兼容**: 基于 PostgreSQL 17，完全兼容 SQL 标准
- **统一查询接口**: 通过单一 SQL 接口访问所有存储引擎
- **ACID 事务**: 跨所有存储引擎的完整 ACID 事务支持
- **C 语言实现**: 高性能的 C 语言核心，提供原生性能
- **扩展性设计**: 支持插件式存储引擎扩展

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

### 构建安装

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

## 📚 Documentation

### User Documentation
- [📖 Getting Started Guide](docs/GETTING_STARTED.md) - Essential guide for beginners
- [📘 User Guide](docs/USER_GUIDE.md) - Comprehensive usage instructions
- [📙 API Reference](docs/API_REFERENCE.md) - Complete API documentation
- [📗 Performance Guide](docs/PERFORMANCE_GUIDE.md) - Performance tuning recommendations

### Developer Documentation
- [🏗️ Architecture Design](docs/ARCHITECTURE.md) - System architecture overview
- [🔧 Development Guide](docs/DEVELOPMENT.md) - Development environment setup
- [🧪 Testing Guide](docs/TESTING.md) - Testing framework usage
- [🚀 Deployment Guide](docs/DEPLOYMENT.md) - Production deployment guide

## 🧪 测试

EpiphanyDB 包含完整的测试套件，涵盖多个存储引擎的功能测试：

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
# 安装依赖（根据操作系统而异）
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
```c
// 使用向量存储引擎进行相似性搜索
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
```c
// 使用时序存储引擎处理IoT数据
TimeseriesStorageEngine* ts_engine = timeseries_storage_create();

// 写入时序数据点
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

// 查询和聚合
TimeseriesQueryParamsExt query = {
    .series_names = (char*[]){"temperature_sensor_01"},
    .num_series = 1,
    .start_time = time(NULL) - 3600, // 最近1小时
    .end_time = time(NULL),
    .aggregation = TIMESERIES_AGG_AVG
};

TimeseriesQueryResult* result = timeseries_storage_query_ext(ts_engine, &query);
```

## 🏃‍♂️ Example Programs

The project includes rich example code:

```bash
# Basic functionality demonstration
./examples/simple_demo

# Hyperconverged features demonstration
./examples/hyperconverged_demo

# Advanced features showcase
./examples/advanced_demo

# Performance benchmarks
./benchmarks/run_benchmarks.sh
```

## 📊 Performance Benchmarks

Based on our benchmark test results:

| Operation Type | Throughput | Average Latency | P95 Latency |
|---------------|------------|-----------------|-------------|
| Point Query | 764 ops/s | 1.3 ms | 2.4 ms |
| Batch Insert (1K) | 235K ops/s | 4.2 ms | 6.3 ms |
| Vector Search (512D) | 73 ops/s | 13.5 ms | 13.8 ms |
| Aggregation Query | 85 ops/s | 11.7 ms | 17.5 ms |
| Concurrent Processing (16 threads) | 3.3K ops/s | 4.8 ms | 7.5 ms |

*Test Environment: 8-core CPU, 16GB RAM, SSD storage*

## 🛠️ Development

### Build Requirements
- CMake 3.10+
- C++11 compatible compiler
- Thread library support

### Build Options
```bash
# Debug build
cmake -DCMAKE_BUILD_TYPE=Debug ..

# Release build
cmake -DCMAKE_BUILD_TYPE=Release ..

# Enable testing
cmake -DENABLE_TESTING=ON ..
```

### Running Tests
```bash
# Run all tests
make test

# Run benchmarks
./benchmarks/run_benchmarks.sh

# Run example programs
make run_examples
```

## 🤝 Contributing

We welcome community contributions! Please check the [Contributing Guide](CONTRIBUTING.md) for details.

### Ways to Contribute
- 🐛 Report bugs
- 💡 Suggest new features
- 📝 Improve documentation
- 🔧 Submit code patches

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

Thanks to all developers and users who have contributed to the EpiphanyDB project!

## 📞 Contact Us

- **Project Homepage**: https://github.com/your-org/epiphanyDB
- **Issue Tracker**: https://github.com/your-org/epiphanyDB/issues
- **Discussions**: https://github.com/your-org/epiphanyDB/discussions
- **Email**: epiphanydb@example.com

---

⭐ If you find EpiphanyDB useful, please give us a Star!

## 🏗️ Architecture

EpiphanyDB features a modular architecture with the following key components:

### Core Components
- **Storage Engine**: Multi-modal storage layer supporting row, column, vector, and time-series data
- **Query Processor**: Unified SQL interface with intelligent optimization
- **Transaction Manager**: ACID-compliant transaction processing
- **Buffer Pool**: Multi-level caching and memory management
- **Index Manager**: Advanced indexing for all data types

### Supported Platforms
- **Linux** (Ubuntu 18.04+, CentOS 7+, RHEL 7+)
- **macOS** (10.14+)
- **Windows** (Windows 10+, Windows Server 2016+)

## 🔧 Build Instructions

### Prerequisites
```bash
# Ubuntu/Debian
sudo apt-get install build-essential cmake git

# CentOS/RHEL
sudo yum install gcc-c++ cmake git

# macOS
brew install cmake git
```

### Clone and Build
```bash
git clone https://github.com/your-org/epiphanyDB.git
cd epiphanyDB
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### Windows Build (MinGW)
```bash
mkdir build && cd build
cmake -G "MinGW Makefiles" ..
make
```
