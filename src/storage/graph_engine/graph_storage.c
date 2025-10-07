/*-------------------------------------------------------------------------
 *
 * graph_storage.c
 *    EpiphanyDB 图存储引擎实现 (Neo4j 兼容)
 *
 * 基于图数据库设计，提供高效的图数据存储和遍历，
 * 支持节点、边、属性、索引和图算法。
 *
 * Copyright (c) 2024, EpiphanyDB Team
 *
 * IDENTIFICATION
 *    src/storage/graph_engine/graph_storage.c
 *
 *-------------------------------------------------------------------------
 */

#include "graph_storage.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

/* 全局变量 */
static GraphEngineConfig *g_graph_config = NULL;
static GraphStats *g_graph_stats = NULL;
static bool g_graph_initialized = false;

/* 内部函数声明 */
static void init_graph_config(void);
static void init_graph_stats(void);
static uint32_t calculate_node_checksum(const GraphNode *node);
static uint32_t calculate_edge_checksum(const GraphEdge *edge);
static bool validate_graph_file(GraphFile *file);
static void write_file_header(GraphFile *file);
static void read_file_header(GraphFile *file);
static GraphNode* create_graph_node(uint64_t node_id);
static GraphEdge* create_graph_edge(uint64_t edge_id, uint64_t source_id, uint64_t target_id);
static void free_graph_node(GraphNode *node);
static void free_graph_edge(GraphEdge *edge);

/*
 * 图存储引擎初始化
 */
void
graph_engine_init(void)
{
    if (g_graph_initialized)
        return;

    /* 初始化配置 */
    init_graph_config();
    
    /* 初始化统计信息 */
    init_graph_stats();
    
    /* 注册存储引擎 */
    register_graph_storage_engine();
    
    g_graph_initialized = true;
    
    printf("EpiphanyDB: 图存储引擎已初始化\n");
}

/*
 * 图存储引擎关闭
 */
void
graph_engine_shutdown(void)
{
    if (!g_graph_initialized)
        return;

    /* 清理配置 */
    if (g_graph_config)
    {
        if (g_graph_config->default_index_type)
            free(g_graph_config->default_index_type);
        if (g_graph_config->compression_algorithm)
            free(g_graph_config->compression_algorithm);
        free(g_graph_config);
        g_graph_config = NULL;
    }
    
    /* 清理统计信息 */
    if (g_graph_stats)
    {
        free(g_graph_stats);
        g_graph_stats = NULL;
    }
    
    g_graph_initialized = false;
    
    printf("EpiphanyDB: 图存储引擎已关闭\n");
}

/*
 * 配置图存储引擎
 */
void
graph_engine_configure(const char *config_json)
{
    if (!config_json)
        return;
        
    /* TODO: 解析 JSON 配置并更新 g_graph_config */
    printf("EpiphanyDB: 图存储引擎配置已更新\n");
}

/*
 * PostgreSQL 兼容的存储管理器接口实现
 */

void
graph_smgr_init(void)
{
    graph_engine_init();
}

void
graph_smgr_shutdown(void)
{
    graph_engine_shutdown();
}

void
graph_smgr_open(SMgrRelation reln)
{
    if (!reln)
        return;
        
    /* TODO: 打开图存储文件 */
    printf("EpiphanyDB: 打开图存储关系\n");
}

void
graph_smgr_close(SMgrRelation reln, ForkNumber forknum)
{
    if (!reln)
        return;
        
    /* TODO: 关闭图存储文件 */
    printf("EpiphanyDB: 关闭图存储关系\n");
}

void
graph_smgr_create(SMgrRelation reln, ForkNumber forknum, bool isRedo)
{
    if (!reln)
        return;
        
    /* TODO: 创建图存储文件 */
    printf("EpiphanyDB: 创建图存储关系\n");
}

bool
graph_smgr_exists(SMgrRelation reln, ForkNumber forknum)
{
    if (!reln)
        return false;
        
    /* TODO: 检查图存储文件是否存在 */
    return true;
}

void
graph_smgr_unlink(RelFileLocatorBackend rlocator, ForkNumber forknum, bool isRedo)
{
    /* TODO: 删除图存储文件 */
    printf("EpiphanyDB: 删除图存储文件\n");
}

void
graph_smgr_extend(SMgrRelation reln, ForkNumber forknum, BlockNumber blocknum, 
                 const void *buffer, bool skipFsync)
{
    if (!reln || !buffer)
        return;
        
    /* TODO: 扩展图存储文件 */
    printf("EpiphanyDB: 扩展图存储文件\n");
}

void
graph_smgr_zeroextend(SMgrRelation reln, ForkNumber forknum, BlockNumber blocknum, 
                     int nblocks, bool skipFsync)
{
    if (!reln)
        return;
        
    /* TODO: 零扩展图存储文件 */
    printf("EpiphanyDB: 零扩展图存储文件\n");
}

bool
graph_smgr_prefetch(SMgrRelation reln, ForkNumber forknum, BlockNumber blocknum, 
                   int nblocks)
{
    if (!reln)
        return false;
        
    /* TODO: 预取图存储数据 */
    return true;
}

void
graph_smgr_readv(SMgrRelation reln, ForkNumber forknum, BlockNumber blocknum,
                void **buffers, BlockNumber nblocks)
{
    if (!reln || !buffers)
        return;
        
    /* TODO: 向量化读取图存储数据 */
    printf("EpiphanyDB: 向量化读取图存储数据\n");
}

void
graph_smgr_writev(SMgrRelation reln, ForkNumber forknum, BlockNumber blocknum,
                 const void **buffers, BlockNumber nblocks, bool skipFsync)
{
    if (!reln || !buffers)
        return;
        
    /* TODO: 向量化写入图存储数据 */
    printf("EpiphanyDB: 向量化写入图存储数据\n");
}

void
graph_smgr_writeback(SMgrRelation reln, ForkNumber forknum, BlockNumber blocknum, 
                    BlockNumber nblocks)
{
    if (!reln)
        return;
        
    /* TODO: 写回图存储数据 */
    printf("EpiphanyDB: 写回图存储数据\n");
}

BlockNumber
graph_smgr_nblocks(SMgrRelation reln, ForkNumber forknum)
{
    if (!reln)
        return 0;
        
    /* TODO: 返回图存储文件块数 */
    return 1000; /* 示例值 */
}

void
graph_smgr_truncate(SMgrRelation reln, ForkNumber forknum, BlockNumber nblocks)
{
    if (!reln)
        return;
        
    /* TODO: 截断图存储文件 */
    printf("EpiphanyDB: 截断图存储文件\n");
}

void
graph_smgr_immedsync(SMgrRelation reln, ForkNumber forknum)
{
    if (!reln)
        return;
        
    /* TODO: 立即同步图存储文件 */
    printf("EpiphanyDB: 立即同步图存储文件\n");
}

void
graph_smgr_registersync(SMgrRelation reln, ForkNumber forknum)
{
    if (!reln)
        return;
        
    /* TODO: 注册同步图存储文件 */
    printf("EpiphanyDB: 注册同步图存储文件\n");
}

/*
 * 图文件操作
 */

GraphFile*
graph_create_file(const char *file_path)
{
    if (!file_path)
        return NULL;
        
    GraphFile *file = (GraphFile*)malloc(sizeof(GraphFile));
    if (!file)
        return NULL;
        
    memset(file, 0, sizeof(GraphFile));
    
    file->file_path = strdup(file_path);
    file->file_descriptor = open(file_path, O_CREAT | O_RDWR, 0644);
    file->file_version = GRAPH_FILE_VERSION;
    file->is_open = true;
    file->is_readonly = false;
    file->created_time = time(NULL);
    
    /* 写入文件头 */
    write_file_header(file);
    
    return file;
}

GraphFile*
graph_open_file(const char *file_path, bool readonly)
{
    if (!file_path)
        return NULL;
        
    GraphFile *file = (GraphFile*)malloc(sizeof(GraphFile));
    if (!file)
        return NULL;
        
    memset(file, 0, sizeof(GraphFile));
    
    file->file_path = strdup(file_path);
    file->file_descriptor = open(file_path, readonly ? O_RDONLY : O_RDWR);
    file->is_open = true;
    file->is_readonly = readonly;
    
    if (file->file_descriptor < 0)
    {
        free(file->file_path);
        free(file);
        return NULL;
    }
    
    /* 读取文件头 */
    read_file_header(file);
    
    /* 验证文件格式 */
    if (!validate_graph_file(file))
    {
        graph_close_file(file);
        return NULL;
    }
    
    return file;
}

void
graph_close_file(GraphFile *file)
{
    if (!file)
        return;
        
    if (file->file_descriptor >= 0)
    {
        close(file->file_descriptor);
        file->file_descriptor = -1;
    }
    
    if (file->file_path)
    {
        free(file->file_path);
        file->file_path = NULL;
    }
    
    file->is_open = false;
    free(file);
}

void
graph_flush_file(GraphFile *file)
{
    if (!file || file->file_descriptor < 0)
        return;
        
    fsync(file->file_descriptor);
    file->last_modified = time(NULL);
}

void
graph_compact_file(GraphFile *file)
{
    if (!file)
        return;
        
    /* TODO: 压缩图文件 */
    printf("EpiphanyDB: 压缩图文件\n");
}

/*
 * 图节点操作
 */

GraphNode*
graph_create_node(const char *primary_label, GraphProperty *properties, uint32_t num_properties)
{
    if (!primary_label)
        return NULL;
        
    GraphNode *node = create_graph_node(0); /* ID 将在插入时分配 */
    if (!node)
        return NULL;
        
    node->primary_label = strdup(primary_label);
    
    if (properties && num_properties > 0)
    {
        node->properties = (GraphProperty*)malloc(sizeof(GraphProperty) * num_properties);
        if (node->properties)
        {
            memcpy(node->properties, properties, sizeof(GraphProperty) * num_properties);
            node->num_properties = num_properties;
        }
    }
    
    node->created_time = time(NULL);
    node->modified_time = node->created_time;
    node->checksum = calculate_node_checksum(node);
    
    return node;
}

GraphNode*
graph_get_node(EpiphanySmgrRelation *relation, uint64_t node_id)
{
    if (!relation || node_id == 0)
        return NULL;
        
    /* TODO: 从存储中获取节点 */
    
    if (g_graph_stats)
        g_graph_stats->query_operations++;
        
    return NULL;
}

bool
graph_update_node(EpiphanySmgrRelation *relation, GraphNode *node)
{
    if (!relation || !node)
        return false;
        
    /* TODO: 更新存储中的节点 */
    
    node->modified_time = time(NULL);
    node->checksum = calculate_node_checksum(node);
    
    return true;
}

bool
graph_delete_node(EpiphanySmgrRelation *relation, uint64_t node_id)
{
    if (!relation || node_id == 0)
        return false;
        
    /* TODO: 从存储中删除节点 */
    
    if (g_graph_stats)
        g_graph_stats->total_nodes--;
        
    return true;
}

GraphNode**
graph_find_nodes_by_label(EpiphanySmgrRelation *relation, const char *label, uint32_t *num_nodes)
{
    if (!relation || !label || !num_nodes)
        return NULL;
        
    /* TODO: 根据标签查找节点 */
    
    *num_nodes = 0;
    return NULL;
}

GraphNode**
graph_find_nodes_by_property(EpiphanySmgrRelation *relation, const char *property_name, 
                             const GraphPropertyValue *value, uint32_t *num_nodes)
{
    if (!relation || !property_name || !value || !num_nodes)
        return NULL;
        
    /* TODO: 根据属性查找节点 */
    
    *num_nodes = 0;
    return NULL;
}

/*
 * 图边操作
 */

GraphEdge*
graph_create_edge(uint64_t source_node_id, uint64_t target_node_id, const char *edge_type,
                 GraphProperty *properties, uint32_t num_properties)
{
    if (source_node_id == 0 || target_node_id == 0 || !edge_type)
        return NULL;
        
    GraphEdge *edge = create_graph_edge(0, source_node_id, target_node_id); /* ID 将在插入时分配 */
    if (!edge)
        return NULL;
        
    edge->edge_type = strdup(edge_type);
    
    if (properties && num_properties > 0)
    {
        edge->properties = (GraphProperty*)malloc(sizeof(GraphProperty) * num_properties);
        if (edge->properties)
        {
            memcpy(edge->properties, properties, sizeof(GraphProperty) * num_properties);
            edge->num_properties = num_properties;
        }
    }
    
    edge->created_time = time(NULL);
    edge->modified_time = edge->created_time;
    edge->checksum = calculate_edge_checksum(edge);
    
    return edge;
}

GraphEdge*
graph_get_edge(EpiphanySmgrRelation *relation, uint64_t edge_id)
{
    if (!relation || edge_id == 0)
        return NULL;
        
    /* TODO: 从存储中获取边 */
    
    if (g_graph_stats)
        g_graph_stats->query_operations++;
        
    return NULL;
}

bool
graph_update_edge(EpiphanySmgrRelation *relation, GraphEdge *edge)
{
    if (!relation || !edge)
        return false;
        
    /* TODO: 更新存储中的边 */
    
    edge->modified_time = time(NULL);
    edge->checksum = calculate_edge_checksum(edge);
    
    return true;
}

bool
graph_delete_edge(EpiphanySmgrRelation *relation, uint64_t edge_id)
{
    if (!relation || edge_id == 0)
        return false;
        
    /* TODO: 从存储中删除边 */
    
    if (g_graph_stats)
        g_graph_stats->total_edges--;
        
    return true;
}

GraphEdge**
graph_get_outgoing_edges(EpiphanySmgrRelation *relation, uint64_t node_id, uint32_t *num_edges)
{
    if (!relation || node_id == 0 || !num_edges)
        return NULL;
        
    /* TODO: 获取节点的出边 */
    
    *num_edges = 0;
    return NULL;
}

GraphEdge**
graph_get_incoming_edges(EpiphanySmgrRelation *relation, uint64_t node_id, uint32_t *num_edges)
{
    if (!relation || node_id == 0 || !num_edges)
        return NULL;
        
    /* TODO: 获取节点的入边 */
    
    *num_edges = 0;
    return NULL;
}

GraphEdge**
graph_find_edges_by_type(EpiphanySmgrRelation *relation, const char *edge_type, uint32_t *num_edges)
{
    if (!relation || !edge_type || !num_edges)
        return NULL;
        
    /* TODO: 根据类型查找边 */
    
    *num_edges = 0;
    return NULL;
}

/*
 * 图遍历操作
 */

GraphScanDesc*
graph_begin_scan(EpiphanySmgrRelation *relation, const GraphQueryParamsExt *params)
{
    if (!relation || !params)
        return NULL;
        
    GraphScanDesc *scan = (GraphScanDesc*)malloc(sizeof(GraphScanDesc));
    if (!scan)
        return NULL;
        
    memset(scan, 0, sizeof(GraphScanDesc));
    
    scan->relation = relation;
    scan->params = *params;
    scan->is_finished = false;
    scan->buffer_size = g_graph_config ? g_graph_config->max_result_nodes : GRAPH_DEFAULT_BATCH_SIZE;
    
    /* 分配结果缓存 */
    scan->result_buffer = (GraphNode**)calloc(scan->buffer_size, sizeof(GraphNode*));
    if (!scan->result_buffer)
    {
        free(scan);
        return NULL;
    }
    
    /* TODO: 执行查询并填充结果缓存 */
    
    if (g_graph_stats)
        g_graph_stats->query_operations++;
        
    return scan;
}

GraphNode*
graph_scan_next(GraphScanDesc *scan)
{
    if (!scan || scan->is_finished)
        return NULL;
        
    if (scan->buffer_pos >= scan->buffer_size)
    {
        /* TODO: 加载下一批数据 */
        scan->is_finished = true;
        return NULL;
    }
    
    return scan->result_buffer[scan->buffer_pos++];
}

void
graph_end_scan(GraphScanDesc *scan)
{
    if (!scan)
        return;
        
    if (scan->result_buffer)
    {
        /* 释放结果缓存中的节点 */
        for (uint32_t i = 0; i < scan->buffer_size; i++)
        {
            if (scan->result_buffer[i])
                free_graph_node(scan->result_buffer[i]);
        }
        free(scan->result_buffer);
        scan->result_buffer = NULL;
    }
    
    free(scan);
}

GraphTraversalResult*
graph_traverse(EpiphanySmgrRelation *relation, const GraphQueryParamsExt *params)
{
    if (!relation || !params)
        return NULL;
        
    GraphTraversalResult *result = (GraphTraversalResult*)malloc(sizeof(GraphTraversalResult));
    if (!result)
        return NULL;
        
    memset(result, 0, sizeof(GraphTraversalResult));
    
    /* TODO: 执行图遍历 */
    
    if (g_graph_stats)
        g_graph_stats->traversal_operations++;
        
    return result;
}

GraphPath**
graph_find_paths(EpiphanySmgrRelation *relation, uint64_t start_node_id, uint64_t end_node_id,
                uint32_t max_depth, uint32_t *num_paths)
{
    if (!relation || start_node_id == 0 || end_node_id == 0 || !num_paths)
        return NULL;
        
    /* TODO: 查找路径 */
    
    *num_paths = 0;
    return NULL;
}

GraphPath*
graph_find_shortest_path(EpiphanySmgrRelation *relation, uint64_t start_node_id, uint64_t end_node_id)
{
    if (!relation || start_node_id == 0 || end_node_id == 0)
        return NULL;
        
    /* TODO: 查找最短路径 */
    
    return NULL;
}

void
graph_free_traversal_result(GraphTraversalResult *result)
{
    if (!result)
        return;
        
    /* 释放节点数组 */
    if (result->nodes)
    {
        for (uint32_t i = 0; i < result->num_nodes; i++)
        {
            if (result->nodes[i])
                free_graph_node(result->nodes[i]);
        }
        free(result->nodes);
    }
    
    /* 释放边数组 */
    if (result->edges)
    {
        for (uint32_t i = 0; i < result->num_edges; i++)
        {
            if (result->edges[i])
                free_graph_edge(result->edges[i]);
        }
        free(result->edges);
    }
    
    /* 释放路径数组 */
    if (result->paths)
    {
        for (uint32_t i = 0; i < result->num_paths; i++)
        {
            if (result->paths[i])
                graph_free_path(result->paths[i]);
        }
        free(result->paths);
    }
    
    if (result->error_message)
        free(result->error_message);
        
    free(result);
}

/*
 * 图插入操作
 */

GraphInsertState*
graph_begin_insert(EpiphanySmgrRelation *relation)
{
    if (!relation)
        return NULL;
        
    GraphInsertState *state = (GraphInsertState*)malloc(sizeof(GraphInsertState));
    if (!state)
        return NULL;
        
    memset(state, 0, sizeof(GraphInsertState));
    
    state->relation = relation;
    state->max_batch_size = g_graph_config ? g_graph_config->max_nodes_per_block : GRAPH_DEFAULT_BATCH_SIZE;
    state->auto_flush = true;
    state->auto_create_labels = true;
    
    /* 分配批次缓存 */
    state->node_buffer = (GraphNode**)calloc(state->max_batch_size, sizeof(GraphNode*));
    state->edge_buffer = (GraphEdge**)calloc(state->max_batch_size, sizeof(GraphEdge*));
    
    if (!state->node_buffer || !state->edge_buffer)
    {
        if (state->node_buffer) free(state->node_buffer);
        if (state->edge_buffer) free(state->edge_buffer);
        free(state);
        return NULL;
    }
    
    if (g_graph_stats)
        g_graph_stats->insert_operations++;
        
    return state;
}

bool
graph_insert_node(GraphInsertState *state, const GraphNode *node)
{
    if (!state || !node)
        return false;
        
    /* 验证节点 */
    if (!graph_validate_node(node))
        return false;
        
    /* 检查批次是否已满 */
    if (state->node_batch_pos >= state->max_batch_size)
    {
        if (state->auto_flush)
            graph_flush_insert(state);
        else
            return false;
    }
    
    /* 复制节点到批次缓存 */
    state->node_buffer[state->node_batch_pos] = (GraphNode*)malloc(sizeof(GraphNode));
    if (!state->node_buffer[state->node_batch_pos])
        return false;
        
    memcpy(state->node_buffer[state->node_batch_pos], node, sizeof(GraphNode));
    state->node_batch_pos++;
    
    return true;
}

bool
graph_insert_edge(GraphInsertState *state, const GraphEdge *edge)
{
    if (!state || !edge)
        return false;
        
    /* 验证边 */
    if (!graph_validate_edge(edge))
        return false;
        
    /* 检查批次是否已满 */
    if (state->edge_batch_pos >= state->max_batch_size)
    {
        if (state->auto_flush)
            graph_flush_insert(state);
        else
            return false;
    }
    
    /* 复制边到批次缓存 */
    state->edge_buffer[state->edge_batch_pos] = (GraphEdge*)malloc(sizeof(GraphEdge));
    if (!state->edge_buffer[state->edge_batch_pos])
        return false;
        
    memcpy(state->edge_buffer[state->edge_batch_pos], edge, sizeof(GraphEdge));
    state->edge_batch_pos++;
    
    return true;
}

bool
graph_insert_batch_nodes(GraphInsertState *state, const GraphNode **nodes, uint32_t num_nodes)
{
    if (!state || !nodes || num_nodes == 0)
        return false;
        
    for (uint32_t i = 0; i < num_nodes; i++)
    {
        if (!graph_insert_node(state, nodes[i]))
            return false;
    }
    
    return true;
}

bool
graph_insert_batch_edges(GraphInsertState *state, const GraphEdge **edges, uint32_t num_edges)
{
    if (!state || !edges || num_edges == 0)
        return false;
        
    for (uint32_t i = 0; i < num_edges; i++)
    {
        if (!graph_insert_edge(state, edges[i]))
            return false;
    }
    
    return true;
}

void
graph_flush_insert(GraphInsertState *state)
{
    if (!state)
        return;
        
    /* TODO: 将批次数据写入存储 */
    
    state->inserted_nodes += state->node_batch_pos;
    state->inserted_edges += state->edge_batch_pos;
    
    /* 清理批次缓存 */
    for (uint32_t i = 0; i < state->node_batch_pos; i++)
    {
        if (state->node_buffer[i])
        {
            free(state->node_buffer[i]);
            state->node_buffer[i] = NULL;
        }
    }
    
    for (uint32_t i = 0; i < state->edge_batch_pos; i++)
    {
        if (state->edge_buffer[i])
        {
            free(state->edge_buffer[i]);
            state->edge_buffer[i] = NULL;
        }
    }
    
    state->node_batch_pos = 0;
    state->edge_batch_pos = 0;
    state->last_flush_time = time(NULL);
    
    printf("EpiphanyDB: 刷新图数据批次\n");
}

void
graph_end_insert(GraphInsertState *state)
{
    if (!state)
        return;
        
    /* 刷新剩余数据 */
    if (state->node_batch_pos > 0 || state->edge_batch_pos > 0)
        graph_flush_insert(state);
        
    /* 清理批次缓存 */
    if (state->node_buffer)
    {
        free(state->node_buffer);
        state->node_buffer = NULL;
    }
    
    if (state->edge_buffer)
    {
        free(state->edge_buffer);
        state->edge_buffer = NULL;
    }
    
    /* 更新统计信息 */
    if (g_graph_stats)
    {
        g_graph_stats->total_nodes += state->inserted_nodes;
        g_graph_stats->total_edges += state->inserted_edges;
    }
    
    free(state);
}

/*
 * 图属性操作
 */

bool
graph_add_node_property(EpiphanySmgrRelation *relation, uint64_t node_id, const GraphProperty *property)
{
    if (!relation || node_id == 0 || !property)
        return false;
        
    /* TODO: 添加节点属性 */
    
    return true;
}

bool
graph_update_node_property(EpiphanySmgrRelation *relation, uint64_t node_id, const GraphProperty *property)
{
    if (!relation || node_id == 0 || !property)
        return false;
        
    /* TODO: 更新节点属性 */
    
    return true;
}

bool
graph_remove_node_property(EpiphanySmgrRelation *relation, uint64_t node_id, const char *property_name)
{
    if (!relation || node_id == 0 || !property_name)
        return false;
        
    /* TODO: 删除节点属性 */
    
    return true;
}

GraphProperty*
graph_get_node_property(EpiphanySmgrRelation *relation, uint64_t node_id, const char *property_name)
{
    if (!relation || node_id == 0 || !property_name)
        return NULL;
        
    /* TODO: 获取节点属性 */
    
    return NULL;
}

bool
graph_add_edge_property(EpiphanySmgrRelation *relation, uint64_t edge_id, const GraphProperty *property)
{
    if (!relation || edge_id == 0 || !property)
        return false;
        
    /* TODO: 添加边属性 */
    
    return true;
}

bool
graph_update_edge_property(EpiphanySmgrRelation *relation, uint64_t edge_id, const GraphProperty *property)
{
    if (!relation || edge_id == 0 || !property)
        return false;
        
    /* TODO: 更新边属性 */
    
    return true;
}

bool
graph_remove_edge_property(EpiphanySmgrRelation *relation, uint64_t edge_id, const char *property_name)
{
    if (!relation || edge_id == 0 || !property_name)
        return false;
        
    /* TODO: 删除边属性 */
    
    return true;
}

GraphProperty*
graph_get_edge_property(EpiphanySmgrRelation *relation, uint64_t edge_id, const char *property_name)
{
    if (!relation || edge_id == 0 || !property_name)
        return NULL;
        
    /* TODO: 获取边属性 */
    
    return NULL;
}

/*
 * 图标签操作
 */

bool
graph_create_label(EpiphanySmgrRelation *relation, const GraphLabel *label)
{
    if (!relation || !label)
        return false;
        
    /* TODO: 创建标签 */
    
    if (g_graph_stats)
        g_graph_stats->total_labels++;
        
    return true;
}

GraphLabel*
graph_get_label(EpiphanySmgrRelation *relation, const char *label_name)
{
    if (!relation || !label_name)
        return NULL;
        
    /* TODO: 获取标签 */
    
    return NULL;
}

bool
graph_add_node_label(EpiphanySmgrRelation *relation, uint64_t node_id, const char *label_name)
{
    if (!relation || node_id == 0 || !label_name)
        return false;
        
    /* TODO: 为节点添加标签 */
    
    return true;
}

bool
graph_remove_node_label(EpiphanySmgrRelation *relation, uint64_t node_id, const char *label_name)
{
    if (!relation || node_id == 0 || !label_name)
        return false;
        
    /* TODO: 从节点删除标签 */
    
    return true;
}

GraphLabel**
graph_list_labels(EpiphanySmgrRelation *relation, uint32_t *num_labels)
{
    if (!relation || !num_labels)
        return NULL;
        
    /* TODO: 列出所有标签 */
    
    *num_labels = 0;
    return NULL;
}

/*
 * 图索引操作
 */

bool
graph_create_node_index(EpiphanySmgrRelation *relation, const char *index_name, const char *property_name, 
                       GraphIndexType index_type, bool is_unique)
{
    if (!relation || !index_name || !property_name)
        return false;
        
    /* TODO: 创建节点索引 */
    
    if (g_graph_stats)
        g_graph_stats->index_operations++;
        
    return true;
}

bool
graph_create_edge_index(EpiphanySmgrRelation *relation, const char *index_name, const char *property_name, 
                       GraphIndexType index_type, bool is_unique)
{
    if (!relation || !index_name || !property_name)
        return false;
        
    /* TODO: 创建边索引 */
    
    if (g_graph_stats)
        g_graph_stats->index_operations++;
        
    return true;
}

bool
graph_create_composite_index(EpiphanySmgrRelation *relation, const char *index_name, 
                            const char **property_names, uint32_t num_properties, 
                            GraphIndexType index_type, bool is_unique)
{
    if (!relation || !index_name || !property_names || num_properties == 0)
        return false;
        
    /* TODO: 创建复合索引 */
    
    if (g_graph_stats)
        g_graph_stats->index_operations++;
        
    return true;
}

bool
graph_drop_index(EpiphanySmgrRelation *relation, const char *index_name)
{
    if (!relation || !index_name)
        return false;
        
    /* TODO: 删除索引 */
    
    return true;
}

void
graph_rebuild_indexes(EpiphanySmgrRelation *relation)
{
    if (!relation)
        return;
        
    /* TODO: 重建索引 */
    printf("EpiphanyDB: 重建图索引\n");
}

GraphIndex**
graph_list_indexes(EpiphanySmgrRelation *relation, uint32_t *num_indexes)
{
    if (!relation || !num_indexes)
        return NULL;
        
    /* TODO: 列出所有索引 */
    
    *num_indexes = 0;
    return NULL;
}

/*
 * 图算法
 */

double*
graph_pagerank(EpiphanySmgrRelation *relation, uint32_t max_iterations, double damping_factor)
{
    if (!relation)
        return NULL;
        
    /* TODO: 实现 PageRank 算法 */
    
    return NULL;
}

uint64_t**
graph_connected_components(EpiphanySmgrRelation *relation, uint32_t *num_components)
{
    if (!relation || !num_components)
        return NULL;
        
    /* TODO: 查找连通分量 */
    
    *num_components = 0;
    return NULL;
}

GraphNode**
graph_community_detection(EpiphanySmgrRelation *relation, const char *algorithm, uint32_t *num_communities)
{
    if (!relation || !algorithm || !num_communities)
        return NULL;
        
    /* TODO: 社区检测 */
    
    *num_communities = 0;
    return NULL;
}

double
graph_clustering_coefficient(EpiphanySmgrRelation *relation, uint64_t node_id)
{
    if (!relation || node_id == 0)
        return 0.0;
        
    /* TODO: 计算聚类系数 */
    
    return 0.0;
}

uint32_t
graph_node_degree(EpiphanySmgrRelation *relation, uint64_t node_id, GraphTraversalDirection direction)
{
    if (!relation || node_id == 0)
        return 0;
        
    /* TODO: 计算节点度数 */
    
    return 0;
}

/*
 * 图数据维护
 */

void
graph_compact_data(EpiphanySmgrRelation *relation)
{
    if (!relation)
        return;
        
    /* TODO: 压缩数据 */
    printf("EpiphanyDB: 压缩图数据\n");
}

void
graph_vacuum_deleted_data(EpiphanySmgrRelation *relation)
{
    if (!relation)
        return;
        
    /* TODO: 清理已删除数据 */
    printf("EpiphanyDB: 清理已删除的图数据\n");
}

void
graph_rebalance_storage(EpiphanySmgrRelation *relation)
{
    if (!relation)
        return;
        
    /* TODO: 重新平衡存储 */
    printf("EpiphanyDB: 重新平衡图存储\n");
}

void
graph_update_statistics(EpiphanySmgrRelation *relation)
{
    if (!relation)
        return;
        
    /* TODO: 更新统计信息 */
    printf("EpiphanyDB: 更新图统计信息\n");
}

/*
 * 统计信息和优化
 */

GraphStats*
graph_collect_stats(EpiphanySmgrRelation *relation)
{
    if (!relation)
        return NULL;
        
    /* TODO: 收集统计信息 */
    
    return g_graph_stats;
}

void
graph_reset_stats(EpiphanySmgrRelation *relation)
{
    if (!relation)
        return;
        
    /* TODO: 重置统计信息 */
    
    if (g_graph_stats)
        memset(g_graph_stats, 0, sizeof(GraphStats));
}

void
graph_optimize_storage(EpiphanySmgrRelation *relation)
{
    if (!relation)
        return;
        
    /* TODO: 优化存储 */
    printf("EpiphanyDB: 优化图存储\n");
}

void
graph_analyze_workload(EpiphanySmgrRelation *relation)
{
    if (!relation)
        return;
        
    /* TODO: 分析工作负载 */
    printf("EpiphanyDB: 分析图工作负载\n");
}

/*
 * 事务支持
 */

void
graph_begin_transaction(EpiphanySmgrRelation *relation)
{
    if (!relation)
        return;
        
    /* TODO: 开始事务 */
    printf("EpiphanyDB: 开始图存储事务\n");
}

void
graph_commit_transaction(EpiphanySmgrRelation *relation)
{
    if (!relation)
        return;
        
    /* TODO: 提交事务 */
    printf("EpiphanyDB: 提交图存储事务\n");
}

void
graph_abort_transaction(EpiphanySmgrRelation *relation)
{
    if (!relation)
        return;
        
    /* TODO: 回滚事务 */
    printf("EpiphanyDB: 回滚图存储事务\n");
}

void
graph_prepare_transaction(EpiphanySmgrRelation *relation, const char *gid)
{
    if (!relation || !gid)
        return;
        
    /* TODO: 准备事务 */
    printf("EpiphanyDB: 准备图存储事务: %s\n", gid);
}

/*
 * 备份和恢复
 */

void
graph_backup_relation(EpiphanySmgrRelation *relation, const char *backup_path)
{
    if (!relation || !backup_path)
        return;
        
    /* TODO: 备份关系 */
    printf("EpiphanyDB: 备份图存储关系到: %s\n", backup_path);
}

void
graph_restore_relation(EpiphanySmgrRelation *relation, const char *backup_path)
{
    if (!relation || !backup_path)
        return;
        
    /* TODO: 恢复关系 */
    printf("EpiphanyDB: 从备份恢复图存储关系: %s\n", backup_path);
}

void
graph_export_data(EpiphanySmgrRelation *relation, const char *export_path, const GraphQueryParamsExt *params)
{
    if (!relation || !export_path)
        return;
        
    /* TODO: 导出数据 */
    printf("EpiphanyDB: 导出图数据到: %s\n", export_path);
}

void
graph_import_data(EpiphanySmgrRelation *relation, const char *import_path)
{
    if (!relation || !import_path)
        return;
        
    /* TODO: 导入数据 */
    printf("EpiphanyDB: 从文件导入图数据: %s\n", import_path);
}

/*
 * 监控和诊断
 */

void
graph_get_health_status(EpiphanySmgrRelation *relation, char **status_json)
{
    if (!relation || !status_json)
        return;
        
    /* TODO: 获取健康状态 */
    *status_json = strdup("{\"status\": \"healthy\", \"engine\": \"graph\"}");
}

void
graph_get_performance_metrics(EpiphanySmgrRelation *relation, char **metrics_json)
{
    if (!relation || !metrics_json)
        return;
        
    /* TODO: 获取性能指标 */
    *metrics_json = strdup("{\"nodes\": 0, \"edges\": 0, \"queries\": 0, \"traversals\": 0}");
}

void
graph_diagnose_performance(EpiphanySmgrRelation *relation, char **diagnosis_json)
{
    if (!relation || !diagnosis_json)
        return;
        
    /* TODO: 诊断性能 */
    *diagnosis_json = strdup("{\"diagnosis\": \"normal\", \"recommendations\": []}");
}

/*
 * 配置管理
 */

GraphEngineConfig*
graph_get_default_config(void)
{
    GraphEngineConfig *config = (GraphEngineConfig*)malloc(sizeof(GraphEngineConfig));
    if (!config)
        return NULL;
        
    memset(config, 0, sizeof(GraphEngineConfig));
    
    config->node_block_size = GRAPH_BLOCK_SIZE;
    config->edge_block_size = GRAPH_BLOCK_SIZE;
    config->property_block_size = GRAPH_BLOCK_SIZE / 2;
    config->max_nodes_per_block = 100;
    config->max_edges_per_block = 200;
    
    config->enable_node_index = true;
    config->enable_edge_index = true;
    config->enable_property_index = true;
    config->enable_label_index = true;
    config->default_index_type = strdup("btree");
    
    config->max_traversal_depth = 10;
    config->max_result_nodes = 10000;
    config->max_result_edges = 20000;
    config->enable_query_cache = true;
    config->query_timeout_ms = 30000;
    
    config->enable_transactions = true;
    config->transaction_timeout_ms = 60000;
    config->enable_write_ahead_log = true;
    
    config->cache_size_mb = 256;
    config->enable_compression = true;
    config->compression_algorithm = strdup("lz4");
    config->enable_parallel_traversal = true;
    config->max_worker_threads = 4;
    
    config->compaction_interval_seconds = 3600;
    config->enable_auto_compaction = true;
    config->enable_statistics_collection = true;
    
    return config;
}

void
graph_set_config(GraphEngineConfig *config)
{
    if (!config)
        return;
        
    if (g_graph_config)
    {
        if (g_graph_config->default_index_type)
            free(g_graph_config->default_index_type);
        if (g_graph_config->compression_algorithm)
            free(g_graph_config->compression_algorithm);
        free(g_graph_config);
    }
        
    g_graph_config = (GraphEngineConfig*)malloc(sizeof(GraphEngineConfig));
    if (g_graph_config)
        memcpy(g_graph_config, config, sizeof(GraphEngineConfig));
}

GraphEngineConfig*
graph_get_config(void)
{
    return g_graph_config;
}

/*
 * 工具函数
 */

EpiphanyStorageEngine*
create_graph_storage_engine(void)
{
    EpiphanyStorageEngine *engine = (EpiphanyStorageEngine*)malloc(sizeof(EpiphanyStorageEngine));
    if (!engine)
        return NULL;
        
    memset(engine, 0, sizeof(EpiphanyStorageEngine));
    
    engine->engine_type = STORAGE_ENGINE_GRAPH;
    engine->engine_name = "graph";
    engine->engine_status = ENGINE_STATUS_UNINITIALIZED;
    
    /* 设置存储管理器函数指针 */
    engine->smgr_init = graph_smgr_init;
    engine->smgr_shutdown = graph_smgr_shutdown;
    engine->smgr_open = graph_smgr_open;
    engine->smgr_close = graph_smgr_close;
    engine->smgr_create = graph_smgr_create;
    engine->smgr_exists = graph_smgr_exists;
    engine->smgr_unlink = graph_smgr_unlink;
    engine->smgr_extend = graph_smgr_extend;
    engine->smgr_zeroextend = graph_smgr_zeroextend;
    engine->smgr_prefetch = graph_smgr_prefetch;
    engine->smgr_readv = graph_smgr_readv;
    engine->smgr_writev = graph_smgr_writev;
    engine->smgr_writeback = graph_smgr_writeback;
    engine->smgr_nblocks = graph_smgr_nblocks;
    engine->smgr_truncate = graph_smgr_truncate;
    engine->smgr_immedsync = graph_smgr_immedsync;
    engine->smgr_registersync = graph_smgr_registersync;
    
    /* 设置引擎特定函数指针 */
    engine->engine_init = graph_engine_init;
    engine->engine_shutdown = graph_engine_shutdown;
    engine->engine_configure = graph_engine_configure;
    
    /* 设置图查询函数指针 */
    /* engine->graph_traverse = graph_traverse; */ /* TODO: 需要适配函数签名 */
    
    /* 设置统计和优化函数指针 */
    /* engine->collect_stats = graph_collect_stats; */ /* TODO: 需要适配函数签名 */
    engine->optimize_storage = graph_optimize_storage;
    engine->rebuild_indexes = graph_rebuild_indexes;
    
    /* 设置事务支持函数指针 */
    engine->begin_transaction = graph_begin_transaction;
    engine->commit_transaction = graph_commit_transaction;
    engine->abort_transaction = graph_abort_transaction;
    engine->prepare_transaction = graph_prepare_transaction;
    
    /* 设置备份恢复函数指针 */
    engine->backup_relation = graph_backup_relation;
    engine->restore_relation = graph_restore_relation;
    
    /* 设置监控诊断函数指针 */
    engine->get_health_status = graph_get_health_status;
    engine->get_performance_metrics = graph_get_performance_metrics;
    
    return engine;
}

void
register_graph_storage_engine(void)
{
    EpiphanyStorageEngine *engine = create_graph_storage_engine();
    if (engine)
    {
        register_storage_engine(STORAGE_ENGINE_GRAPH, engine);
        printf("EpiphanyDB: 图存储引擎已注册\n");
    }
}

const char*
graph_data_type_name(GraphDataType type)
{
    switch (type)
    {
        case GRAPH_DATA_NULL: return "null";
        case GRAPH_DATA_BOOL: return "bool";
        case GRAPH_DATA_INT32: return "int32";
        case GRAPH_DATA_INT64: return "int64";
        case GRAPH_DATA_FLOAT32: return "float32";
        case GRAPH_DATA_FLOAT64: return "float64";
        case GRAPH_DATA_STRING: return "string";
        case GRAPH_DATA_BYTES: return "bytes";
        case GRAPH_DATA_ARRAY: return "array";
        case GRAPH_DATA_MAP: return "map";
        default: return "unknown";
    }
}

const char*
graph_index_type_name(GraphIndexType type)
{
    switch (type)
    {
        case GRAPH_INDEX_BTREE: return "btree";
        case GRAPH_INDEX_HASH: return "hash";
        case GRAPH_INDEX_FULLTEXT: return "fulltext";
        case GRAPH_INDEX_SPATIAL: return "spatial";
        case GRAPH_INDEX_COMPOSITE: return "composite";
        default: return "unknown";
    }
}

const char*
graph_traversal_algorithm_name(GraphTraversalAlgorithm algorithm)
{
    switch (algorithm)
    {
        case GRAPH_TRAVERSAL_BFS: return "bfs";
        case GRAPH_TRAVERSAL_DFS: return "dfs";
        case GRAPH_TRAVERSAL_DIJKSTRA: return "dijkstra";
        case GRAPH_TRAVERSAL_A_STAR: return "a_star";
        case GRAPH_TRAVERSAL_PAGERANK: return "pagerank";
        case GRAPH_TRAVERSAL_SHORTEST_PATH: return "shortest_path";
        default: return "unknown";
    }
}

size_t
graph_data_type_size(GraphDataType type)
{
    switch (type)
    {
        case GRAPH_DATA_NULL: return 0;
        case GRAPH_DATA_BOOL: return sizeof(bool);
        case GRAPH_DATA_INT32: return sizeof(int32_t);
        case GRAPH_DATA_INT64: return sizeof(int64_t);
        case GRAPH_DATA_FLOAT32: return sizeof(float);
        case GRAPH_DATA_FLOAT64: return sizeof(double);
        case GRAPH_DATA_STRING: return sizeof(char*);
        case GRAPH_DATA_BYTES: return sizeof(void*);
        case GRAPH_DATA_ARRAY: return sizeof(void*);
        case GRAPH_DATA_MAP: return sizeof(void*);
        default: return 0;
    }
}

uint32_t
graph_calculate_node_checksum(const GraphNode *node)
{
    if (!node)
        return 0;
        
    return calculate_node_checksum(node);
}

uint32_t
graph_calculate_edge_checksum(const GraphEdge *edge)
{
    if (!edge)
        return 0;
        
    return calculate_edge_checksum(edge);
}

bool
graph_validate_node(const GraphNode *node)
{
    if (!node)
        return false;
        
    /* 检查主标签 */
    if (!node->primary_label || strlen(node->primary_label) == 0)
        return false;
        
    /* 检查属性 */
    if (node->num_properties > 0 && !node->properties)
        return false;
        
    return true;
}

bool
graph_validate_edge(const GraphEdge *edge)
{
    if (!edge)
        return false;
        
    /* 检查节点ID */
    if (edge->source_node_id == 0 || edge->target_node_id == 0)
        return false;
        
    /* 检查边类型 */
    if (!edge->edge_type || strlen(edge->edge_type) == 0)
        return false;
        
    /* 检查属性 */
    if (edge->num_properties > 0 && !edge->properties)
        return false;
        
    return true;
}

GraphPropertyValue*
graph_parse_property_value(const char *value_str, GraphDataType type)
{
    if (!value_str)
        return NULL;
        
    GraphPropertyValue *value = (GraphPropertyValue*)malloc(sizeof(GraphPropertyValue));
    if (!value)
        return NULL;
        
    memset(value, 0, sizeof(GraphPropertyValue));
    value->type = type;
    
    /* TODO: 解析属性值 */
    
    return value;
}

char*
graph_format_property_value(const GraphPropertyValue *value)
{
    if (!value)
        return NULL;
        
    /* TODO: 格式化属性值 */
    
    return strdup("null");
}

void
graph_free_property_value(GraphPropertyValue *value)
{
    if (!value)
        return;
        
    switch (value->type)
    {
        case GRAPH_DATA_STRING:
            if (value->string_value)
                free(value->string_value);
            break;
        case GRAPH_DATA_BYTES:
            if (value->bytes_value.data)
                free(value->bytes_value.data);
            break;
        case GRAPH_DATA_ARRAY:
            if (value->array_value.values)
            {
                for (uint32_t i = 0; i < value->array_value.count; i++)
                    graph_free_property_value(&value->array_value.values[i]);
                free(value->array_value.values);
            }
            break;
        case GRAPH_DATA_MAP:
            if (value->map_value.keys)
            {
                for (uint32_t i = 0; i < value->map_value.count; i++)
                {
                    if (value->map_value.keys[i])
                        free(value->map_value.keys[i]);
                }
                free(value->map_value.keys);
            }
            if (value->map_value.values)
            {
                for (uint32_t i = 0; i < value->map_value.count; i++)
                    graph_free_property_value(&value->map_value.values[i]);
                free(value->map_value.values);
            }
            break;
        default:
            break;
    }
    
    free(value);
}

void
graph_free_node(GraphNode *node)
{
    free_graph_node(node);
}

void
graph_free_edge(GraphEdge *edge)
{
    free_graph_edge(edge);
}

void
graph_free_path(GraphPath *path)
{
    if (!path)
        return;
        
    if (path->nodes)
    {
        for (uint32_t i = 0; i < path->length; i++)
        {
            if (path->nodes[i])
                free_graph_node(path->nodes[i]);
        }
        free(path->nodes);
    }
    
    if (path->edges)
    {
        for (uint32_t i = 0; i < path->length - 1; i++)
        {
            if (path->edges[i])
                free_graph_edge(path->edges[i]);
        }
        free(path->edges);
    }
    
    if (path->path_signature)
        free(path->path_signature);
        
    free(path);
}

/*
 * 内部函数实现
 */

static void
init_graph_config(void)
{
    g_graph_config = graph_get_default_config();
}

static void
init_graph_stats(void)
{
    g_graph_stats = (GraphStats*)malloc(sizeof(GraphStats));
    if (g_graph_stats)
    {
        memset(g_graph_stats, 0, sizeof(GraphStats));
        g_graph_stats->last_updated = time(NULL);
        g_graph_stats->start_time = time(NULL);
    }
}

static uint32_t
calculate_node_checksum(const GraphNode *node)
{
    if (!node)
        return 0;
        
    /* 简单的校验和计算 */
    uint32_t checksum = 0;
    
    /* 节点ID */
    checksum ^= (uint32_t)(node->node_id & 0xFFFFFFFF);
    checksum ^= (uint32_t)(node->node_id >> 32);
    
    /* 主标签 */
    if (node->primary_label)
    {
        const char *label = node->primary_label;
        for (int i = 0; label[i] != '\0'; i++)
        {
            checksum = checksum * 31 + label[i];
        }
    }
    
    /* 属性 */
    for (uint32_t i = 0; i < node->num_properties && node->properties; i++)
    {
        const char *name = node->properties[i].name;
        if (name)
        {
            for (int j = 0; name[j] != '\0'; j++)
                checksum = checksum * 31 + name[j];
        }
    }
    
    return checksum;
}

static uint32_t
calculate_edge_checksum(const GraphEdge *edge)
{
    if (!edge)
        return 0;
        
    /* 简单的校验和计算 */
    uint32_t checksum = 0;
    
    /* 边ID */
    checksum ^= (uint32_t)(edge->edge_id & 0xFFFFFFFF);
    checksum ^= (uint32_t)(edge->edge_id >> 32);
    
    /* 源节点ID */
    checksum ^= (uint32_t)(edge->source_node_id & 0xFFFFFFFF);
    checksum ^= (uint32_t)(edge->source_node_id >> 32);
    
    /* 目标节点ID */
    checksum ^= (uint32_t)(edge->target_node_id & 0xFFFFFFFF);
    checksum ^= (uint32_t)(edge->target_node_id >> 32);
    
    /* 边类型 */
    if (edge->edge_type)
    {
        const char *type = edge->edge_type;
        for (int i = 0; type[i] != '\0'; i++)
        {
            checksum = checksum * 31 + type[i];
        }
    }
    
    return checksum;
}

static bool
validate_graph_file(GraphFile *file)
{
    if (!file)
        return false;
        
    /* 检查文件版本 */
    if (file->file_version != GRAPH_FILE_VERSION)
        return false;
        
    /* 检查文件大小 */
    if (file->file_size < sizeof(GraphFile))
        return false;
        
    return true;
}

static void
write_file_header(GraphFile *file)
{
    if (!file || file->file_descriptor < 0)
        return;
        
    /* TODO: 写入文件头 */
    lseek(file->file_descriptor, 0, SEEK_SET);
    write(file->file_descriptor, file, sizeof(GraphFile));
}

static void
read_file_header(GraphFile *file)
{
    if (!file || file->file_descriptor < 0)
        return;
        
    /* TODO: 读取文件头 */
    lseek(file->file_descriptor, 0, SEEK_SET);
    read(file->file_descriptor, file, sizeof(GraphFile));
}

static GraphNode*
create_graph_node(uint64_t node_id)
{
    GraphNode *node = (GraphNode*)malloc(sizeof(GraphNode));
    if (!node)
        return NULL;
        
    memset(node, 0, sizeof(GraphNode));
    node->node_id = node_id;
    
    return node;
}

static GraphEdge*
create_graph_edge(uint64_t edge_id, uint64_t source_id, uint64_t target_id)
{
    GraphEdge *edge = (GraphEdge*)malloc(sizeof(GraphEdge));
    if (!edge)
        return NULL;
        
    memset(edge, 0, sizeof(GraphEdge));
    edge->edge_id = edge_id;
    edge->source_node_id = source_id;
    edge->target_node_id = target_id;
    
    return edge;
}

static void
free_graph_node(GraphNode *node)
{
    if (!node)
        return;
        
    if (node->primary_label)
        free(node->primary_label);
        
    if (node->labels)
    {
        for (uint32_t i = 0; i < node->num_labels; i++)
        {
            if (node->labels[i])
                free(node->labels[i]);
        }
        free(node->labels);
    }
    
    if (node->properties)
    {
        for (uint32_t i = 0; i < node->num_properties; i++)
        {
            if (node->properties[i].name)
                free(node->properties[i].name);
            graph_free_property_value(&node->properties[i].value);
        }
        free(node->properties);
    }
    
    free(node);
}

static void
free_graph_edge(GraphEdge *edge)
{
    if (!edge)
        return;
        
    if (edge->edge_type)
        free(edge->edge_type);
        
    if (edge->properties)
    {
        for (uint32_t i = 0; i < edge->num_properties; i++)
        {
            if (edge->properties[i].name)
                free(edge->properties[i].name);
            graph_free_property_value(&edge->properties[i].value);
        }
        free(edge->properties);
    }
    
    free(edge);
}