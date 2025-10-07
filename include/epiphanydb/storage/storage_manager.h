/*-------------------------------------------------------------------------
 *
 * storage_manager.h
 *    EpiphanyDB 存储管理器扩展接口
 *
 * 基于 PostgreSQL 17 存储管理器接口，扩展支持多种存储引擎：
 * - 行存储 (PostgreSQL Heap)
 * - 列存储 (Apache Arrow)
 * - 向量存储 (Faiss)
 * - 时序存储 (InfluxDB-like)
 * - 图存储 (Neo4j-like)
 *
 * Copyright (c) 2024, EpiphanyDB Team
 *
 * IDENTIFICATION
 *    include/epiphanydb/storage/storage_manager.h
 *
 *-------------------------------------------------------------------------
 */
#ifndef EPIPHANYDB_STORAGE_MANAGER_H
#define EPIPHANYDB_STORAGE_MANAGER_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/* PostgreSQL 类型前向声明 */
typedef struct SMgrRelationData *SMgrRelation;
typedef uint32_t BlockNumber;
typedef int ForkNumber;
typedef uint32_t Oid;
typedef int ProcNumber;
typedef int64_t Timestamp;

/* PostgreSQL 结构体前向声明 */
typedef struct RelFileLocator
{
    uint32_t spcOid;        /* 表空间 OID */
    uint32_t dbOid;         /* 数据库 OID */
    uint32_t relNumber;     /* 关系编号 */
} RelFileLocator;

typedef struct RelFileLocatorBackend
{
    RelFileLocator locator; /* 基础定位器 */
    int            backend; /* 后端进程 ID */
} RelFileLocatorBackend;

typedef struct Query Query;

/* 哈希表类型定义 */
typedef struct HTAB HTAB;

/* SMgrRelationData 结构体定义 */
typedef struct SMgrRelationData
{
    RelFileLocatorBackend smgr_rlocator;    /* 关系文件定位器 */
    int                   smgr_targblock;   /* 目标块号 */
    int                   smgr_cached_nblocks[4]; /* 缓存的块数 */
    bool                  smgr_which;       /* 标志位 */
} SMgrRelationData;

/* 存储引擎类型枚举 */
typedef enum StorageEngineType
{
    STORAGE_ENGINE_HEAP = 0,        /* PostgreSQL 原生堆存储 (行存储) */
    STORAGE_ENGINE_COLUMNAR,        /* 列存储引擎 (Apache Arrow) */
    STORAGE_ENGINE_VECTOR,          /* 向量存储引擎 (Faiss) */
    STORAGE_ENGINE_TIMESERIES,      /* 时序存储引擎 (InfluxDB-like) */
    STORAGE_ENGINE_GRAPH,           /* 图存储引擎 (Neo4j-like) */
    STORAGE_ENGINE_COUNT            /* 存储引擎总数 */
} StorageEngineType;

/* 存储引擎状态 */
typedef enum StorageEngineStatus
{
    ENGINE_STATUS_UNINITIALIZED = 0,
    ENGINE_STATUS_INITIALIZING,
    ENGINE_STATUS_READY,
    ENGINE_STATUS_ERROR,
    ENGINE_STATUS_SHUTDOWN
} StorageEngineStatus;

/* 前向声明 */
typedef struct EpiphanyStorageEngine EpiphanyStorageEngine;
typedef struct EpiphanySmgrRelation EpiphanySmgrRelation;
typedef struct StorageRouter StorageRouter;

/* 向量查询参数 */
typedef struct VectorQueryParams
{
    const void *query_vector;       /* 查询向量 */
    int         vector_dim;         /* 向量维度 */
    int         k;                  /* 返回前 k 个结果 */
    float       threshold;          /* 相似度阈值 */
    const char *distance_metric;    /* 距离度量 (L2, cosine, dot) */
} VectorQueryParams;

/* 时序查询参数 */
typedef struct TimeseriesQueryParams
{
    Timestamp   start_time;         /* 开始时间 */
    Timestamp   end_time;           /* 结束时间 */
    const char *metric_name;        /* 指标名称 */
    const char *aggregation;        /* 聚合函数 (avg, sum, max, min) */
    int         interval_seconds;   /* 聚合间隔 (秒) */
} TimeseriesQueryParams;

/* 图遍历参数 */
typedef struct GraphTraverseParams
{
    const void *start_node;         /* 起始节点 */
    int         max_depth;          /* 最大遍历深度 */
    const char *relationship_type;  /* 关系类型 */
    const char *direction;          /* 遍历方向 (in, out, both) */
    bool        include_properties; /* 是否包含属性 */
} GraphTraverseParams;

/* 查询结果集 */
typedef struct QueryResultSet
{
    void      **results;            /* 结果数组 */
    int         num_results;        /* 结果数量 */
    size_t      result_size;        /* 单个结果大小 */
    void       *metadata;           /* 元数据 */
    void      (*free_results)(struct QueryResultSet *rs);  /* 释放函数 */
} QueryResultSet;

/* 存储引擎统计信息 */
typedef struct StorageEngineStats
{
    StorageEngineType engine_type;
    uint64_t    total_relations;    /* 总关系数 */
    uint64_t    total_blocks;       /* 总块数 */
    uint64_t    total_tuples;       /* 总元组数 */
    uint64_t    read_operations;    /* 读操作数 */
    uint64_t    write_operations;   /* 写操作数 */
    uint64_t    cache_hits;         /* 缓存命中数 */
    uint64_t    cache_misses;       /* 缓存未命中数 */
    double      avg_query_time;     /* 平均查询时间 (毫秒) */
    Timestamp   last_updated;       /* 最后更新时间 */
} StorageEngineStats;

/* 扩展的存储管理器接口 */
typedef struct EpiphanyStorageEngine
{
    /* 基础 PostgreSQL 存储管理器接口 */
    void        (*smgr_init)(void);
    void        (*smgr_shutdown)(void);
    void        (*smgr_open)(SMgrRelation reln);
    void        (*smgr_close)(SMgrRelation reln, ForkNumber forknum);
    void        (*smgr_create)(SMgrRelation reln, ForkNumber forknum, bool isRedo);
    bool        (*smgr_exists)(SMgrRelation reln, ForkNumber forknum);
    void        (*smgr_unlink)(RelFileLocatorBackend rlocator, ForkNumber forknum, bool isRedo);
    void        (*smgr_extend)(SMgrRelation reln, ForkNumber forknum, BlockNumber blocknum, 
                              const void *buffer, bool skipFsync);
    void        (*smgr_zeroextend)(SMgrRelation reln, ForkNumber forknum, BlockNumber blocknum, 
                                  int nblocks, bool skipFsync);
    bool        (*smgr_prefetch)(SMgrRelation reln, ForkNumber forknum, BlockNumber blocknum, 
                                int nblocks);
    void        (*smgr_readv)(SMgrRelation reln, ForkNumber forknum, BlockNumber blocknum,
                             void **buffers, BlockNumber nblocks);
    void        (*smgr_writev)(SMgrRelation reln, ForkNumber forknum, BlockNumber blocknum,
                              const void **buffers, BlockNumber nblocks, bool skipFsync);
    void        (*smgr_writeback)(SMgrRelation reln, ForkNumber forknum, BlockNumber blocknum, 
                                 BlockNumber nblocks);
    BlockNumber (*smgr_nblocks)(SMgrRelation reln, ForkNumber forknum);
    void        (*smgr_truncate)(SMgrRelation reln, ForkNumber forknum, BlockNumber nblocks);
    void        (*smgr_immedsync)(SMgrRelation reln, ForkNumber forknum);
    void        (*smgr_registersync)(SMgrRelation reln, ForkNumber forknum);

    /* EpiphanyDB 扩展接口 */
    StorageEngineType   engine_type;
    const char         *engine_name;
    StorageEngineStatus engine_status;
    
    /* 引擎生命周期管理 */
    void        (*engine_init)(void);
    void        (*engine_shutdown)(void);
    void        (*engine_configure)(const char *config_json);
    
    /* 多模态查询接口 */
    QueryResultSet* (*vector_search)(SMgrRelation reln, const VectorQueryParams *params);
    QueryResultSet* (*timeseries_query)(SMgrRelation reln, const TimeseriesQueryParams *params);
    QueryResultSet* (*graph_traverse)(SMgrRelation reln, const GraphTraverseParams *params);
    
    /* 批量操作接口 */
    void        (*bulk_insert)(SMgrRelation reln, void **tuples, int num_tuples);
    void        (*bulk_update)(SMgrRelation reln, void **tuples, int num_tuples);
    void        (*bulk_delete)(SMgrRelation reln, void **keys, int num_keys);
    
    /* 统计和优化接口 */
    StorageEngineStats* (*collect_stats)(SMgrRelation reln);
    void        (*optimize_storage)(SMgrRelation reln);
    void        (*rebuild_indexes)(SMgrRelation reln);
    
    /* 事务支持接口 */
    void        (*begin_transaction)(SMgrRelation reln);
    void        (*commit_transaction)(SMgrRelation reln);
    void        (*abort_transaction)(SMgrRelation reln);
    void        (*prepare_transaction)(SMgrRelation reln, const char *gid);
    
    /* 备份和恢复接口 */
    void        (*backup_relation)(SMgrRelation reln, const char *backup_path);
    void        (*restore_relation)(SMgrRelation reln, const char *backup_path);
    
    /* 监控和诊断接口 */
    void        (*get_health_status)(SMgrRelation reln, char **status_json);
    void        (*get_performance_metrics)(SMgrRelation reln, char **metrics_json);
    
} EpiphanyStorageEngine;

/* 扩展的 SMgrRelation 结构 */
typedef struct EpiphanySmgrRelation
{
    SMgrRelationData    base;               /* PostgreSQL 基础结构 */
    
    /* EpiphanyDB 扩展字段 */
    StorageEngineType   engine_type;        /* 存储引擎类型 */
    EpiphanyStorageEngine *engine;          /* 存储引擎实例 */
    void               *engine_private;     /* 引擎私有数据 */
    
    /* 缓存和优化 */
    void               *query_cache;        /* 查询缓存 */
    void               *stats_cache;        /* 统计信息缓存 */
    Timestamp           last_optimized;     /* 最后优化时间 */
    
    /* 事务状态 */
    bool                in_transaction;     /* 是否在事务中 */
    void               *transaction_state;  /* 事务状态 */
    
} EpiphanySmgrRelation;

/* 存储引擎路由器 */
typedef struct StorageRouter
{
    /* 路由表 */
    HTAB               *table_engine_map;   /* 表到存储引擎的映射 */
    HTAB               *engine_instances;   /* 存储引擎实例缓存 */
    
    /* 路由决策函数 */
    StorageEngineType   (*route_table)(Oid relid);
    StorageEngineType   (*route_query)(struct Query *query);
    
    /* 跨引擎查询支持 */
    QueryResultSet*     (*cross_engine_join)(EpiphanySmgrRelation *left_rel, 
                                            EpiphanySmgrRelation *right_rel,
                                            int join_type);
    
    /* 负载均衡 */
    EpiphanySmgrRelation* (*select_replica)(Oid relid, bool for_write);
    
    /* 统计信息 */
    uint64_t            total_queries;
    uint64_t            cross_engine_queries;
    double              avg_routing_time;
    
} StorageRouter;

/* 全局变量声明 */
extern EpiphanyStorageEngine epiphany_storage_engines[STORAGE_ENGINE_COUNT];
extern StorageRouter *global_storage_router;

/* 函数声明 */

/* 存储管理器初始化和关闭 */
extern void epiphany_smgr_init(void);
extern void epiphany_smgr_shutdown(void);

/* 存储引擎注册和管理 */
extern void register_storage_engine(StorageEngineType type, EpiphanyStorageEngine *engine);
extern EpiphanyStorageEngine* get_storage_engine(StorageEngineType type);
extern const char* storage_engine_type_name(StorageEngineType type);

/* 存储引擎路由 */
extern void storage_router_init(void);
extern void storage_router_shutdown(void);
extern StorageEngineType get_table_storage_engine(Oid relid);
extern void set_table_storage_engine(Oid relid, StorageEngineType engine_type);

/* 关系管理 */
extern EpiphanySmgrRelation* epiphany_smgropen(RelFileLocator rlocator, ProcNumber backend);
extern void epiphany_smgrclose(EpiphanySmgrRelation *reln);
extern void epiphany_smgrdestroy(EpiphanySmgrRelation *reln);

/* 多模态查询接口 */
extern QueryResultSet* epiphany_vector_search(Oid relid, const VectorQueryParams *params);
extern QueryResultSet* epiphany_timeseries_query(Oid relid, const TimeseriesQueryParams *params);
extern QueryResultSet* epiphany_graph_traverse(Oid relid, const GraphTraverseParams *params);

/* 跨引擎查询 */
extern QueryResultSet* epiphany_cross_engine_query(struct Query *query);

/* 统计信息 */
extern StorageEngineStats* epiphany_get_engine_stats(StorageEngineType type);
extern void epiphany_reset_engine_stats(StorageEngineType type);

/* 配置管理 */
extern void epiphany_configure_engine(StorageEngineType type, const char *config_json);
extern char* epiphany_get_engine_config(StorageEngineType type);

/* 监控和诊断 */
extern char* epiphany_get_system_status(void);
extern char* epiphany_get_performance_report(void);

/* 工具函数 */
extern void free_query_result_set(QueryResultSet *rs);
extern QueryResultSet* create_query_result_set(int num_results, size_t result_size);

/* 错误处理 */
extern void epiphany_storage_error(const char *fmt, ...);
extern void epiphany_storage_warning(const char *fmt, ...);

/* 调试和日志 */
#ifdef DEBUG
extern void epiphany_storage_debug(const char *fmt, ...);
#else
#define epiphany_storage_debug(...)
#endif

/* 宏定义 */
#define EPIPHANY_SMGR_MAGIC     0x45504944  /* "EPID" */
#define EPIPHANY_VERSION_MAJOR  1
#define EPIPHANY_VERSION_MINOR  0
#define EPIPHANY_VERSION_PATCH  0

#define IsEpiphanySmgrRelation(reln) \
    ((reln) != NULL && ((EpiphanySmgrRelation*)(reln))->engine != NULL)

#define GetStorageEngine(reln) \
    (IsEpiphanySmgrRelation(reln) ? ((EpiphanySmgrRelation*)(reln))->engine : NULL)

#define GetEngineType(reln) \
    (IsEpiphanySmgrRelation(reln) ? ((EpiphanySmgrRelation*)(reln))->engine_type : STORAGE_ENGINE_HEAP)

/* 常量定义 */
#define EPIPHANY_MAX_ENGINE_NAME_LEN    64
#define EPIPHANY_MAX_CONFIG_LEN         4096
#define EPIPHANY_MAX_QUERY_CACHE_SIZE   1024
#define EPIPHANY_DEFAULT_VECTOR_DIM     512
#define EPIPHANY_MAX_GRAPH_DEPTH        10

#endif /* EPIPHANYDB_STORAGE_MANAGER_H */