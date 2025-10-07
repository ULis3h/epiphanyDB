# EpiphanyDB 多存储引擎集成设计

## 项目概述

基于 PostgreSQL 17 存储管理器接口，设计和实现多存储引擎架构，支持行存储、列存储、向量存储、时序存储和图存储的统一查询平台。

## PostgreSQL 存储管理器架构分析

### 核心接口结构

PostgreSQL 通过 `f_smgr` 结构体定义存储管理器接口：

```c
typedef struct f_smgr
{
    void (*smgr_init)(void);                    // 初始化
    void (*smgr_shutdown)(void);                // 关闭
    void (*smgr_open)(SMgrRelation reln);       // 打开关系
    void (*smgr_close)(SMgrRelation reln, ForkNumber forknum);  // 关闭
    void (*smgr_create)(SMgrRelation reln, ForkNumber forknum, bool isRedo);  // 创建
    bool (*smgr_exists)(SMgrRelation reln, ForkNumber forknum);  // 检查存在
    void (*smgr_unlink)(RelFileLocatorBackend rlocator, ForkNumber forknum, bool isRedo);  // 删除
    void (*smgr_extend)(SMgrRelation reln, ForkNumber forknum, BlockNumber blocknum, const void *buffer, bool skipFsync);  // 扩展
    void (*smgr_zeroextend)(SMgrRelation reln, ForkNumber forknum, BlockNumber blocknum, int nblocks, bool skipFsync);  // 零扩展
    bool (*smgr_prefetch)(SMgrRelation reln, ForkNumber forknum, BlockNumber blocknum, int nblocks);  // 预取
    void (*smgr_readv)(SMgrRelation reln, ForkNumber forknum, BlockNumber blocknum, void **buffers, BlockNumber nblocks);  // 批量读
    void (*smgr_writev)(SMgrRelation reln, ForkNumber forknum, BlockNumber blocknum, const void **buffers, BlockNumber nblocks, bool skipFsync);  // 批量写
    void (*smgr_writeback)(SMgrRelation reln, ForkNumber forknum, BlockNumber blocknum, BlockNumber nblocks);  // 写回
    BlockNumber (*smgr_nblocks)(SMgrRelation reln, ForkNumber forknum);  // 获取块数
    void (*smgr_truncate)(SMgrRelation reln, ForkNumber forknum, BlockNumber nblocks);  // 截断
    void (*smgr_immedsync)(SMgrRelation reln, ForkNumber forknum);  // 立即同步
    void (*smgr_registersync)(SMgrRelation reln, ForkNumber forknum);  // 注册同步
} f_smgr;
```

### SMgrRelation 结构

```c
typedef struct SMgrRelationData
{
    RelFileLocatorBackend smgr_rlocator;        // 关系物理标识符
    BlockNumber smgr_targblock;                 // 当前插入目标块
    BlockNumber smgr_cached_nblocks[MAX_FORKNUM + 1];  // 每个分支的已知大小
    int smgr_which;                             // 存储管理器选择器
    // ... 其他字段
} SMgrRelationData;
```

## EpiphanyDB 多存储引擎架构

### 1. 存储引擎类型定义

```c
typedef enum StorageEngineType
{
    STORAGE_ENGINE_HEAP = 0,        // PostgreSQL 原生堆存储 (行存储)
    STORAGE_ENGINE_COLUMNAR,        // 列存储引擎 (Apache Arrow)
    STORAGE_ENGINE_VECTOR,          // 向量存储引擎 (Faiss)
    STORAGE_ENGINE_TIMESERIES,      // 时序存储引擎 (InfluxDB-like)
    STORAGE_ENGINE_GRAPH,           // 图存储引擎 (Neo4j-like)
    STORAGE_ENGINE_COUNT            // 存储引擎总数
} StorageEngineType;
```

### 2. 扩展的存储管理器接口

```c
typedef struct epiphany_smgr
{
    f_smgr base;                    // 继承 PostgreSQL 基础接口
    
    // 扩展接口
    StorageEngineType engine_type;
    void (*engine_init)(void);
    void (*engine_shutdown)(void);
    
    // 多模态查询接口
    void (*vector_search)(SMgrRelation reln, const void *query_vector, 
                         int k, void **results);
    void (*timeseries_query)(SMgrRelation reln, Timestamp start_time, 
                            Timestamp end_time, void **results);
    void (*graph_traverse)(SMgrRelation reln, const void *start_node, 
                          int max_depth, void **results);
    
    // 统计和优化接口
    void (*collect_stats)(SMgrRelation reln, void **stats);
    void (*optimize_storage)(SMgrRelation reln);
} epiphany_smgr;
```

### 3. 存储引擎注册机制

```c
// 存储引擎注册表
static const epiphany_smgr epiphany_smgrsw[STORAGE_ENGINE_COUNT] = {
    // PostgreSQL 原生堆存储
    [STORAGE_ENGINE_HEAP] = {
        .base = {
            .smgr_init = mdinit,
            .smgr_shutdown = NULL,
            .smgr_open = mdopen,
            // ... 其他 PostgreSQL 原生接口
        },
        .engine_type = STORAGE_ENGINE_HEAP,
        .engine_init = heap_engine_init,
        .engine_shutdown = heap_engine_shutdown,
    },
    
    // 列存储引擎
    [STORAGE_ENGINE_COLUMNAR] = {
        .base = {
            .smgr_init = columnar_init,
            .smgr_shutdown = columnar_shutdown,
            .smgr_open = columnar_open,
            // ... 列存储接口实现
        },
        .engine_type = STORAGE_ENGINE_COLUMNAR,
        .engine_init = columnar_engine_init,
        .engine_shutdown = columnar_engine_shutdown,
    },
    
    // 向量存储引擎
    [STORAGE_ENGINE_VECTOR] = {
        .base = {
            .smgr_init = vector_init,
            .smgr_shutdown = vector_shutdown,
            .smgr_open = vector_open,
            // ... 向量存储接口实现
        },
        .engine_type = STORAGE_ENGINE_VECTOR,
        .engine_init = vector_engine_init,
        .engine_shutdown = vector_engine_shutdown,
        .vector_search = faiss_vector_search,
    },
    
    // 时序存储引擎
    [STORAGE_ENGINE_TIMESERIES] = {
        .base = {
            .smgr_init = timeseries_init,
            .smgr_shutdown = timeseries_shutdown,
            .smgr_open = timeseries_open,
            // ... 时序存储接口实现
        },
        .engine_type = STORAGE_ENGINE_TIMESERIES,
        .engine_init = timeseries_engine_init,
        .engine_shutdown = timeseries_engine_shutdown,
        .timeseries_query = influx_timeseries_query,
    },
    
    // 图存储引擎
    [STORAGE_ENGINE_GRAPH] = {
        .base = {
            .smgr_init = graph_init,
            .smgr_shutdown = graph_shutdown,
            .smgr_open = graph_open,
            // ... 图存储接口实现
        },
        .engine_type = STORAGE_ENGINE_GRAPH,
        .engine_init = graph_engine_init,
        .engine_shutdown = graph_engine_shutdown,
        .graph_traverse = neo4j_graph_traverse,
    }
};
```

## 存储引擎路由机制

### 1. 表级存储引擎选择

```sql
-- 创建表时指定存储引擎
CREATE TABLE users (
    id SERIAL PRIMARY KEY,
    name VARCHAR(100),
    email VARCHAR(100)
) USING heap_engine;

CREATE TABLE analytics_data (
    timestamp TIMESTAMPTZ,
    metric_name VARCHAR(50),
    value DOUBLE PRECISION
) USING columnar_engine;

CREATE TABLE embeddings (
    id SERIAL PRIMARY KEY,
    vector FLOAT8[],
    metadata JSONB
) USING vector_engine;

CREATE TABLE time_series (
    timestamp TIMESTAMPTZ,
    sensor_id INTEGER,
    temperature FLOAT8,
    humidity FLOAT8
) USING timeseries_engine;

CREATE TABLE social_graph (
    user_id INTEGER,
    friend_id INTEGER,
    relationship_type VARCHAR(20)
) USING graph_engine;
```

### 2. 存储引擎路由器

```c
typedef struct StorageRouter
{
    HTAB *table_engine_map;         // 表到存储引擎的映射
    HTAB *engine_instances;         // 存储引擎实例缓存
    
    // 路由决策函数
    StorageEngineType (*route_table)(Oid relid);
    StorageEngineType (*route_query)(Query *query);
    
    // 跨引擎查询支持
    void (*cross_engine_join)(SMgrRelation left_rel, SMgrRelation right_rel,
                             JoinType join_type, void **results);
} StorageRouter;

// 全局存储路由器
extern StorageRouter *global_storage_router;

// 路由器初始化
void storage_router_init(void);

// 获取表的存储引擎
StorageEngineType get_table_storage_engine(Oid relid);

// 创建存储引擎实例
SMgrRelation create_storage_engine_relation(RelFileLocator rlocator, 
                                           StorageEngineType engine_type);
```

## 查询处理流程

### 1. 查询解析和路由

```
SQL Query
    ↓
PostgreSQL Parser
    ↓
Query Tree
    ↓
Storage Router (分析涉及的表和存储引擎)
    ↓
┌─────────────────────────────────────────────────────────┐
│  单引擎查询        │        跨引擎查询                    │
├─────────────────────────────────────────────────────────┤
│  直接路由到        │   1. 分解查询                       │
│  对应存储引擎      │   2. 各引擎执行子查询               │
│                    │   3. 结果合并和后处理               │
└─────────────────────────────────────────────────────────┘
    ↓
Query Execution
    ↓
Results
```

### 2. 跨存储引擎查询示例

```sql
-- 示例：关联查询涉及多个存储引擎
SELECT 
    u.name,
    ts.avg_temperature,
    v.similarity_score
FROM 
    users u                          -- heap_engine
    JOIN (
        SELECT 
            sensor_id,
            AVG(temperature) as avg_temperature
        FROM time_series             -- timeseries_engine
        WHERE timestamp >= NOW() - INTERVAL '1 day'
        GROUP BY sensor_id
    ) ts ON u.id = ts.sensor_id
    JOIN (
        SELECT 
            id,
            vector_similarity(vector, $1) as similarity_score
        FROM embeddings              -- vector_engine
        WHERE vector_similarity(vector, $1) > 0.8
    ) v ON u.id = v.id;
```

处理流程：
1. **查询分解**：识别涉及三个不同存储引擎的表
2. **子查询执行**：
   - 时序引擎执行聚合查询
   - 向量引擎执行相似性搜索
   - 堆存储执行用户数据查询
3. **结果合并**：在 PostgreSQL 执行器层面进行 JOIN 操作

## 元数据管理

### 1. 存储引擎元数据表

```sql
-- 存储引擎配置表
CREATE TABLE epiphany_storage_engines (
    engine_id INTEGER PRIMARY KEY,
    engine_name VARCHAR(50) UNIQUE NOT NULL,
    engine_type INTEGER NOT NULL,
    config JSONB,
    created_at TIMESTAMPTZ DEFAULT NOW()
);

-- 表存储引擎映射表
CREATE TABLE epiphany_table_engines (
    relid OID PRIMARY KEY,
    engine_id INTEGER REFERENCES epiphany_storage_engines(engine_id),
    engine_config JSONB,
    created_at TIMESTAMPTZ DEFAULT NOW()
);

-- 存储引擎统计信息表
CREATE TABLE epiphany_engine_stats (
    relid OID,
    engine_id INTEGER,
    stat_name VARCHAR(50),
    stat_value JSONB,
    updated_at TIMESTAMPTZ DEFAULT NOW(),
    PRIMARY KEY (relid, engine_id, stat_name)
);
```

### 2. 系统目录扩展

```c
// 扩展 pg_class 系统目录
typedef struct FormData_pg_class_extended
{
    FormData_pg_class base;         // PostgreSQL 原生字段
    
    // EpiphanyDB 扩展字段
    int32 epiphany_engine_type;     // 存储引擎类型
    text epiphany_engine_config;    // 存储引擎配置
    Oid epiphany_engine_oid;        // 存储引擎实例 OID
} FormData_pg_class_extended;
```

## 性能优化策略

### 1. 缓存机制

```c
typedef struct EngineCache
{
    HTAB *relation_cache;           // 关系缓存
    HTAB *stats_cache;              // 统计信息缓存
    HTAB *query_plan_cache;         // 查询计划缓存
    
    // 缓存管理
    void (*invalidate_cache)(Oid relid);
    void (*refresh_stats)(Oid relid);
} EngineCache;
```

### 2. 并行处理

```c
// 并行查询执行器
typedef struct ParallelQueryExecutor
{
    int num_workers;                // 工作进程数
    BackgroundWorker *workers;      // 工作进程数组
    
    // 并行执行接口
    void (*execute_parallel)(Query *query, int num_partitions);
    void (*merge_results)(void **partial_results, int num_results);
} ParallelQueryExecutor;
```

### 3. 智能路由优化

```c
// 查询优化器扩展
typedef struct QueryOptimizer
{
    // 成本估算
    Cost (*estimate_engine_cost)(StorageEngineType engine_type, Query *query);
    
    // 执行计划优化
    Plan *(*optimize_cross_engine_plan)(Query *query);
    
    // 统计信息收集
    void (*collect_engine_statistics)(StorageEngineType engine_type);
} QueryOptimizer;
```

## 实现路线图

### Phase 1: 基础架构 (4周)
- [ ] **Week 1**: PostgreSQL 17 编译和基础配置
- [ ] **Week 2**: 存储管理器接口扩展
- [ ] **Week 3**: 存储引擎注册机制实现
- [ ] **Week 4**: 基础路由器实现

### Phase 2: 核心存储引擎 (8周)
- [ ] **Week 5-6**: 列存储引擎集成 (Apache Arrow)
- [ ] **Week 7-8**: 向量存储引擎集成 (Faiss)
- [ ] **Week 9-10**: 时序存储引擎集成 (InfluxDB-like)
- [ ] **Week 11-12**: 图存储引擎集成 (Neo4j-like)

### Phase 3: 查询处理 (4周)
- [ ] **Week 13**: 跨引擎查询处理
- [ ] **Week 14**: 查询优化器扩展
- [ ] **Week 15**: 并行查询执行
- [ ] **Week 16**: 性能调优和测试

### Phase 4: 企业特性 (2周)
- [ ] **Week 17**: 监控和管理工具
- [ ] **Week 18**: 文档和部署指南

## 技术挑战和解决方案

### 1. 事务一致性
**挑战**: 跨存储引擎的事务一致性保证
**解决方案**: 
- 利用 PostgreSQL 的两阶段提交 (2PC) 机制
- 实现分布式事务协调器
- 为每个存储引擎实现事务接口

### 2. 查询优化
**挑战**: 跨存储引擎查询的成本估算和优化
**解决方案**:
- 扩展 PostgreSQL 查询优化器
- 为每个存储引擎实现成本模型
- 实现智能查询路由算法

### 3. 数据一致性
**挑战**: 不同存储引擎间的数据一致性
**解决方案**:
- 实现统一的元数据管理
- 设计数据同步机制
- 提供数据迁移工具

### 4. 性能优化
**挑战**: 多存储引擎的性能协调
**解决方案**:
- 实现智能缓存机制
- 优化跨引擎数据传输
- 提供性能监控和调优工具

## 总结

EpiphanyDB 多存储引擎架构通过扩展 PostgreSQL 的存储管理器接口，实现了统一的多模态数据处理平台。该设计保持了与 PostgreSQL 生态的完全兼容性，同时提供了企业级的性能、可靠性和可扩展性。

通过模块化的架构设计，EpiphanyDB 可以根据不同的工作负载特点选择最优的存储引擎，并支持跨引擎的复杂查询处理，为用户提供统一、高效的数据访问体验。