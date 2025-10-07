# 测试指南

本文档详细介绍 EpiphanyDB 项目的测试策略、测试框架使用方法和测试最佳实践。

## 🎯 测试策略

### 测试金字塔

EpiphanyDB 采用经典的测试金字塔策略：

```
        /\
       /  \
      / UI \     <- 端到端测试 (少量)
     /______\
    /        \
   / 集成测试  \   <- 集成测试 (适量)
  /____________\
 /              \
/    单元测试     \  <- 单元测试 (大量)
/________________\
```

### 测试类型

#### 1. 单元测试 (Unit Tests)
- **目标**: 测试单个函数、类或模块
- **覆盖率**: 目标 90%+
- **运行频率**: 每次提交
- **位置**: `tests/unit/`

#### 2. 集成测试 (Integration Tests)
- **目标**: 测试模块间的交互
- **覆盖率**: 关键路径 100%
- **运行频率**: 每次 PR
- **位置**: `tests/integration/`

#### 3. 性能测试 (Performance Tests)
- **目标**: 验证性能指标
- **基准**: 回归测试
- **运行频率**: 每日构建
- **位置**: `benchmarks/`

#### 4. 端到端测试 (E2E Tests)
- **目标**: 完整用户场景
- **覆盖率**: 主要用例
- **运行频率**: 发布前
- **位置**: `tests/e2e/`

## 🛠️ 测试框架

### Google Test (单元测试)

#### 安装和配置

```bash
# Ubuntu/Debian
sudo apt install -y libgtest-dev

# macOS
brew install googletest

# 从源码构建
git clone https://github.com/google/googletest.git
cd googletest
mkdir build && cd build
cmake ..
make && sudo make install
```

#### 基本用法

```cpp
#include <gtest/gtest.h>
#include "epiphany/core/engine.h"

// 简单测试
TEST(EngineTest, ShouldCreateInstance) {
    Engine engine;
    EXPECT_TRUE(engine.isValid());
}

// 参数化测试
class StorageModeTest : public ::testing::TestWithParam<StorageMode> {
protected:
    void SetUp() override {
        mode = GetParam();
        engine.setStorageMode(mode);
    }
    
    StorageMode mode;
    Engine engine;
};

TEST_P(StorageModeTest, ShouldSupportAllModes) {
    EXPECT_TRUE(engine.supportsMode(mode));
}

INSTANTIATE_TEST_SUITE_P(
    AllStorageModes,
    StorageModeTest,
    ::testing::Values(
        StorageMode::ROW,
        StorageMode::COLUMN,
        StorageMode::VECTOR,
        StorageMode::TIMESERIES
    )
);

// 测试夹具
class DatabaseTest : public ::testing::Test {
protected:
    void SetUp() override {
        epiphany_create_instance(&instance);
        epiphany_create_session(instance, &session);
    }
    
    void TearDown() override {
        epiphany_destroy_session(session);
        epiphany_destroy_instance(instance);
    }
    
    EPIPHANY_INSTANCE* instance = nullptr;
    EPIPHANY_SESSION* session = nullptr;
};

TEST_F(DatabaseTest, ShouldCreateTable) {
    auto result = epiphany_execute_sql(session,
        "CREATE TABLE test (id INT, name VARCHAR(100))", nullptr);
    EXPECT_EQ(result->error_code, EPIPHANY_SUCCESS);
    epiphany_free_result(result);
}
```

### Google Benchmark (性能测试)

#### 基本用法

```cpp
#include <benchmark/benchmark.h>
#include "epiphany/epiphany.h"

// 简单基准测试
static void BM_SimpleQuery(benchmark::State& state) {
    EPIPHANY_INSTANCE* instance;
    EPIPHANY_SESSION* session;
    
    epiphany_create_instance(&instance);
    epiphany_create_session(instance, &session);
    
    for (auto _ : state) {
        auto result = epiphany_execute_sql(session, "SELECT 1", nullptr);
        benchmark::DoNotOptimize(result);
        epiphany_free_result(result);
    }
    
    epiphany_destroy_session(session);
    epiphany_destroy_instance(instance);
}
BENCHMARK(BM_SimpleQuery);

// 参数化基准测试
static void BM_InsertData(benchmark::State& state) {
    const int batch_size = state.range(0);
    
    EPIPHANY_INSTANCE* instance;
    EPIPHANY_SESSION* session;
    
    epiphany_create_instance(&instance);
    epiphany_create_session(instance, &session);
    
    // 创建表
    epiphany_execute_sql(session,
        "CREATE TABLE bench_test (id INT, value VARCHAR(100))", nullptr);
    
    for (auto _ : state) {
        state.PauseTiming();
        std::string sql = generateInsertSQL(batch_size);
        state.ResumeTiming();
        
        auto result = epiphany_execute_sql(session, sql.c_str(), nullptr);
        epiphany_free_result(result);
    }
    
    state.SetItemsProcessed(state.iterations() * batch_size);
    
    epiphany_destroy_session(session);
    epiphany_destroy_instance(instance);
}
BENCHMARK(BM_InsertData)->Range(1, 1000)->Unit(benchmark::kMicrosecond);

// 多线程基准测试
static void BM_ConcurrentQueries(benchmark::State& state) {
    if (state.thread_index == 0) {
        // 设置共享资源
    }
    
    EPIPHANY_INSTANCE* instance;
    EPIPHANY_SESSION* session;
    
    epiphany_create_instance(&instance);
    epiphany_create_session(instance, &session);
    
    for (auto _ : state) {
        auto result = epiphany_execute_sql(session, "SELECT COUNT(*) FROM users", nullptr);
        benchmark::DoNotOptimize(result);
        epiphany_free_result(result);
    }
    
    epiphany_destroy_session(session);
    epiphany_destroy_instance(instance);
}
BENCHMARK(BM_ConcurrentQueries)->ThreadRange(1, 16);

BENCHMARK_MAIN();
```

## 📁 测试组织结构

```
tests/
├── CMakeLists.txt          # 测试构建配置
├── unit/                   # 单元测试
│   ├── CMakeLists.txt
│   ├── core/               # 核心模块测试
│   │   ├── engine_test.cpp
│   │   ├── query_test.cpp
│   │   └── transaction_test.cpp
│   ├── storage/            # 存储层测试
│   │   ├── row_store_test.cpp
│   │   ├── column_store_test.cpp
│   │   ├── vector_store_test.cpp
│   │   └── timeseries_store_test.cpp
│   ├── api/                # API 测试
│   │   └── epiphany_api_test.cpp
│   └── utils/              # 工具类测试
│       └── utils_test.cpp
├── integration/            # 集成测试
│   ├── CMakeLists.txt
│   ├── multimodal_test.cpp
│   ├── cross_storage_test.cpp
│   └── transaction_integration_test.cpp
├── performance/            # 性能测试
│   ├── CMakeLists.txt
│   ├── benchmark_suite.cpp
│   └── stress_test.cpp
├── e2e/                    # 端到端测试
│   ├── CMakeLists.txt
│   ├── user_scenarios_test.cpp
│   └── compatibility_test.cpp
└── fixtures/               # 测试数据
    ├── sample_data.sql
    ├── test_vectors.bin
    └── timeseries_data.csv
```

## 🧪 编写测试

### 单元测试最佳实践

#### 1. 测试命名规范

```cpp
// 格式: TEST(TestSuiteName, ShouldDoSomethingWhenCondition)
TEST(QueryEngineTest, ShouldReturnErrorWhenSQLIsInvalid) {
    QueryEngine engine;
    auto result = engine.execute("INVALID SQL");
    EXPECT_FALSE(result.isSuccess());
    EXPECT_EQ(result.getErrorCode(), ERROR_SYNTAX);
}

TEST(StorageEngineTest, ShouldCreateTableWhenValidSchema) {
    StorageEngine engine;
    TableSchema schema = createValidSchema();
    auto result = engine.createTable("test_table", schema);
    EXPECT_TRUE(result.isSuccess());
}
```

#### 2. 断言选择

```cpp
// 使用 EXPECT_* 而不是 ASSERT_*（除非测试无法继续）
TEST(MathTest, BasicOperations) {
    EXPECT_EQ(add(2, 3), 5);        // 相等比较
    EXPECT_NE(add(2, 3), 6);        // 不等比较
    EXPECT_LT(2, 3);                // 小于
    EXPECT_LE(2, 3);                // 小于等于
    EXPECT_GT(3, 2);                // 大于
    EXPECT_GE(3, 2);                // 大于等于
    
    EXPECT_TRUE(isValid());         // 布尔值
    EXPECT_FALSE(isEmpty());
    
    EXPECT_STREQ("hello", str);     // 字符串比较
    EXPECT_STRNE("hello", "world");
    
    EXPECT_FLOAT_EQ(1.0f, result);  // 浮点数比较
    EXPECT_DOUBLE_EQ(1.0, result);
    
    EXPECT_THROW(func(), std::exception);  // 异常测试
    EXPECT_NO_THROW(func());
}
```

#### 3. 测试数据管理

```cpp
class DatabaseTestFixture : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建测试数据库
        createTestDatabase();
        loadTestData();
    }
    
    void TearDown() override {
        // 清理测试数据
        cleanupTestData();
        destroyTestDatabase();
    }
    
    // 辅助方法
    void createTestTable(const std::string& name) {
        std::string sql = "CREATE TABLE " + name + " (id INT, name VARCHAR(100))";
        executeSQL(sql);
    }
    
    void insertTestData(const std::string& table, int count) {
        for (int i = 0; i < count; ++i) {
            std::string sql = "INSERT INTO " + table + 
                             " VALUES (" + std::to_string(i) + ", 'test" + 
                             std::to_string(i) + "')";
            executeSQL(sql);
        }
    }
    
private:
    EPIPHANY_INSTANCE* instance_;
    EPIPHANY_SESSION* session_;
};
```

### 集成测试示例

```cpp
#include <gtest/gtest.h>
#include "epiphany/epiphany.h"

class MultiModalIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        epiphany_create_instance(&instance);
        epiphany_create_session(instance, &session);
    }
    
    void TearDown() override {
        epiphany_destroy_session(session);
        epiphany_destroy_instance(instance);
    }
    
    EPIPHANY_INSTANCE* instance = nullptr;
    EPIPHANY_SESSION* session = nullptr;
};

TEST_F(MultiModalIntegrationTest, ShouldSupportCrossModalQueries) {
    // 创建不同存储模式的表
    epiphany_execute_sql(session,
        "CREATE TABLE users (id INT, name VARCHAR(100)) STORAGE_MODE = ROW", nullptr);
    
    epiphany_execute_sql(session,
        "CREATE TABLE analytics (user_id INT, score FLOAT) STORAGE_MODE = COLUMN", nullptr);
    
    epiphany_execute_sql(session,
        "CREATE TABLE embeddings (user_id INT, vector VECTOR(128)) STORAGE_MODE = VECTOR", nullptr);
    
    // 插入测试数据
    epiphany_execute_sql(session,
        "INSERT INTO users VALUES (1, 'Alice'), (2, 'Bob')", nullptr);
    
    epiphany_execute_sql(session,
        "INSERT INTO analytics VALUES (1, 95.5), (2, 87.2)", nullptr);
    
    // 跨模态联合查询
    auto result = epiphany_execute_sql(session,
        "SELECT u.name, a.score FROM users u JOIN analytics a ON u.id = a.user_id", nullptr);
    
    EXPECT_EQ(result->error_code, EPIPHANY_SUCCESS);
    EXPECT_EQ(result->row_count, 2);
    
    epiphany_free_result(result);
}

TEST_F(MultiModalIntegrationTest, ShouldHandleTransactionsAcrossStorageModes) {
    // 开始事务
    epiphany_begin_transaction(session);
    
    // 在不同存储模式中执行操作
    epiphany_execute_sql(session,
        "CREATE TABLE orders (id INT, amount DECIMAL) STORAGE_MODE = ROW", nullptr);
    
    epiphany_execute_sql(session,
        "CREATE TABLE order_stats (date DATE, total DECIMAL) STORAGE_MODE = COLUMN", nullptr);
    
    epiphany_execute_sql(session,
        "INSERT INTO orders VALUES (1, 100.00)", nullptr);
    
    epiphany_execute_sql(session,
        "INSERT INTO order_stats VALUES ('2024-01-01', 100.00)", nullptr);
    
    // 提交事务
    auto commit_result = epiphany_commit_transaction(session);
    EXPECT_EQ(commit_result, EPIPHANY_SUCCESS);
    
    // 验证数据一致性
    auto result = epiphany_execute_sql(session,
        "SELECT COUNT(*) FROM orders", nullptr);
    EXPECT_EQ(result->row_count, 1);
    
    epiphany_free_result(result);
}
```

## 🚀 运行测试

### 构建测试

```bash
# 配置构建（启用测试）
mkdir build && cd build
cmake -DENABLE_TESTING=ON ..

# 构建所有测试
make

# 构建特定测试
make unit_tests
make integration_tests
make performance_tests
```

### 运行测试

```bash
# 运行所有测试
make test

# 或者使用 ctest
ctest

# 运行特定测试套件
./tests/unit_tests
./tests/integration_tests

# 运行特定测试
./tests/unit_tests --gtest_filter=EngineTest.*

# 详细输出
./tests/unit_tests --gtest_verbose

# 并行运行
ctest -j$(nproc)

# 运行失败的测试
ctest --rerun-failed
```

### 测试报告

```bash
# 生成 XML 报告
./tests/unit_tests --gtest_output=xml:test_results.xml

# 生成 JSON 报告
./tests/unit_tests --gtest_output=json:test_results.json

# 使用 ctest 生成报告
ctest --output-on-failure --verbose
```

## 📊 代码覆盖率

### 配置覆盖率

```bash
# 使用 gcov
cmake -DENABLE_COVERAGE=ON -DCMAKE_BUILD_TYPE=Debug ..
make

# 运行测试
make test

# 生成覆盖率报告
make coverage

# 查看 HTML 报告
open coverage/index.html
```

### lcov 使用

```bash
# 安装 lcov
sudo apt install -y lcov

# 重置计数器
lcov --directory . --zerocounters

# 运行测试
make test

# 收集覆盖率数据
lcov --directory . --capture --output-file coverage.info

# 过滤系统文件
lcov --remove coverage.info '/usr/*' --output-file coverage.info
lcov --remove coverage.info '*/tests/*' --output-file coverage.info

# 生成 HTML 报告
genhtml coverage.info --output-directory coverage_html
```

## 🔧 测试工具

### 内存检查

```bash
# Valgrind
valgrind --tool=memcheck --leak-check=full ./tests/unit_tests

# AddressSanitizer
cmake -DENABLE_ASAN=ON ..
make
./tests/unit_tests
```

### 线程检查

```bash
# ThreadSanitizer
cmake -DENABLE_TSAN=ON ..
make
./tests/unit_tests
```

### 模糊测试

```cpp
#include <fuzzer/FuzzedDataProvider.h>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    FuzzedDataProvider fdp(data, size);
    
    // 生成随机 SQL
    std::string sql = fdp.ConsumeRandomLengthString(1000);
    
    EPIPHANY_INSTANCE* instance;
    EPIPHANY_SESSION* session;
    
    epiphany_create_instance(&instance);
    epiphany_create_session(instance, &session);
    
    // 执行模糊测试
    auto result = epiphany_execute_sql(session, sql.c_str(), nullptr);
    if (result) {
        epiphany_free_result(result);
    }
    
    epiphany_destroy_session(session);
    epiphany_destroy_instance(instance);
    
    return 0;
}
```

## 📈 性能测试

### 基准测试

```cpp
// 吞吐量测试
static void BM_Throughput(benchmark::State& state) {
    const int operations = state.range(0);
    
    for (auto _ : state) {
        performOperations(operations);
    }
    
    state.SetItemsProcessed(state.iterations() * operations);
}
BENCHMARK(BM_Throughput)->Range(1000, 100000);

// 延迟测试
static void BM_Latency(benchmark::State& state) {
    for (auto _ : state) {
        auto start = std::chrono::high_resolution_clock::now();
        performSingleOperation();
        auto end = std::chrono::high_resolution_clock::now();
        
        auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
        state.SetIterationTime(elapsed.count() * 1e-9);
    }
}
BENCHMARK(BM_Latency)->UseManualTime();
```

### 压力测试

```cpp
class StressTest : public ::testing::Test {
protected:
    void SetUp() override {
        epiphany_create_instance(&instance);
    }
    
    void TearDown() override {
        epiphany_destroy_instance(instance);
    }
    
    EPIPHANY_INSTANCE* instance = nullptr;
};

TEST_F(StressTest, ShouldHandleHighConcurrency) {
    const int thread_count = 100;
    const int operations_per_thread = 1000;
    
    std::vector<std::thread> threads;
    std::atomic<int> success_count{0};
    std::atomic<int> error_count{0};
    
    for (int i = 0; i < thread_count; ++i) {
        threads.emplace_back([&, i]() {
            EPIPHANY_SESSION* session;
            epiphany_create_session(instance, &session);
            
            for (int j = 0; j < operations_per_thread; ++j) {
                auto result = epiphany_execute_sql(session, "SELECT 1", nullptr);
                if (result && result->error_code == EPIPHANY_SUCCESS) {
                    success_count++;
                } else {
                    error_count++;
                }
                if (result) epiphany_free_result(result);
            }
            
            epiphany_destroy_session(session);
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    EXPECT_GT(success_count.load(), thread_count * operations_per_thread * 0.95);
    EXPECT_LT(error_count.load(), thread_count * operations_per_thread * 0.05);
}
```

## 🔄 持续集成

### GitHub Actions 配置

```yaml
name: Tests

on: [push, pull_request]

jobs:
  unit-tests:
    runs-on: ubuntu-latest
    
    steps:
    - uses: actions/checkout@v2
    
    - name: Install dependencies
      run: |
        sudo apt update
        sudo apt install -y build-essential cmake libgtest-dev
    
    - name: Configure
      run: |
        mkdir build && cd build
        cmake -DENABLE_TESTING=ON -DENABLE_COVERAGE=ON ..
    
    - name: Build
      run: |
        cd build
        make -j$(nproc)
    
    - name: Run tests
      run: |
        cd build
        make test
    
    - name: Generate coverage
      run: |
        cd build
        make coverage
    
    - name: Upload coverage
      uses: codecov/codecov-action@v1
      with:
        file: ./build/coverage.info

  integration-tests:
    runs-on: ubuntu-latest
    needs: unit-tests
    
    steps:
    - uses: actions/checkout@v2
    
    - name: Run integration tests
      run: |
        mkdir build && cd build
        cmake -DENABLE_TESTING=ON ..
        make integration_tests
        ./tests/integration_tests

  performance-tests:
    runs-on: ubuntu-latest
    
    steps:
    - uses: actions/checkout@v2
    
    - name: Run benchmarks
      run: |
        mkdir build && cd build
        cmake -DENABLE_BENCHMARKS=ON ..
        make benchmark_suite
        ./benchmarks/benchmark_suite --benchmark_format=json > benchmark_results.json
    
    - name: Store benchmark results
      uses: benchmark-action/github-action-benchmark@v1
      with:
        tool: 'googlecpp'
        output-file-path: build/benchmark_results.json
```

## 📋 测试清单

### 提交前检查

- [ ] 所有单元测试通过
- [ ] 代码覆盖率 ≥ 90%
- [ ] 集成测试通过
- [ ] 性能测试无回归
- [ ] 内存检查无泄漏
- [ ] 线程安全检查通过

### 发布前检查

- [ ] 完整测试套件通过
- [ ] 压力测试通过
- [ ] 兼容性测试通过
- [ ] 端到端测试通过
- [ ] 文档测试通过
- [ ] 安全测试通过

## 🐛 调试测试

### 调试失败的测试

```bash
# 运行特定测试并调试
gdb --args ./tests/unit_tests --gtest_filter=FailingTest.*

# 在 GDB 中
(gdb) run
(gdb) bt
(gdb) print variable_name
```

### 测试隔离

```bash
# 运行单个测试
./tests/unit_tests --gtest_filter=SpecificTest.SpecificCase

# 重复运行以发现间歇性问题
./tests/unit_tests --gtest_repeat=100 --gtest_break_on_failure
```

## 📚 最佳实践

### 测试设计原则

1. **FIRST 原则**
   - **Fast**: 测试应该快速运行
   - **Independent**: 测试之间应该独立
   - **Repeatable**: 测试应该可重复
   - **Self-Validating**: 测试应该有明确的通过/失败结果
   - **Timely**: 测试应该及时编写

2. **AAA 模式**
   ```cpp
   TEST(ExampleTest, ShouldDoSomething) {
       // Arrange - 准备测试数据
       Engine engine;
       std::string input = "test data";
       
       // Act - 执行被测试的操作
       auto result = engine.process(input);
       
       // Assert - 验证结果
       EXPECT_TRUE(result.isSuccess());
       EXPECT_EQ(result.getValue(), "expected output");
   }
   ```

3. **测试命名**
   - 使用描述性名称
   - 说明测试的条件和期望结果
   - 遵循一致的命名约定

### 常见陷阱

1. **避免测试实现细节**
2. **不要编写脆弱的测试**
3. **避免测试之间的依赖**
4. **不要忽略测试维护**
5. **避免过度模拟**

---

通过遵循本指南，您可以为 EpiphanyDB 项目编写高质量、可维护的测试代码！