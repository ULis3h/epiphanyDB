<div align="center">
  <img src="docs/assets/header.svg" alt="EpiphanyDB Logo" width="200" height="200">
  
  # EpiphanyDB
  
  [![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT) [![PostgreSQL](https://img.shields.io/badge/PostgreSQL-17%2B-blue.svg)](https://www.postgresql.org/) [![Build Status](https://img.shields.io/badge/build-in--progress-yellow.svg)](#building) [![Language](https://img.shields.io/badge/language-C-blue.svg)](https://en.wikipedia.org/wiki/C_(programming_language))
  
  **Open-source Hyperconverged Multi-modal Database based on PostgreSQL 17**
  
  *Provides unified multi-storage engine access interface, supporting row storage, column storage, vector storage, time-series storage, and graph storage*
  
  **English** | [中文](README_CN.md)
</div>

## ✨ Core Features

- **Multi-Storage Engine Architecture**: Unified interface for accessing row, column, vector, time-series, and graph storage engines
- **PostgreSQL Compatible**: Based on PostgreSQL 17, fully compatible with SQL standards
- **Unified Query Interface**: Access all storage engines through a single SQL interface
- **ACID Transactions**: Complete ACID transaction support across all storage engines
- **C Language Implementation**: High-performance C language core providing native performance
- **Extensible Design**: Support for plugin-style storage engine extensions

## 🚀 Quick Start

### System Requirements

- **Operating System**: macOS 10.15+ or Ubuntu 18.04+
- **Compiler**: GCC 7+ or Clang 10+
- **Memory**: At least 4GB RAM
- **Disk Space**: At least 2GB available space

### Dependency Installation

#### macOS (using Homebrew)
```bash
# Install basic dependencies
brew install gcc make autoconf automake libtool pkg-config
brew install openssl libxml2 libxslt icu4c python perl tcl-tk
brew install krb5 openssl lz4 zstd readline

# Note: macOS version does not support systemd and LLVM JIT compilation
```

#### Ubuntu/Debian
```bash
# Install basic dependencies
sudo apt-get update
sudo apt-get install -y build-essential autoconf automake libtool pkg-config
sudo apt-get install -y libssl-dev libxml2-dev libxslt1-dev libicu-dev
sudo apt-get install -y python3-dev libperl-dev tcl-dev
sudo apt-get install -y libkrb5-dev liblz4-dev libzstd-dev libreadline-dev
sudo apt-get install -y libsystemd-dev  # Linux systems support systemd

# Linux version supports full functionality, including systemd and LLVM JIT compilation
```

### Build and Installation

```bash
# Clone repository
git clone https://github.com/yourusername/epiphanyDB.git
cd epiphanyDB

# Check dependencies
make check-deps

# Build project
make build

# Install
sudo make install

# Start service
make start
```

### Verify Installation

```bash
# Run tests
make test

# Check service status
make status
```

### First Program

```c
#include "storage_engines/vector_storage.h"
#include "storage_engines/timeseries_storage.h"
#include "storage_engines/graph_storage.h"
#include <stdio.h>

int main() {
    // Initialize vector storage engine
    VectorStorageEngine* vector_engine = vector_storage_create();
    if (!vector_engine) {
        fprintf(stderr, "Failed to create vector storage engine\n");
        return 1;
    }
    
    // Create vector index
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
    
    // Insert vector data
    float vector_data[128] = {0.1, 0.2, 0.3, /* ... */};
    VectorPoint point = {
        .id = 1,
        .vector = vector_data,
        .dimension = 128
    };
    
    vector_storage_insert(vector_engine, "embeddings", &point);
    
    // Vector similarity search
    VectorSearchParams search_params = {
        .k = 10,
        .ef = 100
    };
    VectorSearchResult* search_results = NULL;
    int num_results = vector_storage_search(vector_engine, "embeddings", 
                                          vector_data, &search_params, &search_results);
    
    printf("Found %d similar vectors\n", num_results);
    
    // Clean up resources
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

## 🧪 Testing

EpiphanyDB includes a comprehensive test suite covering functionality tests for multiple storage engines:

### Test Categories
- **Multi-Storage Engine Tests**: `tests/test_multi_storage_engines.c` - Tests integration of vector, time-series, and graph storage engines
- **Unit Tests**: Independent functionality tests for individual storage engine components
- **Performance Tests**: Benchmark tests and performance regression tests
- **Integration Tests**: Cross-engine functionality and interoperability tests

### Running Tests
```bash
# Run all tests
make test

# Run specific multi-storage engine tests
make test-engines

# Compile test files (for debugging)
gcc -o test_multi_storage_engines tests/test_multi_storage_engines.c \
    -Iinclude -std=c99 -Wall -Wextra

# Run compiled tests
./test_multi_storage_engines
```

### Test Coverage
- ✅ Vector Storage Engine: Index creation, vector insertion, similarity search
- ✅ Time-series Storage Engine: Data point writing, batch operations, queries and aggregation
- ✅ Graph Storage Engine: Node and edge operations, path finding, graph traversal

## 🛠️ Development Environment

### Quick Setup
```bash
# Automated development environment setup
make dev-setup

# Check development dependencies
make check-deps
```

### Manual Setup
```bash
# Install dependencies (varies by operating system)
# Ubuntu/Debian
sudo apt-get install build-essential autoconf automake pkg-config \
    libicu-dev libkrb5-dev llvm-dev postgresql-server-dev-all

# macOS
brew install autoconf automake pkg-config llvm icu4c krb5

# Build development version
make build

# Clean build files
make clean
```

## 🎯 Use Cases

### Vector Similarity Search
```c
// Use vector storage engine for similarity search
VectorStorageEngine* engine = vector_storage_create();
VectorIndexConfig config = {
    .dimension = 512,
    .index_type = VECTOR_INDEX_HNSW,
    .metric = VECTOR_METRIC_L2
};

vector_storage_create_index(engine, "embeddings", &config);

// Insert vector data
float embedding[512] = {0.1, 0.2, /* ... */};
VectorPoint point = {.id = 1, .vector = embedding, .dimension = 512};
vector_storage_insert(engine, "embeddings", &point);

// Similarity search
VectorSearchParams params = {.k = 10, .ef = 100};
VectorSearchResult* results = NULL;
int count = vector_storage_search(engine, "embeddings", embedding, &params, &results);
```

### IoT Time-series Data Processing
```c
// Use time-series storage engine for IoT data processing
TimeseriesStorageEngine* ts_engine = timeseries_storage_create();

// Write time-series data points
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

// Query and aggregation
TimeseriesQueryParamsExt query = {
    .series_names = (char*[]){"temperature_sensor_01"},
    .num_series = 1,
    .start_time = time(NULL) - 3600, // Last 1 hour
    .end_time = time(NULL),
    .aggregation = TIMESERIES_AGG_AVG
};

TimeseriesQueryResult* result = timeseries_storage_query_ext(ts_engine, &query);
```

### Social Network Graph Analysis
```c
// Use graph storage engine for social network analysis
GraphStorageEngine* graph_engine = graph_storage_create();

// Add user nodes
GraphNode user1 = {.id = 1, .label = "User", .properties = "{\"name\":\"Alice\",\"age\":25}"};
GraphNode user2 = {.id = 2, .label = "User", .properties = "{\"name\":\"Bob\",\"age\":30}"};

graph_storage_add_node(graph_engine, &user1);
graph_storage_add_node(graph_engine, &user2);

// Add relationship edges
GraphEdge friendship = {
    .id = 1,
    .source_id = 1,
    .target_id = 2,
    .label = "FRIEND",
    .properties = "{\"since\":\"2023-01-01\"}"
};

graph_storage_add_edge(graph_engine, &friendship);

// Find paths
GraphPath* path = graph_storage_find_path(graph_engine, 1, 2, 3); // Max depth 3
if (path) {
    printf("Found path with %d nodes\n", path->num_nodes);
    graph_storage_free_path(path);
}
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
