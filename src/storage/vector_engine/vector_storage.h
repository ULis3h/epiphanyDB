/*-------------------------------------------------------------------------
 *
 * vector_storage.h
 *    EpiphanyDB 向量存储引擎 (Faiss 兼容)
 *
 * 基于 Faiss 的向量存储实现，提供高效的向量相似性搜索，
 * 支持多种索引类型、距离度量和近似搜索算法。
 *
 * Copyright (c) 2024, EpiphanyDB Team
 *
 * IDENTIFICATION
 *    src/storage/vector_engine/vector_storage.h
 *
 *-------------------------------------------------------------------------
 */
#ifndef EPIPHANYDB_VECTOR_STORAGE_H
#define EPIPHANYDB_VECTOR_STORAGE_H

#include "../../../include/epiphanydb/storage/storage_manager.h"

/* 向量存储引擎配置 */
typedef struct VectorEngineConfig
{
    int         default_dimension;      /* 默认向量维度 */
    const char *default_index_type;     /* 默认索引类型 */
    const char *default_metric;         /* 默认距离度量 */
    int         nprobe;                 /* 搜索时的探测数量 */
    int         nlist;                  /* 聚类中心数量 */
    int         m;                      /* PQ 编码参数 */
    int         nbits;                  /* PQ 编码位数 */
    bool        use_gpu;                /* 是否使用 GPU */
    int         gpu_device;             /* GPU 设备 ID */
    int         max_vectors_per_chunk;  /* 每个块的最大向量数 */
    int         index_build_threshold;  /* 索引构建阈值 */
    bool        enable_compression;     /* 是否启用压缩 */
    bool        enable_quantization;    /* 是否启用量化 */
    double      training_sample_ratio;  /* 训练样本比例 */
} VectorEngineConfig;

/* 向量索引类型枚举 */
typedef enum VectorIndexType
{
    VECTOR_INDEX_FLAT = 0,              /* 暴力搜索 */
    VECTOR_INDEX_IVF_FLAT,              /* IVF + 暴力搜索 */
    VECTOR_INDEX_IVF_PQ,                /* IVF + 乘积量化 */
    VECTOR_INDEX_IVF_SQ8,               /* IVF + 标量量化 */
    VECTOR_INDEX_HNSW,                  /* 分层导航小世界 */
    VECTOR_INDEX_LSH,                   /* 局部敏感哈希 */
    VECTOR_INDEX_ANNOY,                 /* Annoy 索引 */
    VECTOR_INDEX_NGTPANNG,              /* NGT-panng 索引 */
    VECTOR_INDEX_COUNT
} VectorIndexType;

/* 距离度量类型枚举 */
typedef enum VectorMetricType
{
    VECTOR_METRIC_L2 = 0,               /* 欧几里得距离 */
    VECTOR_METRIC_IP,                   /* 内积 */
    VECTOR_METRIC_COSINE,               /* 余弦相似度 */
    VECTOR_METRIC_L1,                   /* 曼哈顿距离 */
    VECTOR_METRIC_LINF,                 /* 无穷范数 */
    VECTOR_METRIC_HAMMING,              /* 汉明距离 */
    VECTOR_METRIC_JACCARD,              /* 杰卡德距离 */
    VECTOR_METRIC_COUNT
} VectorMetricType;

/* 向量数据类型枚举 */
typedef enum VectorDataType
{
    VECTOR_DATA_FLOAT32 = 0,            /* 32位浮点数 */
    VECTOR_DATA_FLOAT16,                /* 16位浮点数 */
    VECTOR_DATA_INT8,                   /* 8位整数 */
    VECTOR_DATA_UINT8,                  /* 8位无符号整数 */
    VECTOR_DATA_BINARY,                 /* 二进制向量 */
    VECTOR_DATA_COUNT
} VectorDataType;

/* 向量元数据 */
typedef struct VectorMeta
{
    uint64_t        vector_id;          /* 向量 ID */
    int             dimension;          /* 向量维度 */
    VectorDataType  data_type;          /* 数据类型 */
    void           *metadata;           /* 用户元数据 */
    size_t          metadata_size;      /* 元数据大小 */
    Timestamp       created_time;       /* 创建时间 */
    Timestamp       updated_time;       /* 更新时间 */
    uint32_t        checksum;           /* 校验和 */
} VectorMeta;

/* 向量数据 */
typedef struct VectorData
{
    VectorMeta      meta;               /* 向量元数据 */
    void           *vector;             /* 向量数据 */
    size_t          vector_size;        /* 向量大小 */
    bool            is_normalized;      /* 是否已归一化 */
} VectorData;

/* 向量索引 */
typedef struct VectorIndex
{
    uint32_t            index_id;       /* 索引 ID */
    VectorIndexType     index_type;     /* 索引类型 */
    VectorMetricType    metric_type;    /* 距离度量 */
    int                 dimension;      /* 向量维度 */
    VectorDataType      data_type;      /* 数据类型 */
    uint64_t            total_vectors;  /* 总向量数 */
    bool                is_trained;     /* 是否已训练 */
    void               *index_data;     /* 索引数据 */
    size_t              index_size;     /* 索引大小 */
    void               *config;         /* 索引配置 */
    Timestamp           created_time;   /* 创建时间 */
    Timestamp           trained_time;   /* 训练时间 */
    uint32_t            checksum;       /* 校验和 */
} VectorIndex;

/* 向量搜索结果 */
typedef struct VectorSearchResult
{
    uint64_t        vector_id;          /* 向量 ID */
    float           distance;           /* 距离/相似度 */
    void           *metadata;           /* 元数据 */
    size_t          metadata_size;      /* 元数据大小 */
} VectorSearchResult;

/* 向量搜索参数 */
typedef struct VectorSearchParams
{
    const void         *query_vector;   /* 查询向量 */
    int                 dimension;      /* 向量维度 */
    VectorDataType      data_type;      /* 数据类型 */
    int                 k;              /* 返回结果数 */
    float               threshold;      /* 距离阈值 */
    VectorMetricType    metric_type;    /* 距离度量 */
    int                 nprobe;         /* 探测数量 */
    bool                include_metadata; /* 是否包含元数据 */
    void               *filter;         /* 过滤条件 */
} VectorSearchParams;

/* 向量批次 */
typedef struct VectorBatch
{
    uint32_t        batch_size;         /* 批次大小 */
    int             dimension;          /* 向量维度 */
    VectorDataType  data_type;          /* 数据类型 */
    void          **vectors;            /* 向量数组 */
    uint64_t       *vector_ids;         /* 向量 ID 数组 */
    void          **metadata;           /* 元数据数组 */
    size_t         *metadata_sizes;     /* 元数据大小数组 */
} VectorBatch;

/* 向量存储文件 */
typedef struct VectorFile
{
    char               *file_path;      /* 文件路径 */
    int                 file_descriptor; /* 文件描述符 */
    uint32_t            num_indexes;    /* 索引数量 */
    VectorIndex        *indexes;        /* 索引数组 */
    uint64_t            total_vectors;  /* 总向量数 */
    uint64_t            file_size;      /* 文件大小 */
    void               *header;         /* 文件头 */
    void               *metadata;       /* 元数据 */
    bool                is_open;        /* 是否已打开 */
    bool                is_readonly;    /* 是否只读 */
} VectorFile;

/* 向量扫描描述符 */
typedef struct VectorScanDesc
{
    EpiphanySmgrRelation   *relation;   /* 关系 */
    VectorFile             *file;       /* 向量文件 */
    VectorSearchParams      params;     /* 搜索参数 */
    VectorSearchResult     *results;    /* 搜索结果 */
    uint32_t                num_results; /* 结果数量 */
    uint32_t                current_pos; /* 当前位置 */
    bool                    is_finished; /* 是否完成 */
} VectorScanDesc;

/* 向量插入状态 */
typedef struct VectorInsertState
{
    EpiphanySmgrRelation   *relation;   /* 关系 */
    VectorFile             *file;       /* 向量文件 */
    VectorBatch            *batch;      /* 当前批次 */
    uint32_t                batch_pos;  /* 批次位置 */
    uint32_t                max_batch_size; /* 最大批次大小 */
    bool                    auto_flush; /* 是否自动刷新 */
    bool                    auto_index; /* 是否自动建索引 */
} VectorInsertState;

/* 向量统计信息 */
typedef struct VectorStats
{
    uint64_t            total_vectors;  /* 总向量数 */
    uint64_t            total_indexes;  /* 总索引数 */
    uint64_t            total_files;    /* 总文件数 */
    uint64_t            search_operations; /* 搜索操作数 */
    uint64_t            insert_operations; /* 插入操作数 */
    uint64_t            index_operations;  /* 索引操作数 */
    double              avg_search_time;   /* 平均搜索时间 */
    double              avg_insert_time;   /* 平均插入时间 */
    double              avg_index_time;    /* 平均索引时间 */
    uint64_t            cache_hits;        /* 缓存命中数 */
    uint64_t            cache_misses;      /* 缓存未命中数 */
    double              cache_hit_ratio;   /* 缓存命中率 */
    Timestamp           last_updated;      /* 最后更新时间 */
} VectorStats;

/* 函数声明 */

/* 向量存储引擎初始化和关闭 */
extern void vector_engine_init(void);
extern void vector_engine_shutdown(void);
extern void vector_engine_configure(const char *config_json);

/* PostgreSQL 兼容的存储管理器接口 */
extern void vector_smgr_init(void);
extern void vector_smgr_shutdown(void);
extern void vector_smgr_open(SMgrRelation reln);
extern void vector_smgr_close(SMgrRelation reln, ForkNumber forknum);
extern void vector_smgr_create(SMgrRelation reln, ForkNumber forknum, bool isRedo);
extern bool vector_smgr_exists(SMgrRelation reln, ForkNumber forknum);
extern void vector_smgr_unlink(RelFileLocatorBackend rlocator, ForkNumber forknum, bool isRedo);
extern void vector_smgr_extend(SMgrRelation reln, ForkNumber forknum, BlockNumber blocknum, 
                              const void *buffer, bool skipFsync);
extern void vector_smgr_zeroextend(SMgrRelation reln, ForkNumber forknum, BlockNumber blocknum, 
                                  int nblocks, bool skipFsync);
extern bool vector_smgr_prefetch(SMgrRelation reln, ForkNumber forknum, BlockNumber blocknum, 
                                int nblocks);
extern void vector_smgr_readv(SMgrRelation reln, ForkNumber forknum, BlockNumber blocknum,
                             void **buffers, BlockNumber nblocks);
extern void vector_smgr_writev(SMgrRelation reln, ForkNumber forknum, BlockNumber blocknum,
                              const void **buffers, BlockNumber nblocks, bool skipFsync);
extern void vector_smgr_writeback(SMgrRelation reln, ForkNumber forknum, BlockNumber blocknum, 
                                 BlockNumber nblocks);
extern BlockNumber vector_smgr_nblocks(SMgrRelation reln, ForkNumber forknum);
extern void vector_smgr_truncate(SMgrRelation reln, ForkNumber forknum, BlockNumber nblocks);
extern void vector_smgr_immedsync(SMgrRelation reln, ForkNumber forknum);
extern void vector_smgr_registersync(SMgrRelation reln, ForkNumber forknum);

/* 向量文件操作 */
extern VectorFile* vector_file_create(const char *file_path, int dimension, 
                                     VectorDataType data_type, VectorMetricType metric_type);
extern VectorFile* vector_file_open(const char *file_path, bool readonly);
extern void vector_file_close(VectorFile *file);
extern void vector_file_flush(VectorFile *file);
extern void vector_file_sync(VectorFile *file);

/* 向量索引操作 */
extern VectorIndex* vector_create_index(VectorIndexType index_type, int dimension, 
                                       VectorDataType data_type, VectorMetricType metric_type);
extern bool vector_train_index(VectorIndex *index, VectorBatch *training_data);
extern bool vector_add_to_index(VectorIndex *index, VectorBatch *batch);
extern bool vector_remove_from_index(VectorIndex *index, uint64_t *vector_ids, int num_ids);
extern void vector_destroy_index(VectorIndex *index);

/* 向量搜索操作 */
extern VectorScanDesc* vector_begin_search(EpiphanySmgrRelation *relation, 
                                          const VectorSearchParams *params);
extern VectorSearchResult* vector_search_next(VectorScanDesc *scan);
extern void vector_end_search(VectorScanDesc *scan);
extern VectorSearchResult* vector_search_batch(EpiphanySmgrRelation *relation, 
                                              const VectorSearchParams *params, 
                                              int *num_results);

/* 向量插入操作 */
extern VectorInsertState* vector_begin_insert(EpiphanySmgrRelation *relation);
extern bool vector_insert_vector(VectorInsertState *state, const VectorData *vector);
extern bool vector_insert_batch(VectorInsertState *state, const VectorBatch *batch);
extern void vector_flush_batch(VectorInsertState *state);
extern void vector_end_insert(VectorInsertState *state);

/* 向量更新和删除操作 */
extern bool vector_update_vector(EpiphanySmgrRelation *relation, uint64_t vector_id, 
                                const VectorData *new_vector);
extern bool vector_delete_vector(EpiphanySmgrRelation *relation, uint64_t vector_id);
extern bool vector_delete_batch(EpiphanySmgrRelation *relation, uint64_t *vector_ids, int num_ids);

/* 向量批次操作 */
extern VectorBatch* vector_create_batch(uint32_t batch_size, int dimension, VectorDataType data_type);
extern void vector_free_batch(VectorBatch *batch);
extern bool vector_batch_add_vector(VectorBatch *batch, uint64_t vector_id, const void *vector, 
                                   const void *metadata, size_t metadata_size);
extern bool vector_batch_is_full(const VectorBatch *batch);
extern void vector_batch_clear(VectorBatch *batch);

/* 向量数据操作 */
extern VectorData* vector_create_data(uint64_t vector_id, int dimension, VectorDataType data_type, 
                                     const void *vector, const void *metadata, size_t metadata_size);
extern void vector_free_data(VectorData *data);
extern bool vector_normalize(VectorData *data);
extern float vector_compute_distance(const VectorData *v1, const VectorData *v2, VectorMetricType metric);

/* 统计信息和优化 */
extern VectorStats* vector_collect_stats(EpiphanySmgrRelation *relation);
extern void vector_reset_stats(EpiphanySmgrRelation *relation);
extern void vector_optimize_indexes(EpiphanySmgrRelation *relation);
extern void vector_rebuild_indexes(EpiphanySmgrRelation *relation);
extern void vector_compact_storage(EpiphanySmgrRelation *relation);

/* 事务支持 */
extern void vector_begin_transaction(EpiphanySmgrRelation *relation);
extern void vector_commit_transaction(EpiphanySmgrRelation *relation);
extern void vector_abort_transaction(EpiphanySmgrRelation *relation);
extern void vector_prepare_transaction(EpiphanySmgrRelation *relation, const char *gid);

/* 备份和恢复 */
extern void vector_backup_relation(EpiphanySmgrRelation *relation, const char *backup_path);
extern void vector_restore_relation(EpiphanySmgrRelation *relation, const char *backup_path);

/* 监控和诊断 */
extern void vector_get_health_status(EpiphanySmgrRelation *relation, char **status_json);
extern void vector_get_performance_metrics(EpiphanySmgrRelation *relation, char **metrics_json);

/* 配置管理 */
extern VectorEngineConfig* vector_get_default_config(void);
extern void vector_set_config(VectorEngineConfig *config);
extern VectorEngineConfig* vector_get_config(void);

/* 工具函数 */
extern EpiphanyStorageEngine* create_vector_storage_engine(void);
extern void register_vector_storage_engine(void);
extern const char* vector_index_type_name(VectorIndexType type);
extern const char* vector_metric_type_name(VectorMetricType type);
extern const char* vector_data_type_name(VectorDataType type);
extern size_t vector_data_type_size(VectorDataType type);

/* 常量定义 */
#define VECTOR_FILE_MAGIC           0x56454354  /* "VECT" */
#define VECTOR_INDEX_MAGIC          0x56494458  /* "VIDX" */
#define VECTOR_DEFAULT_DIMENSION    512         /* 默认向量维度 */
#define VECTOR_DEFAULT_K            10          /* 默认返回结果数 */
#define VECTOR_DEFAULT_NPROBE       8           /* 默认探测数量 */
#define VECTOR_DEFAULT_NLIST        1024        /* 默认聚类中心数 */
#define VECTOR_DEFAULT_M            8           /* 默认 PQ 编码参数 */
#define VECTOR_DEFAULT_NBITS        8           /* 默认 PQ 编码位数 */
#define VECTOR_MAX_DIMENSION        65536       /* 最大向量维度 */
#define VECTOR_MAX_BATCH_SIZE       10000       /* 最大批次大小 */
#define VECTOR_DEFAULT_BATCH_SIZE   1000        /* 默认批次大小 */
#define VECTOR_INDEX_BUILD_THRESHOLD 10000      /* 索引构建阈值 */

/* 文件格式版本 */
#define VECTOR_FORMAT_VERSION_MAJOR 1
#define VECTOR_FORMAT_VERSION_MINOR 0

/* 搜索参数限制 */
#define VECTOR_MAX_K                10000       /* 最大返回结果数 */
#define VECTOR_MIN_THRESHOLD        0.0f        /* 最小距离阈值 */
#define VECTOR_MAX_THRESHOLD        1000000.0f  /* 最大距离阈值 */

/* 性能调优参数 */
#define VECTOR_CACHE_SIZE           1024        /* 缓存大小 (MB) */
#define VECTOR_PREFETCH_SIZE        64          /* 预取大小 */
#define VECTOR_PARALLEL_THRESHOLD   1000        /* 并行处理阈值 */

#endif /* EPIPHANYDB_VECTOR_STORAGE_H */