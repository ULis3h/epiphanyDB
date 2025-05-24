#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <thread>
#include <vector>
#include <cassert>

#include "log.hpp"
#include "time.hpp"
#include "mem_manager.hpp"

using epiphany::LoggerFactory;
using epiphany::common::MemPool;
using epiphany::common::EPIPHANY_MEM_MANAGER;

#define MEM_POOL_SIZE (1024 * 1024 * 1)  // 1MB
#define SMALL_ALLOC_SIZE 16
#define MEDIUM_ALLOC_SIZE 1024
#define LARGE_ALLOC_SIZE (64 * 1024)

// 测试结果统计
struct TestStats {
    size_t total_tests;
    size_t passed_tests;
    double std_malloc_time;
    double mem_pool_time;
    
    TestStats() : total_tests(0), passed_tests(0), 
                 std_malloc_time(0), mem_pool_time(0) {}
};

// 验证内存内容
bool verify_memory(void* ptr, size_t size, uint8_t pattern) {
    uint8_t* mem = reinterpret_cast<uint8_t*>(ptr);
    for (size_t i = 0; i < size; i++) {
        if (mem[i] != pattern) {
            return false;
        }
    }
    return true;
}

// 基础分配测试
bool test_basic_allocation(TestStats& stats) {
    stats.total_tests++;
    
    // 测试小块内存分配
    void* ptr = MemPool::instance.alloc(SMALL_ALLOC_SIZE);
    if (!ptr) {
        LogErr("Basic allocation test failed: couldn't allocate small block");
        return false;
    }
    
    // 写入测试模式并验证
    memset(ptr, 0xAA, SMALL_ALLOC_SIZE);
    if (!verify_memory(ptr, SMALL_ALLOC_SIZE, 0xAA)) {
        LogErr("Basic allocation test failed: memory verification failed");
        return false;
    }
    
    MemPool::instance.free(ptr);
    stats.passed_tests++;
    return true;
}

int mem_manager_test_main() {   
    TestStats stats;
    
    // 初始化内存池
    void* base_addr = malloc(MEM_POOL_SIZE);
    if (!base_addr) {
        LogErr("Failed to allocate base memory pool");
        return -1;
    }
    
    MemPool::instance.init(base_addr, MEM_POOL_SIZE, EPIPHANY_MEM_MANAGER);
    LoggerFactory::init_default("", epiphany::LogLevel_Info, epiphany::LogLevel_Info);
    
    // 运行基础测试
    LogInfo("Running basic allocation tests...");
    if (!test_basic_allocation(stats)) {
        LogErr("Basic allocation tests failed");
    }
    
    // 清理
    free(base_addr);
    
    // 输出测试结果
    LogInfo("Test Results:");
    LogInfo("Total tests: %zu", stats.total_tests);
    LogInfo("Passed tests: %zu", stats.passed_tests);
    LogInfo("Failed tests: %zu", stats.total_tests - stats.passed_tests);
    
    return (stats.total_tests == stats.passed_tests) ? 0 : -1;
}
