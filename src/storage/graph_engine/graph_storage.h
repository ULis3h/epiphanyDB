/*-------------------------------------------------------------------------
 *
 * graph_storage.h
 *    EpiphanyDB 图存储引擎头文件 (Neo4j 兼容)
 *
 * 基于图数据库设计，提供高效的图数据存储和遍历，
 * 支持节点、边、属性、索引和图算法。
 *
 * Copyright (c) 2024, EpiphanyDB Team
 *
 * IDENTIFICATION
 *    src/storage/graph_engine/graph_storage.h
 *
 *-------------------------------------------------------------------------
 */

#ifndef GRAPH_STORAGE_H
#define GRAPH_STORAGE_H

#include "../../../include/epiphanydb/storage/storage_manager.h"
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

/* 常量定义 */
#define GRAPH_FILE_MAGIC            0x47524150  /* "GRAP" */
#define GRAPH_FILE_VERSION          1
#define GRAPH_DEFAULT_BATCH_SIZE    1000
#define GRAPH_MAX_LABEL_LENGTH      256
#define GRAPH_MAX_PROPERTY_NAME     256
#define GRAPH_MAX_PROPERTY_VALUE    4096
#define GRAPH_BLOCK_SIZE            (8 * 1024)  /* 8KB */
#define GRAPH_INDEX_BLOCK_SIZE      (4 * 1024)  /* 4KB */

/* 图引擎配置 */
typedef struct GraphEngineConfig
{
    /* 存储配置 */
    uint32_t    node_block_size;
    uint32_t    edge_block_size;
    uint32_t    property_block_size;
    uint32_t    max_nodes_per_block;
    uint32_t    max_edges_per_block;
    
    /* 索引配置 */
    bool        enable_node_index;
    bool        enable_edge_index;
    bool        enable_property_index;
    bool        enable_label_index;
    char       *default_index_type;
    
    /* 查询配置 */
    uint32_t    max_traversal_depth;
    uint32_t    max_result_nodes;
    uint32_t    max_result_edges;
    bool        enable_query_cache;
    uint32_t    query_timeout_ms;
    
    /* 事务配置 */
    bool        enable_transactions;
    uint32_t    transaction_timeout_ms;
    bool        enable_write_ahead_log;
    
    /* 性能配置 */
    uint32_t    cache_size_mb;
    bool        enable_compression;
    char       *compression_algorithm;
    bool        enable_parallel_traversal;
    uint32_t    max_worker_threads;
    
    /* 维护配置 */
    uint32_t    compaction_interval_seconds;
    bool        enable_auto_compaction;
    bool        enable_statistics_collection;
} GraphEngineConfig;

/* 图数据类型 */
typedef enum GraphDataType
{
    GRAPH_DATA_NULL = 0,
    GRAPH_DATA_BOOL,
    GRAPH_DATA_INT32,
    GRAPH_DATA_INT64,
    GRAPH_DATA_FLOAT32,
    GRAPH_DATA_FLOAT64,
    GRAPH_DATA_STRING,
    GRAPH_DATA_BYTES,
    GRAPH_DATA_ARRAY,
    GRAPH_DATA_MAP
} GraphDataType;

/* 图索引类型 */
typedef enum GraphIndexType
{
    GRAPH_INDEX_BTREE = 0,
    GRAPH_INDEX_HASH,
    GRAPH_INDEX_FULLTEXT,
    GRAPH_INDEX_SPATIAL,
    GRAPH_INDEX_COMPOSITE
} GraphIndexType;

/* 图遍历方向 */
typedef enum GraphTraversalDirection
{
    GRAPH_DIRECTION_OUTGOING = 0,
    GRAPH_DIRECTION_INCOMING,
    GRAPH_DIRECTION_BOTH
} GraphTraversalDirection;

/* 图遍历算法 */
typedef enum GraphTraversalAlgorithm
{
    GRAPH_TRAVERSAL_BFS = 0,
    GRAPH_TRAVERSAL_DFS,
    GRAPH_TRAVERSAL_DIJKSTRA,
    GRAPH_TRAVERSAL_A_STAR,
    GRAPH_TRAVERSAL_PAGERANK,
    GRAPH_TRAVERSAL_SHORTEST_PATH
} GraphTraversalAlgorithm;

/* 图属性值 */
typedef struct GraphPropertyValue
{
    GraphDataType   type;
    union
    {
        bool        bool_value;
        int32_t     int32_value;
        int64_t     int64_value;
        float       float32_value;
        double      float64_value;
        char       *string_value;
        struct
        {
            void   *data;
            size_t  size;
        } bytes_value;
        struct
        {
            struct GraphPropertyValue *values;
            uint32_t count;
        } array_value;
        struct
        {
            char   **keys;
            struct GraphPropertyValue *values;
            uint32_t count;
        } map_value;
    };
} GraphPropertyValue;

/* 图属性 */
typedef struct GraphProperty
{
    char                *name;
    GraphPropertyValue   value;
    uint32_t            checksum;
} GraphProperty;

/* 图标签 */
typedef struct GraphLabel
{
    char        *name;
    uint32_t     label_id;
    uint32_t     node_count;
    uint32_t     edge_count;
} GraphLabel;

/* 图节点 */
typedef struct GraphNode
{
    uint64_t        node_id;
    char           *primary_label;
    GraphLabel    **labels;
    uint32_t        num_labels;
    GraphProperty  *properties;
    uint32_t        num_properties;
    uint64_t       *outgoing_edges;
    uint32_t        num_outgoing;
    uint64_t       *incoming_edges;
    uint32_t        num_incoming;
    time_t          created_time;
    time_t          modified_time;
    uint32_t        checksum;
} GraphNode;

/* 图边 */
typedef struct GraphEdge
{
    uint64_t        edge_id;
    uint64_t        source_node_id;
    uint64_t        target_node_id;
    char           *edge_type;
    GraphProperty  *properties;
    uint32_t        num_properties;
    time_t          created_time;
    time_t          modified_time;
    uint32_t        checksum;
} GraphEdge;

/* 图路径 */
typedef struct GraphPath
{
    GraphNode     **nodes;
    GraphEdge     **edges;
    uint32_t        length;
    double          weight;
    char           *path_signature;
} GraphPath;

/* 图遍历结果 */
typedef struct GraphTraversalResult
{
    GraphNode     **nodes;
    uint32_t        num_nodes;
    GraphEdge     **edges;
    uint32_t        num_edges;
    GraphPath     **paths;
    uint32_t        num_paths;
    double          total_weight;
    uint32_t        traversal_depth;
    uint64_t        execution_time_us;
    char           *error_message;
} GraphTraversalResult;

/* 图查询参数扩展 */
typedef struct GraphQueryParamsExt
{
    /* 节点过滤 */
    char          **node_labels;
    uint32_t        num_node_labels;
    GraphProperty  *node_properties;
    uint32_t        num_node_properties;
    
    /* 边过滤 */
    char          **edge_types;
    uint32_t        num_edge_types;
    GraphProperty  *edge_properties;
    uint32_t        num_edge_properties;
    
    /* 遍历参数 */
    GraphTraversalDirection direction;
    GraphTraversalAlgorithm algorithm;
    uint32_t        max_depth;
    uint32_t        max_results;
    
    /* 路径参数 */
    uint64_t        start_node_id;
    uint64_t        end_node_id;
    bool            find_all_paths;
    bool            find_shortest_path;
    
    /* 性能参数 */
    uint32_t        timeout_ms;
    bool            enable_cache;
    bool            enable_parallel;
} GraphQueryParamsExt;

/* 图存储文件 */
typedef struct GraphFile
{
    char           *file_path;
    int             file_descriptor;
    uint32_t        file_version;
    uint64_t        file_size;
    bool            is_open;
    bool            is_readonly;
    time_t          created_time;
    time_t          last_modified;
    uint32_t        checksum;
} GraphFile;

/* 图扫描描述符 */
typedef struct GraphScanDesc
{
    EpiphanySmgrRelation   *relation;
    GraphQueryParamsExt     params;
    GraphNode             **result_buffer;
    uint32_t                buffer_size;
    uint32_t                buffer_pos;
    bool                    is_finished;
    void                   *scan_state;
} GraphScanDesc;

/* 图插入状态 */
typedef struct GraphInsertState
{
    EpiphanySmgrRelation   *relation;
    GraphNode             **node_buffer;
    GraphEdge             **edge_buffer;
    uint32_t                max_batch_size;
    uint32_t                node_batch_pos;
    uint32_t                edge_batch_pos;
    bool                    auto_flush;
    bool                    auto_create_labels;
    uint64_t                inserted_nodes;
    uint64_t                inserted_edges;
    time_t                  last_flush_time;
} GraphInsertState;

/* 图统计信息 */
typedef struct GraphStats
{
    uint64_t        total_nodes;
    uint64_t        total_edges;
    uint64_t        total_labels;
    uint64_t        total_properties;
    uint64_t        insert_operations;
    uint64_t        query_operations;
    uint64_t        traversal_operations;
    uint64_t        index_operations;
    double          avg_query_time_ms;
    double          avg_traversal_time_ms;
    uint64_t        cache_hits;
    uint64_t        cache_misses;
    uint64_t        disk_reads;
    uint64_t        disk_writes;
    time_t          last_updated;
    time_t          start_time;
} GraphStats;

/* 图索引 */
typedef struct GraphIndex
{
    char               *index_name;
    GraphIndexType      index_type;
    char              **indexed_properties;
    uint32_t            num_properties;
    bool                is_unique;
    bool                is_composite;
    void               *index_data;
    uint64_t            index_size;
    time_t              created_time;
    time_t              last_updated;
} GraphIndex;

/* 函数声明 */

/* 图存储引擎初始化 */
void graph_engine_init(void);
void graph_engine_shutdown(void);
void graph_engine_configure(const char *config_json);

/* PostgreSQL 兼容的存储管理器接口 */
void graph_smgr_init(void);
void graph_smgr_shutdown(void);
void graph_smgr_open(SMgrRelation reln);
void graph_smgr_close(SMgrRelation reln, ForkNumber forknum);
void graph_smgr_create(SMgrRelation reln, ForkNumber forknum, bool isRedo);
bool graph_smgr_exists(SMgrRelation reln, ForkNumber forknum);
void graph_smgr_unlink(RelFileLocatorBackend rlocator, ForkNumber forknum, bool isRedo);
void graph_smgr_extend(SMgrRelation reln, ForkNumber forknum, BlockNumber blocknum, 
                      const void *buffer, bool skipFsync);
void graph_smgr_zeroextend(SMgrRelation reln, ForkNumber forknum, BlockNumber blocknum, 
                          int nblocks, bool skipFsync);
bool graph_smgr_prefetch(SMgrRelation reln, ForkNumber forknum, BlockNumber blocknum, 
                        int nblocks);
void graph_smgr_readv(SMgrRelation reln, ForkNumber forknum, BlockNumber blocknum,
                     void **buffers, BlockNumber nblocks);
void graph_smgr_writev(SMgrRelation reln, ForkNumber forknum, BlockNumber blocknum,
                      const void **buffers, BlockNumber nblocks, bool skipFsync);
void graph_smgr_writeback(SMgrRelation reln, ForkNumber forknum, BlockNumber blocknum, 
                         BlockNumber nblocks);
BlockNumber graph_smgr_nblocks(SMgrRelation reln, ForkNumber forknum);
void graph_smgr_truncate(SMgrRelation reln, ForkNumber forknum, BlockNumber nblocks);
void graph_smgr_immedsync(SMgrRelation reln, ForkNumber forknum);
void graph_smgr_registersync(SMgrRelation reln, ForkNumber forknum);

/* 图文件操作 */
GraphFile* graph_create_file(const char *file_path);
GraphFile* graph_open_file(const char *file_path, bool readonly);
void graph_close_file(GraphFile *file);
void graph_flush_file(GraphFile *file);
void graph_compact_file(GraphFile *file);

/* 图节点操作 */
GraphNode* graph_create_node(const char *primary_label, GraphProperty *properties, uint32_t num_properties);
GraphNode* graph_get_node(EpiphanySmgrRelation *relation, uint64_t node_id);
bool graph_update_node(EpiphanySmgrRelation *relation, GraphNode *node);
bool graph_delete_node(EpiphanySmgrRelation *relation, uint64_t node_id);
GraphNode** graph_find_nodes_by_label(EpiphanySmgrRelation *relation, const char *label, uint32_t *num_nodes);
GraphNode** graph_find_nodes_by_property(EpiphanySmgrRelation *relation, const char *property_name, 
                                         const GraphPropertyValue *value, uint32_t *num_nodes);

/* 图边操作 */
GraphEdge* graph_create_edge(uint64_t source_node_id, uint64_t target_node_id, const char *edge_type,
                            GraphProperty *properties, uint32_t num_properties);
GraphEdge* graph_get_edge(EpiphanySmgrRelation *relation, uint64_t edge_id);
bool graph_update_edge(EpiphanySmgrRelation *relation, GraphEdge *edge);
bool graph_delete_edge(EpiphanySmgrRelation *relation, uint64_t edge_id);
GraphEdge** graph_get_outgoing_edges(EpiphanySmgrRelation *relation, uint64_t node_id, uint32_t *num_edges);
GraphEdge** graph_get_incoming_edges(EpiphanySmgrRelation *relation, uint64_t node_id, uint32_t *num_edges);
GraphEdge** graph_find_edges_by_type(EpiphanySmgrRelation *relation, const char *edge_type, uint32_t *num_edges);

/* 图遍历操作 */
GraphScanDesc* graph_begin_scan(EpiphanySmgrRelation *relation, const GraphQueryParamsExt *params);
GraphNode* graph_scan_next(GraphScanDesc *scan);
void graph_end_scan(GraphScanDesc *scan);
GraphTraversalResult* graph_traverse(EpiphanySmgrRelation *relation, const GraphQueryParamsExt *params);
GraphPath** graph_find_paths(EpiphanySmgrRelation *relation, uint64_t start_node_id, uint64_t end_node_id,
                             uint32_t max_depth, uint32_t *num_paths);
GraphPath* graph_find_shortest_path(EpiphanySmgrRelation *relation, uint64_t start_node_id, uint64_t end_node_id);
void graph_free_traversal_result(GraphTraversalResult *result);

/* 图插入操作 */
GraphInsertState* graph_begin_insert(EpiphanySmgrRelation *relation);
bool graph_insert_node(GraphInsertState *state, const GraphNode *node);
bool graph_insert_edge(GraphInsertState *state, const GraphEdge *edge);
bool graph_insert_batch_nodes(GraphInsertState *state, const GraphNode **nodes, uint32_t num_nodes);
bool graph_insert_batch_edges(GraphInsertState *state, const GraphEdge **edges, uint32_t num_edges);
void graph_flush_insert(GraphInsertState *state);
void graph_end_insert(GraphInsertState *state);

/* 图属性操作 */
bool graph_add_node_property(EpiphanySmgrRelation *relation, uint64_t node_id, const GraphProperty *property);
bool graph_update_node_property(EpiphanySmgrRelation *relation, uint64_t node_id, const GraphProperty *property);
bool graph_remove_node_property(EpiphanySmgrRelation *relation, uint64_t node_id, const char *property_name);
GraphProperty* graph_get_node_property(EpiphanySmgrRelation *relation, uint64_t node_id, const char *property_name);
bool graph_add_edge_property(EpiphanySmgrRelation *relation, uint64_t edge_id, const GraphProperty *property);
bool graph_update_edge_property(EpiphanySmgrRelation *relation, uint64_t edge_id, const GraphProperty *property);
bool graph_remove_edge_property(EpiphanySmgrRelation *relation, uint64_t edge_id, const char *property_name);
GraphProperty* graph_get_edge_property(EpiphanySmgrRelation *relation, uint64_t edge_id, const char *property_name);

/* 图标签操作 */
bool graph_create_label(EpiphanySmgrRelation *relation, const GraphLabel *label);
GraphLabel* graph_get_label(EpiphanySmgrRelation *relation, const char *label_name);
bool graph_add_node_label(EpiphanySmgrRelation *relation, uint64_t node_id, const char *label_name);
bool graph_remove_node_label(EpiphanySmgrRelation *relation, uint64_t node_id, const char *label_name);
GraphLabel** graph_list_labels(EpiphanySmgrRelation *relation, uint32_t *num_labels);

/* 图索引操作 */
bool graph_create_node_index(EpiphanySmgrRelation *relation, const char *index_name, const char *property_name, 
                            GraphIndexType index_type, bool is_unique);
bool graph_create_edge_index(EpiphanySmgrRelation *relation, const char *index_name, const char *property_name, 
                            GraphIndexType index_type, bool is_unique);
bool graph_create_composite_index(EpiphanySmgrRelation *relation, const char *index_name, 
                                 const char **property_names, uint32_t num_properties, 
                                 GraphIndexType index_type, bool is_unique);
bool graph_drop_index(EpiphanySmgrRelation *relation, const char *index_name);
void graph_rebuild_indexes(EpiphanySmgrRelation *relation);
GraphIndex** graph_list_indexes(EpiphanySmgrRelation *relation, uint32_t *num_indexes);

/* 图算法 */
double* graph_pagerank(EpiphanySmgrRelation *relation, uint32_t max_iterations, double damping_factor);
uint64_t** graph_connected_components(EpiphanySmgrRelation *relation, uint32_t *num_components);
GraphNode** graph_community_detection(EpiphanySmgrRelation *relation, const char *algorithm, uint32_t *num_communities);
double graph_clustering_coefficient(EpiphanySmgrRelation *relation, uint64_t node_id);
uint32_t graph_node_degree(EpiphanySmgrRelation *relation, uint64_t node_id, GraphTraversalDirection direction);

/* 图数据维护 */
void graph_compact_data(EpiphanySmgrRelation *relation);
void graph_vacuum_deleted_data(EpiphanySmgrRelation *relation);
void graph_rebalance_storage(EpiphanySmgrRelation *relation);
void graph_update_statistics(EpiphanySmgrRelation *relation);

/* 统计信息和优化 */
GraphStats* graph_collect_stats(EpiphanySmgrRelation *relation);
void graph_reset_stats(EpiphanySmgrRelation *relation);
void graph_optimize_storage(EpiphanySmgrRelation *relation);
void graph_analyze_workload(EpiphanySmgrRelation *relation);

/* 事务支持 */
void graph_begin_transaction(EpiphanySmgrRelation *relation);
void graph_commit_transaction(EpiphanySmgrRelation *relation);
void graph_abort_transaction(EpiphanySmgrRelation *relation);
void graph_prepare_transaction(EpiphanySmgrRelation *relation, const char *gid);

/* 备份和恢复 */
void graph_backup_relation(EpiphanySmgrRelation *relation, const char *backup_path);
void graph_restore_relation(EpiphanySmgrRelation *relation, const char *backup_path);
void graph_export_data(EpiphanySmgrRelation *relation, const char *export_path, const GraphQueryParamsExt *params);
void graph_import_data(EpiphanySmgrRelation *relation, const char *import_path);

/* 监控和诊断 */
void graph_get_health_status(EpiphanySmgrRelation *relation, char **status_json);
void graph_get_performance_metrics(EpiphanySmgrRelation *relation, char **metrics_json);
void graph_diagnose_performance(EpiphanySmgrRelation *relation, char **diagnosis_json);

/* 配置管理 */
GraphEngineConfig* graph_get_default_config(void);
void graph_set_config(GraphEngineConfig *config);
GraphEngineConfig* graph_get_config(void);

/* 工具函数 */
EpiphanyStorageEngine* create_graph_storage_engine(void);
void register_graph_storage_engine(void);
const char* graph_data_type_name(GraphDataType type);
const char* graph_index_type_name(GraphIndexType type);
const char* graph_traversal_algorithm_name(GraphTraversalAlgorithm algorithm);
size_t graph_data_type_size(GraphDataType type);
uint32_t graph_calculate_node_checksum(const GraphNode *node);
uint32_t graph_calculate_edge_checksum(const GraphEdge *edge);
bool graph_validate_node(const GraphNode *node);
bool graph_validate_edge(const GraphEdge *edge);
GraphPropertyValue* graph_parse_property_value(const char *value_str, GraphDataType type);
char* graph_format_property_value(const GraphPropertyValue *value);
void graph_free_property_value(GraphPropertyValue *value);
void graph_free_node(GraphNode *node);
void graph_free_edge(GraphEdge *edge);
void graph_free_path(GraphPath *path);

#endif /* GRAPH_STORAGE_H */