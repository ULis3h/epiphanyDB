# 开发指南

本文档为 EpiphanyDB 开发者提供详细的开发环境搭建和开发流程指导。

## 🛠️ 开发环境搭建

### 系统要求

#### 最低要求
- **操作系统**: Linux (Ubuntu 18.04+), macOS (10.14+), Windows (10+)
- **内存**: 4GB RAM
- **存储**: 2GB 可用空间
- **编译器**: 支持 C++11 的编译器

#### 推荐配置
- **操作系统**: Linux (Ubuntu 20.04+), macOS (11.0+)
- **内存**: 8GB+ RAM
- **存储**: 10GB+ 可用空间 (SSD 推荐)
- **CPU**: 4+ 核心

### 依赖工具

#### 必需工具
```bash
# Ubuntu/Debian
sudo apt update
sudo apt install -y \
    build-essential \
    cmake \
    git \
    pkg-config \
    libtool \
    autoconf

# macOS (使用 Homebrew)
brew install cmake git pkg-config libtool autoconf

# CentOS/RHEL
sudo yum groupinstall -y "Development Tools"
sudo yum install -y cmake git pkgconfig libtool autoconf
```

#### 可选工具
```bash
# 代码格式化
sudo apt install -y clang-format

# 静态分析
sudo apt install -y cppcheck clang-tidy

# 文档生成
sudo apt install -y doxygen graphviz

# 性能分析
sudo apt install -y valgrind perf

# 调试工具
sudo apt install -y gdb lldb
```

### 编译器支持

#### GCC
- **最低版本**: GCC 4.8+
- **推荐版本**: GCC 9.0+
- **安装**:
  ```bash
  # Ubuntu
  sudo apt install -y gcc-9 g++-9
  
  # 设置默认版本
  sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-9 90
  sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-9 90
  ```

#### Clang
- **最低版本**: Clang 3.4+
- **推荐版本**: Clang 10.0+
- **安装**:
  ```bash
  # Ubuntu
  sudo apt install -y clang-10
  
  # macOS (通过 Xcode)
  xcode-select --install
  ```

#### MSVC (Windows)
- **最低版本**: Visual Studio 2015
- **推荐版本**: Visual Studio 2019+

## 🏗️ 项目结构

```
epiphanyDB/
├── src/                    # 源代码
│   ├── core/              # 核心模块
│   │   ├── engine/        # 存储引擎
│   │   ├── query/         # 查询处理
│   │   ├── index/         # 索引管理
│   │   └── transaction/   # 事务管理
│   ├── storage/           # 存储层
│   │   ├── row/           # 行存储
│   │   ├── column/        # 列存储
│   │   ├── vector/        # 向量存储
│   │   └── timeseries/    # 时序存储
│   ├── api/               # API 接口
│   └── utils/             # 工具类
├── include/               # 头文件
│   └── epiphany/          # 公共头文件
├── examples/              # 示例程序
├── tests/                 # 测试代码
│   ├── unit/              # 单元测试
│   ├── integration/       # 集成测试
│   └── performance/       # 性能测试
├── benchmarks/            # 基准测试
├── docs/                  # 文档
├── tools/                 # 开发工具
├── scripts/               # 构建脚本
└── cmake/                 # CMake 模块
```

## 🔧 构建系统

### CMake 配置

#### 基本构建
```bash
# 创建构建目录
mkdir build && cd build

# 配置项目
cmake ..

# 编译
make -j$(nproc)
```

#### 构建选项
```bash
# 调试构建
cmake -DCMAKE_BUILD_TYPE=Debug ..

# 发布构建
cmake -DCMAKE_BUILD_TYPE=Release ..

# 启用测试
cmake -DENABLE_TESTING=ON ..

# 启用基准测试
cmake -DENABLE_BENCHMARKS=ON ..

# 启用代码覆盖率
cmake -DENABLE_COVERAGE=ON ..

# 启用静态分析
cmake -DENABLE_STATIC_ANALYSIS=ON ..

# 自定义安装路径
cmake -DCMAKE_INSTALL_PREFIX=/usr/local ..
```

#### 高级选项
```bash
# 指定编译器
cmake -DCMAKE_C_COMPILER=gcc-9 -DCMAKE_CXX_COMPILER=g++-9 ..

# 启用 LTO (链接时优化)
cmake -DENABLE_LTO=ON ..

# 启用 AddressSanitizer
cmake -DENABLE_ASAN=ON ..

# 启用 ThreadSanitizer
cmake -DENABLE_TSAN=ON ..

# 启用 UndefinedBehaviorSanitizer
cmake -DENABLE_UBSAN=ON ..
```

### 构建目标

```bash
# 构建所有目标
make

# 构建特定目标
make epiphany_core
make simple_demo
make benchmark_suite

# 运行测试
make test

# 运行基准测试
make run_benchmarks

# 安装
make install

# 生成文档
make docs

# 代码格式化
make format

# 静态分析
make static_analysis
```

## 🧪 测试框架

### 单元测试

使用 Google Test 框架：

```cpp
#include <gtest/gtest.h>
#include "epiphany/core/engine.h"

class EngineTest : public ::testing::Test {
protected:
    void SetUp() override {
        engine = std::make_unique<Engine>();
    }
    
    void TearDown() override {
        engine.reset();
    }
    
    std::unique_ptr<Engine> engine;
};

TEST_F(EngineTest, ShouldCreateInstance) {
    EXPECT_NE(engine, nullptr);
}

TEST_F(EngineTest, ShouldExecuteQuery) {
    auto result = engine->executeQuery("SELECT 1");
    EXPECT_TRUE(result.isValid());
}
```

### 集成测试

```cpp
#include <gtest/gtest.h>
#include "epiphany/epiphany.h"

class IntegrationTest : public ::testing::Test {
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

TEST_F(IntegrationTest, ShouldCreateAndQueryTable) {
    // 创建表
    auto result = epiphany_execute_sql(session,
        "CREATE TABLE test (id INT, name VARCHAR(100))", nullptr);
    EXPECT_EQ(result->error_code, EPIPHANY_SUCCESS);
    
    // 插入数据
    result = epiphany_execute_sql(session,
        "INSERT INTO test VALUES (1, 'Alice')", nullptr);
    EXPECT_EQ(result->error_code, EPIPHANY_SUCCESS);
    
    // 查询数据
    result = epiphany_execute_sql(session,
        "SELECT * FROM test", nullptr);
    EXPECT_EQ(result->error_code, EPIPHANY_SUCCESS);
    EXPECT_EQ(result->row_count, 1);
}
```

### 性能测试

```cpp
#include <benchmark/benchmark.h>
#include "epiphany/epiphany.h"

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
```

## 🔍 调试和分析

### 调试构建

```bash
# 调试构建
cmake -DCMAKE_BUILD_TYPE=Debug ..
make

# 使用 GDB 调试
gdb ./examples/simple_demo
(gdb) run
(gdb) bt
(gdb) print variable_name
```

### 内存检查

```bash
# 使用 Valgrind
valgrind --tool=memcheck --leak-check=full ./examples/simple_demo

# 使用 AddressSanitizer
cmake -DENABLE_ASAN=ON ..
make
./examples/simple_demo
```

### 性能分析

```bash
# 使用 perf
perf record ./benchmarks/benchmark_suite
perf report

# 使用 Callgrind
valgrind --tool=callgrind ./benchmarks/benchmark_suite
kcachegrind callgrind.out.*
```

## 📝 代码规范

### 格式化

使用 clang-format：

```bash
# 格式化单个文件
clang-format -i src/core/engine.cpp

# 格式化所有文件
find src include -name "*.cpp" -o -name "*.h" | xargs clang-format -i
```

### 静态分析

```bash
# 使用 cppcheck
cppcheck --enable=all --std=c++11 src/

# 使用 clang-tidy
clang-tidy src/**/*.cpp -- -Iinclude
```

### 代码审查清单

- [ ] 代码符合项目风格
- [ ] 包含适当的注释
- [ ] 添加了必要的测试
- [ ] 更新了相关文档
- [ ] 通过了所有测试
- [ ] 没有内存泄漏
- [ ] 性能影响可接受

## 🚀 持续集成

### GitHub Actions

```yaml
name: CI

on: [push, pull_request]

jobs:
  build:
    runs-on: ubuntu-latest
    
    steps:
    - uses: actions/checkout@v2
    
    - name: Install dependencies
      run: |
        sudo apt update
        sudo apt install -y build-essential cmake
    
    - name: Configure
      run: |
        mkdir build
        cd build
        cmake -DENABLE_TESTING=ON ..
    
    - name: Build
      run: |
        cd build
        make -j$(nproc)
    
    - name: Test
      run: |
        cd build
        make test
```

### 本地 Git Hooks

```bash
# 安装 pre-commit hook
cp tools/pre-commit .git/hooks/
chmod +x .git/hooks/pre-commit
```

## 📚 开发工具

### IDE 配置

#### Visual Studio Code
推荐插件：
- C/C++ Extension Pack
- CMake Tools
- GitLens
- Doxygen Documentation Generator

#### CLion
- 导入 CMake 项目
- 配置代码风格
- 设置调试配置

### 有用的脚本

```bash
# 快速构建脚本
./scripts/build.sh

# 运行所有测试
./scripts/test.sh

# 代码格式化
./scripts/format.sh

# 静态分析
./scripts/analyze.sh
```

## 🐛 故障排除

### 常见问题

#### 编译错误
```bash
# 清理构建目录
rm -rf build
mkdir build && cd build
cmake ..
```

#### 链接错误
```bash
# 检查依赖库
ldd ./examples/simple_demo

# 更新 LD_LIBRARY_PATH
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
```

#### 测试失败
```bash
# 运行特定测试
./tests/unit_tests --gtest_filter=EngineTest.*

# 详细输出
./tests/unit_tests --gtest_verbose
```

### 获取帮助

- **GitHub Issues**: https://github.com/your-org/epiphanyDB/issues
- **讨论区**: https://github.com/your-org/epiphanyDB/discussions
- **邮件**: epiphanydb-dev@example.com

## 📈 性能优化

### 编译优化

```bash
# 最大优化
cmake -DCMAKE_BUILD_TYPE=Release -DENABLE_LTO=ON ..

# 针对本机优化
cmake -DCMAKE_CXX_FLAGS="-march=native" ..
```

### 运行时优化

```bash
# 设置线程数
export EPIPHANY_THREADS=8

# 设置内存限制
export EPIPHANY_MEMORY_LIMIT=4G

# 启用性能监控
export EPIPHANY_ENABLE_PROFILING=1
```

## 🔄 发布流程

### 版本发布

1. **更新版本号**
   ```bash
   # 更新 CMakeLists.txt 中的版本
   sed -i 's/VERSION 1.0.0/VERSION 1.1.0/' CMakeLists.txt
   ```

2. **更新变更日志**
   ```bash
   # 编辑 CHANGELOG.md
   vim CHANGELOG.md
   ```

3. **创建标签**
   ```bash
   git tag -a v1.1.0 -m "Release version 1.1.0"
   git push origin v1.1.0
   ```

4. **构建发布包**
   ```bash
   ./scripts/package.sh
   ```

### 文档更新

```bash
# 生成 API 文档
make docs

# 更新网站
cd docs/website
npm run build
npm run deploy
```

---

如有任何开发相关问题，请随时联系开发团队！