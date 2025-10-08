/*
 * EpiphanyDB Storage Engines Test Suite
 * 
 * Comprehensive tests for all storage engines
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "../../include/epiphanydb.h"

/* Test result structure */
typedef struct TestResult {
    char *test_name;
    bool passed;
    char *error_message;
    double execution_time_ms;
} TestResult;

/* Test suite statistics */
typedef struct TestSuite {
    size_t total_tests;
    size_t passed_tests;
    size_t failed_tests;
    TestResult *results;
} TestSuite;

/* Global test suite */
static TestSuite g_test_suite = {0};

/* Test utility functions */
void test_init(void) {
    g_test_suite.total_tests = 0;
    g_test_suite.passed_tests = 0;
    g_test_suite.failed_tests = 0;
    g_test_suite.results = NULL;
}

void test_cleanup(void) {
    if (g_test_suite.results) {
        for (size_t i = 0; i < g_test_suite.total_tests; i++) {
            free(g_test_suite.results[i].test_name);
            if (g_test_suite.results[i].error_message) {
                free(g_test_suite.results[i].error_message);
            }
        }
        free(g_test_suite.results);
    }
}

void test_add_result(const char *test_name, bool passed, const char *error_message, double execution_time_ms) {
    g_test_suite.results = realloc(g_test_suite.results, (g_test_suite.total_tests + 1) * sizeof(TestResult));
    
    TestResult *result = &g_test_suite.results[g_test_suite.total_tests];
    result->test_name = strdup(test_name);
    result->passed = passed;
    result->error_message = error_message ? strdup(error_message) : NULL;
    result->execution_time_ms = execution_time_ms;
    
    g_test_suite.total_tests++;
    if (passed) {
        g_test_suite.passed_tests++;
    } else {
        g_test_suite.failed_tests++;
    }
}

void test_print_results(void) {
    printf("\n=== EpiphanyDB Storage Engine Test Results ===\n");
    printf("Total Tests: %zu\n", g_test_suite.total_tests);
    printf("Passed: %zu\n", g_test_suite.passed_tests);
    printf("Failed: %zu\n", g_test_suite.failed_tests);
    printf("Success Rate: %.2f%%\n", 
           g_test_suite.total_tests > 0 ? 
           (double)g_test_suite.passed_tests / g_test_suite.total_tests * 100.0 : 0.0);
    
    printf("\nDetailed Results:\n");
    for (size_t i = 0; i < g_test_suite.total_tests; i++) {
        TestResult *result = &g_test_suite.results[i];
        printf("[%s] %s (%.2fms)", 
               result->passed ? "PASS" : "FAIL", 
               result->test_name, 
               result->execution_time_ms);
        
        if (!result->passed && result->error_message) {
            printf(" - %s", result->error_message);
        }
        printf("\n");
    }
}

/* Core functionality tests */
void test_epiphanydb_context_creation(void) {
    clock_t start = clock();
    
    EpiphanyDBContext *ctx = NULL;
    int result = epiphanydb_init_context(&ctx);
    
    bool passed = (result == EPIPHANYDB_SUCCESS && ctx != NULL);
    
    if (ctx) {
        epiphanydb_cleanup_context(ctx);
    }
    
    clock_t end = clock();
    double execution_time = ((double)(end - start)) / CLOCKS_PER_SEC * 1000.0;
    
    test_add_result("Context Creation", passed, 
                   passed ? NULL : "Failed to create EpiphanyDB context", 
                   execution_time);
}

void test_storage_engine_availability(void) {
    clock_t start = clock();
    
    EpiphanyDBContext *ctx = NULL;
    epiphanydb_init_context(&ctx);
    
    bool heap_available = epiphanydb_is_storage_engine_available(ctx, EPIPHANYDB_STORAGE_HEAP);
    bool columnar_available = epiphanydb_is_storage_engine_available(ctx, EPIPHANYDB_STORAGE_COLUMNAR);
    bool vector_available = epiphanydb_is_storage_engine_available(ctx, EPIPHANYDB_STORAGE_VECTOR);
    bool timeseries_available = epiphanydb_is_storage_engine_available(ctx, EPIPHANYDB_STORAGE_TIMESERIES);
    bool graph_available = epiphanydb_is_storage_engine_available(ctx, EPIPHANYDB_STORAGE_GRAPH);
    
    bool passed = heap_available && columnar_available && vector_available && 
                  timeseries_available && graph_available;
    
    epiphanydb_cleanup_context(ctx);
    
    clock_t end = clock();
    double execution_time = ((double)(end - start)) / CLOCKS_PER_SEC * 1000.0;
    
    test_add_result("Storage Engine Availability", passed, 
                   passed ? NULL : "Not all storage engines are available", 
                   execution_time);
}

/* Heap storage tests */
void test_heap_table_creation(void) {
    clock_t start = clock();
    
    EpiphanyDBContext *ctx = NULL;
    epiphanydb_init_context(&ctx);
    
    int result = epiphanydb_create_table(ctx, "test_heap_table", EPIPHANYDB_STORAGE_HEAP, 
                                        "id INTEGER, name TEXT, age INTEGER");
    
    bool passed = (result == EPIPHANYDB_SUCCESS);
    
    epiphanydb_cleanup_context(ctx);
    
    clock_t end = clock();
    double execution_time = ((double)(end - start)) / CLOCKS_PER_SEC * 1000.0;
    
    test_add_result("Heap Table Creation", passed, 
                   passed ? NULL : "Failed to create heap table", 
                   execution_time);
}

/* Columnar storage tests */
void test_columnar_table_creation(void) {
    clock_t start = clock();
    
    EpiphanyDBContext *ctx = NULL;
    epiphanydb_init_context(&ctx);
    
    int result = epiphanydb_create_table(ctx, "test_columnar_table", EPIPHANYDB_STORAGE_COLUMNAR, 
                                        "id INTEGER, sales DOUBLE, region TEXT");
    
    bool passed = (result == EPIPHANYDB_SUCCESS);
    
    epiphanydb_cleanup_context(ctx);
    
    clock_t end = clock();
    double execution_time = ((double)(end - start)) / CLOCKS_PER_SEC * 1000.0;
    
    test_add_result("Columnar Table Creation", passed, 
                   passed ? NULL : "Failed to create columnar table", 
                   execution_time);
}

/* Vector storage tests */
void test_vector_table_creation(void) {
    clock_t start = clock();
    
    EpiphanyDBContext *ctx = NULL;
    epiphanydb_init_context(&ctx);
    
    int result = epiphanydb_create_table(ctx, "test_vector_table", EPIPHANYDB_STORAGE_VECTOR, 
                                        "id INTEGER, embedding VECTOR(768), metadata TEXT");
    
    bool passed = (result == EPIPHANYDB_SUCCESS);
    
    epiphanydb_cleanup_context(ctx);
    
    clock_t end = clock();
    double execution_time = ((double)(end - start)) / CLOCKS_PER_SEC * 1000.0;
    
    test_add_result("Vector Table Creation", passed, 
                   passed ? NULL : "Failed to create vector table", 
                   execution_time);
}

/* Time series storage tests */
void test_timeseries_table_creation(void) {
    clock_t start = clock();
    
    EpiphanyDBContext *ctx = NULL;
    epiphanydb_init_context(&ctx);
    
    int result = epiphanydb_create_table(ctx, "test_timeseries_table", EPIPHANYDB_STORAGE_TIMESERIES, 
                                        "timestamp TIMESTAMP, value DOUBLE, tags TEXT");
    
    bool passed = (result == EPIPHANYDB_SUCCESS);
    
    epiphanydb_cleanup_context(ctx);
    
    clock_t end = clock();
    double execution_time = ((double)(end - start)) / CLOCKS_PER_SEC * 1000.0;
    
    test_add_result("Time Series Table Creation", passed, 
                   passed ? NULL : "Failed to create time series table", 
                   execution_time);
}

/* Graph storage tests */
void test_graph_table_creation(void) {
    clock_t start = clock();
    
    EpiphanyDBContext *ctx = NULL;
    epiphanydb_init_context(&ctx);
    
    int result = epiphanydb_create_table(ctx, "test_graph_table", EPIPHANYDB_STORAGE_GRAPH, 
                                        "vertices (id INTEGER, label TEXT), edges (source INTEGER, target INTEGER, weight DOUBLE)");
    
    bool passed = (result == EPIPHANYDB_SUCCESS);
    
    epiphanydb_cleanup_context(ctx);
    
    clock_t end = clock();
    double execution_time = ((double)(end - start)) / CLOCKS_PER_SEC * 1000.0;
    
    test_add_result("Graph Table Creation", passed, 
                   passed ? NULL : "Failed to create graph table", 
                   execution_time);
}

/* Performance tests */
void test_bulk_insert_performance(void) {
    clock_t start = clock();
    
    EpiphanyDBContext *ctx = NULL;
    epiphanydb_init_context(&ctx);
    
    /* Create test table */
    epiphanydb_create_table(ctx, "perf_test_table", EPIPHANYDB_STORAGE_HEAP, 
                           "id INTEGER, data TEXT");
    
    EpiphanyDBTable *table = NULL;
    epiphanydb_open_table(ctx, "perf_test_table", &table);
    
    /* Insert 1000 rows */
    bool passed = true;
    for (int i = 0; i < 1000 && passed; i++) {
        char data[256];
        snprintf(data, sizeof(data), "test_data_%d", i);
        
        int result = epiphanydb_insert_row(table, &i, sizeof(i));
        if (result != EPIPHANYDB_SUCCESS) {
            passed = false;
        }
    }
    
    epiphanydb_close_table(table);
    epiphanydb_cleanup_context(ctx);
    
    clock_t end = clock();
    double execution_time = ((double)(end - start)) / CLOCKS_PER_SEC * 1000.0;
    
    test_add_result("Bulk Insert Performance (1000 rows)", passed, 
                   passed ? NULL : "Failed during bulk insert", 
                   execution_time);
}

/* Main test runner */
int main(void) {
    printf("Starting EpiphanyDB Storage Engine Tests...\n");
    
    test_init();
    
    /* Run core functionality tests */
    test_epiphanydb_context_creation();
    test_storage_engine_availability();
    
    /* Run storage engine specific tests */
    test_heap_table_creation();
    test_columnar_table_creation();
    test_vector_table_creation();
    test_timeseries_table_creation();
    test_graph_table_creation();
    
    /* Run performance tests */
    test_bulk_insert_performance();
    
    /* Print results */
    test_print_results();
    
    /* Cleanup */
    test_cleanup();
    
    return g_test_suite.failed_tests == 0 ? 0 : 1;
}