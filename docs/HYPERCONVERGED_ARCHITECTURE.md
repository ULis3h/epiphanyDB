# EpiphanyDB 超融合数据库架构设计

## 项目愿景

构建基于 PostgreSQL 17 的超融合数据库系统，实现 OLTP、OLAP、向量搜索、时序分析、图计算的统一查询平台，对标 YMatrix，提供企业级的多模态数据处理能力。

## 核心设计理念

### 1. 统一查询引擎
- 基于 PostgreSQL 成熟的查询优化器
- 扩展支持多种数据模型和查询模式
- 保持 PostgreSQL 生态兼容性

### 2. 多存储引擎架构
- 插拔式存储引擎设计
- 智能存储选择和路由
- 跨存储引擎的统一事务管理

### 3. 云原生设计
- 容器化部署
- 弹性扩缩容
- 分布式计算支持

## 系统架构

```
┌─────────────────────────────────────────────────────────────┐
│                    Client Applications                       │
├─────────────────────────────────────────────────────────────┤
│                PostgreSQL Protocol Layer                     │
├─────────────────────────────────────────────────────────────┤
│                  Unified Query Engine                        │
│  ┌─────────────┐ ┌─────────────┐ ┌─────────────────────────┐ │
│  │   Parser    │ │  Planner    │ │      Optimizer          │ │
│  └─────────────┘ └─────────────┘ └─────────────────────────┘ │
├─────────────────────────────────────────────────────────────┤
│                   Storage Router                             │
│  ┌─────────────────────────────────────────────────────────┐ │
│  │              Metadata Manager                           │ │
│  └─────────────────────────────────────────────────────────┘ │
├─────────────────────────────────────────────────────────────┤
│                Multi-Storage Engine Layer                    │
│ ┌──────────┐ ┌──────────┐ ┌──────────┐ ┌──────────────────┐ │
│ │   Row    │ │  Column  │ │  Vector  │ │   Time Series    │ │
│ │  Store   │ │  Store   │ │  Store   │ │     Store        │ │
│ └──────────┘ └──────────┘ └──────────┘ └──────────────────┘ │
│ ┌──────────┐ ┌──────────┐ ┌──────────┐ ┌──────────────────┐ │
│ │  Graph   │ │  Object  │ │   Key    │ │     Cache        │ │
│ │  Store   │ │  Store   │ │  Value   │ │     Layer        │ │
│ └──────────┘ └──────────┘ └──────────┘ └──────────────────┘ │
├─────────────────────────────────────────────────────────────┤
│                    Storage Layer                             │
│  ┌─────────────────────────────────────────────────────────┐ │
│  │           Distributed File System                       │ │
│  └─────────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────────┘
```

## 核心组件设计

### 1. 统一查询引擎 (Unified Query Engine)

基于 PostgreSQL 17 的查询引擎，扩展支持多种数据模型：

```c
// 扩展的查询节点类型
typedef enum NodeTag {
    // PostgreSQL 原生节点
    T_SeqScan,
    T_IndexScan,
    T_BitmapHeapScan,
    
    // 扩展的存储引擎节点
    T_ColumnScan,      // 列存储扫描
    T_VectorScan,      // 向量搜索
    T_TimeSeriesScan,  // 时序查询
    T_GraphScan,       // 图遍历
    
    // 跨存储引擎操作
    T_CrossEngineJoin, // 跨引擎连接
    T_DataTransfer,    // 数据传输
} NodeTag;
```

### 2. 存储路由器 (Storage Router)

智能选择最优存储引擎：

```c
typedef struct StorageRouter {
    // 存储引擎注册表
    List *registered_engines;
    
    // 路由策略
    StorageRoutingPolicy *policy;
    
    // 统计信息收集器
    StatisticsCollector *stats_collector;
    
    // 成本估算器
    CostEstimator *cost_estimator;
} StorageRouter;

// 存储引擎选择函数
StorageEngine* select_optimal_engine(Query *query, 
                                    TableInfo *table_info,
                                    QueryContext *context);
```

### 3. 多存储引擎层

#### 3.1 行存储引擎 (Row Store)
- 基于 PostgreSQL 原生 Heap 存储
- 适用于 OLTP 工作负载
- 支持高并发事务处理

#### 3.2 列存储引擎 (Column Store)
```c
typedef struct ColumnStoreEngine {
    // Apache Arrow 集成
    ArrowSchema *schema;
    ArrowArray *data;
    
    // 压缩算法
    CompressionType compression;
    
    // 列式索引
    ColumnIndex *indexes;
    
    // 分区管理
    PartitionManager *partition_mgr;
} ColumnStoreEngine;
```

#### 3.3 向量存储引擎 (Vector Store)
```c
typedef struct VectorStoreEngine {
    // FAISS 索引集成
    FaissIndex *faiss_index;
    
    // 向量维度
    int dimension;
    
    // 距离度量
    DistanceMetric metric;
    
    // 向量压缩
    VectorCompression compression;
} VectorStoreEngine;
```

#### 3.4 时序存储引擎 (Time Series Store)
```c
typedef struct TimeSeriesEngine {
    // 时间分区策略
    TimePartitionStrategy partition_strategy;
    
    // 数据压缩
    TimeSeriesCompression compression;
    
    // 聚合预计算
    AggregationCache *agg_cache;
    
    // 保留策略
    RetentionPolicy *retention_policy;
} TimeSeriesEngine;
```

### 4. 元数据管理器 (Metadata Manager)

扩展 PostgreSQL 系统目录：

```sql
-- 存储引擎注册表
CREATE TABLE pg_storage_engines (
    engine_id oid NOT NULL,
    engine_name name NOT NULL,
    engine_type text NOT NULL,
    engine_config jsonb,
    is_active boolean DEFAULT true
);

-- 表存储映射
CREATE TABLE pg_table_storage (
    table_oid oid NOT NULL,
    engine_id oid NOT NULL,
    storage_config jsonb,
    created_at timestamp DEFAULT now()
);

-- 跨引擎统计信息
CREATE TABLE pg_cross_engine_stats (
    table_oid oid NOT NULL,
    engine_id oid NOT NULL,
    stat_name text NOT NULL,
    stat_value numeric,
    last_updated timestamp DEFAULT now()
);
```

## 查询处理流程

### 1. SQL 解析和重写
```
SQL Query → Parser → AST → Rewriter → Rewritten AST
```

### 2. 查询规划
```
Rewritten AST → Planner → Logical Plan → Storage Router → Physical Plan
```

### 3. 查询优化
```
Physical Plan → Optimizer → Optimized Plan → Cost Estimator → Final Plan
```

### 4. 查询执行
```
Final Plan → Executor → Storage Engines → Result Set → Client
```

## 跨存储引擎查询示例

### 示例 1: OLTP + OLAP 混合查询
```sql
-- 实时订单数据 (行存储) + 历史分析数据 (列存储)
SELECT 
    o.order_id,
    o.customer_id,
    o.order_date,
    h.total_orders,
    h.avg_order_value
FROM orders o  -- 行存储引擎
JOIN customer_history h  -- 列存储引擎
ON o.customer_id = h.customer_id
WHERE o.order_date >= CURRENT_DATE - INTERVAL '7 days';
```

### 示例 2: 向量搜索 + 关系查询
```sql
-- 商品向量搜索 + 商品信息查询
SELECT 
    p.product_id,
    p.product_name,
    p.price,
    v.similarity_score
FROM products p  -- 行存储引擎
JOIN (
    SELECT product_id, 
           vector_similarity(embedding, $1) as similarity_score
    FROM product_vectors  -- 向量存储引擎
    ORDER BY embedding <-> $1
    LIMIT 10
) v ON p.product_id = v.product_id;
```

### 示例 3: 时序数据分析
```sql
-- 时序数据聚合 + 维度表关联
SELECT 
    d.device_name,
    d.location,
    ts.avg_temperature,
    ts.max_temperature
FROM devices d  -- 行存储引擎
JOIN (
    SELECT device_id,
           AVG(temperature) as avg_temperature,
           MAX(temperature) as max_temperature
    FROM sensor_data  -- 时序存储引擎
    WHERE timestamp >= NOW() - INTERVAL '1 hour'
    GROUP BY device_id
) ts ON d.device_id = ts.device_id;
```

## 性能优化策略

### 1. 智能数据分布
- 基于查询模式的数据分布策略
- 热数据自动迁移到高性能存储
- 冷数据压缩和归档

### 2. 查询优化
- 跨存储引擎的谓词下推
- 智能连接算法选择
- 并行查询执行

### 3. 缓存策略
- 多级缓存架构
- 智能缓存替换算法
- 跨存储引擎的结果缓存

## 高可用和容灾

### 1. 数据复制
- 基于 PostgreSQL 流复制
- 跨存储引擎的一致性复制
- 异地容灾备份

### 2. 故障恢复
- 自动故障检测
- 快速故障切换
- 数据一致性保证

## 监控和管理

### 1. 性能监控
- 实时性能指标收集
- 查询执行计划分析
- 存储引擎性能对比

### 2. 管理工具
- Web 管理界面
- 命令行工具
- API 接口

## 部署架构

### 1. 单机部署
```
┌─────────────────────────────────────┐
│           EpiphanyDB                │
│  ┌─────────────────────────────────┐ │
│  │      Query Engine               │ │
│  ├─────────────────────────────────┤ │
│  │    Multi-Storage Engines        │ │
│  └─────────────────────────────────┘ │
└─────────────────────────────────────┘
```

### 2. 分布式部署
```
┌─────────────┐    ┌─────────────┐    ┌─────────────┐
│  Coordinator │    │    Node 1   │    │    Node 2   │
│             │    │             │    │             │
│ Query Router│◄──►│Storage Eng. │◄──►│Storage Eng. │
│             │    │             │    │             │
└─────────────┘    └─────────────┘    └─────────────┘
```

## 开发路线图

### Phase 1: 基础架构 (3个月)
- [ ] PostgreSQL 17 集成和编译
- [ ] 存储引擎抽象接口设计
- [ ] 基础的存储路由器实现
- [ ] 元数据管理器扩展

### Phase 2: 核心存储引擎 (6个月)
- [ ] 列存储引擎 (Apache Arrow)
- [ ] 向量存储引擎 (FAISS)
- [ ] 时序存储引擎 (InfluxDB 架构)
- [ ] 基础的跨引擎查询支持

### Phase 3: 查询优化 (3个月)
- [ ] 查询优化器扩展
- [ ] 跨存储引擎查询计划
- [ ] 性能调优和基准测试

### Phase 4: 企业级特性 (6个月)
- [ ] 高可用和容灾
- [ ] 分布式架构支持
- [ ] 监控和管理工具
- [ ] 生产环境部署

## 技术挑战和解决方案

### 挑战 1: PostgreSQL 内核修改复杂度
**解决方案**: 
- 最小化内核修改，主要通过扩展机制实现
- 利用 PostgreSQL 的 Hook 机制
- 基于 Table Access Method (TAM) API

### 挑战 2: 跨存储引擎事务一致性
**解决方案**:
- 基于 PostgreSQL 的两阶段提交 (2PC)
- 分布式事务协调器
- 最终一致性模型

### 挑战 3: 性能优化
**解决方案**:
- 智能查询路由和优化
- 并行查询执行
- 多级缓存策略

## 总结

EpiphanyDB 超融合数据库架构设计充分利用了 PostgreSQL 的成熟架构和扩展能力，通过创新的多存储引擎设计，实现了统一的多模态数据处理平台。该架构不仅保持了与 PostgreSQL 生态的兼容性，还提供了企业级的性能、可靠性和可扩展性。