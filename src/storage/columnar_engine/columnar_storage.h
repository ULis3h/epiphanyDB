/*-------------------------------------------------------------------------
 *
 * columnar_storage.h
 *    EpiphanyDB 列存储引擎 (Apache Arrow 兼容)
 *
 * 基于 Apache Arrow 格式的列存储实现，提供高效的分析查询性能，
 * 支持向量化执行、压缩存储和并行处理。
 *
 * Copyright (c) 2024, EpiphanyDB Team
 *
 * IDENTIFICATION
 *    src/storage/columnar_engine/columnar_storage.h
 *
 *-------------------------------------------------------------------------
 */
#ifndef EPIPHANYDB_COLUMNAR_STORAGE_H
#define EPIPHANYDB_COLUMNAR_STORAGE_H

#include "../../../include/epiphanydb/storage/storage_manager.h"

/* 列存储引擎配置 */
typedef struct ColumnarEngineConfig
{
    int         chunk_size;             /* 数据块大小 (行数) */
    int         compression_level;      /* 压缩级别 (0-9) */
    const char *compression_codec;      /* 压缩算法 (lz4, zstd, snappy) */
    bool        enable_dictionary;      /* 是否启用字典编码 */
    bool        enable_bit_packing;     /* 是否启用位打包 */
    bool        enable_run_length;      /* 是否启用游程编码 */
    int         stripe_size;            /* 条带大小 (MB) */
    int         max_file_size;          /* 最大文件大小 (MB) */
    bool        enable_bloom_filter;    /* 是否启用布隆过滤器 */
    bool        enable_zone_maps;       /* 是否启用区域映射 */
    bool        enable_vectorization;   /* 是否启用向量化 */
    int         batch_size;             /* 批处理大小 */
} ColumnarEngineConfig;

/* 数据类型枚举 */
typedef enum ColumnarDataType
{
    COLUMNAR_TYPE_BOOL = 0,
    COLUMNAR_TYPE_INT8,
    COLUMNAR_TYPE_INT16,
    COLUMNAR_TYPE_INT32,
    COLUMNAR_TYPE_INT64,
    COLUMNAR_TYPE_UINT8,
    COLUMNAR_TYPE_UINT16,
    COLUMNAR_TYPE_UINT32,
    COLUMNAR_TYPE_UINT64,
    COLUMNAR_TYPE_FLOAT32,
    COLUMNAR_TYPE_FLOAT64,
    COLUMNAR_TYPE_STRING,
    COLUMNAR_TYPE_BINARY,
    COLUMNAR_TYPE_DATE,
    COLUMNAR_TYPE_TIMESTAMP,
    COLUMNAR_TYPE_DECIMAL,
    COLUMNAR_TYPE_LIST,
    COLUMNAR_TYPE_STRUCT,
    COLUMNAR_TYPE_MAP,
    COLUMNAR_TYPE_COUNT
} ColumnarDataType;

/* 压缩类型枚举 */
typedef enum ColumnarCompressionType
{
    COLUMNAR_COMPRESSION_NONE = 0,
    COLUMNAR_COMPRESSION_LZ4,
    COLUMNAR_COMPRESSION_ZSTD,
    COLUMNAR_COMPRESSION_SNAPPY,
    COLUMNAR_COMPRESSION_GZIP,
    COLUMNAR_COMPRESSION_BROTLI,
    COLUMNAR_COMPRESSION_COUNT
} ColumnarCompressionType;

/* 编码类型枚举 */
typedef enum ColumnarEncodingType
{
    COLUMNAR_ENCODING_PLAIN = 0,
    COLUMNAR_ENCODING_DICTIONARY,
    COLUMNAR_ENCODING_RLE,
    COLUMNAR_ENCODING_BIT_PACKED,
    COLUMNAR_ENCODING_DELTA,
    COLUMNAR_ENCODING_DELTA_BINARY_PACKED,
    COLUMNAR_ENCODING_COUNT
} ColumnarEncodingType;

/* 列元数据 */
typedef struct ColumnarColumnMeta
{
    char               *column_name;    /* 列名 */
    ColumnarDataType    data_type;      /* 数据类型 */
    ColumnarCompressionType compression; /* 压缩类型 */
    ColumnarEncodingType encoding;      /* 编码类型 */
    uint64_t           total_values;    /* 总值数 */
    uint64_t           null_count;      /* NULL 值数量 */
    uint64_t           distinct_count;  /* 不同值数量 */
    uint64_t           compressed_size; /* 压缩后大小 */
    uint64_t           uncompressed_size; /* 未压缩大小 */
    void              *min_value;       /* 最小值 */
    void              *max_value;       /* 最大值 */
    void              *statistics;      /* 统计信息 */
    void              *bloom_filter;    /* 布隆过滤器 */
} ColumnarColumnMeta;

/* 数据块 (Chunk) */
typedef struct ColumnarChunk
{
    uint32_t           chunk_id;        /* 块 ID */
    uint32_t           row_count;       /* 行数 */
    uint32_t           column_count;    /* 列数 */
    ColumnarColumnMeta *columns;        /* 列元数据数组 */
    void             **column_data;     /* 列数据数组 */
    uint64_t          *column_sizes;    /* 列大小数组 */
    void              *null_bitmap;     /* NULL 位图 */
    uint64_t           total_size;      /* 总大小 */
    Timestamp          created_time;    /* 创建时间 */
    uint32_t           checksum;        /* 校验和 */
} ColumnarChunk;

/* 条带 (Stripe) */
typedef struct ColumnarStripe
{
    uint32_t           stripe_id;       /* 条带 ID */
    uint32_t           chunk_count;     /* 块数量 */
    ColumnarChunk     *chunks;          /* 数据块数组 */
    uint64_t           total_rows;      /* 总行数 */
    uint64_t           total_size;      /* 总大小 */
    uint64_t           compressed_size; /* 压缩后大小 */
    void              *footer;          /* 条带尾部信息 */
    Timestamp          created_time;    /* 创建时间 */
} ColumnarStripe;

/* 列存储文件 */
typedef struct ColumnarFile
{
    char              *file_path;       /* 文件路径 */
    int                file_descriptor; /* 文件描述符 */
    uint32_t           stripe_count;    /* 条带数量 */
    ColumnarStripe    *stripes;         /* 条带数组 */
    uint64_t           total_rows;      /* 总行数 */
    uint64_t           file_size;       /* 文件大小 */
    void              *schema;          /* 模式信息 */
    void              *metadata;        /* 元数据 */
    void              *footer;          /* 文件尾部信息 */
    bool               is_open;         /* 是否已打开 */
    bool               is_readonly;     /* 是否只读 */
} ColumnarFile;

/* 列存储扫描描述符 */
typedef struct ColumnarScanDesc
{
    EpiphanySmgrRelation *relation;     /* 关系 */
    ColumnarFile      *file;            /* 列存储文件 */
    uint32_t           current_stripe;  /* 当前条带 */
    uint32_t           current_chunk;   /* 当前块 */
    uint32_t           current_row;     /* 当前行 */
    uint32_t           rows_remaining;  /* 剩余行数 */
    void              *projection;      /* 投影列 */
    void              *filter;          /* 过滤条件 */
    void              *batch_buffer;    /* 批处理缓冲区 */
    uint32_t           batch_size;      /* 批大小 */
    bool               vectorized;      /* 是否向量化 */
} ColumnarScanDesc;

/* 列存储插入状态 */
typedef struct ColumnarInsertState
{
    EpiphanySmgrRelation *relation;     /* 关系 */
    ColumnarFile      *file;            /* 列存储文件 */
    ColumnarChunk     *current_chunk;   /* 当前块 */
    void             **column_builders; /* 列构建器数组 */
    uint32_t           rows_in_chunk;   /* 块中行数 */
    uint32_t           max_chunk_size;  /* 最大块大小 */
    bool               auto_flush;      /* 是否自动刷新 */
} ColumnarInsertState;

/* 列存储统计信息 */
typedef struct ColumnarStats
{
    uint64_t           total_files;     /* 总文件数 */
    uint64_t           total_stripes;   /* 总条带数 */
    uint64_t           total_chunks;    /* 总块数 */
    uint64_t           total_rows;      /* 总行数 */
    uint64_t           total_columns;   /* 总列数 */
    uint64_t           compressed_size; /* 压缩后大小 */
    uint64_t           uncompressed_size; /* 未压缩大小 */
    double             compression_ratio; /* 压缩比 */
    uint64_t           scan_operations; /* 扫描操作数 */
    uint64_t           insert_operations; /* 插入操作数 */
    uint64_t           vectorized_scans; /* 向量化扫描数 */
    double             avg_scan_time;   /* 平均扫描时间 */
    double             avg_insert_time; /* 平均插入时间 */
    Timestamp          last_updated;    /* 最后更新时间 */
} ColumnarStats;

/* 向量化批处理 */
typedef struct ColumnarBatch
{
    uint32_t           row_count;       /* 行数 */
    uint32_t           column_count;    /* 列数 */
    void             **column_arrays;   /* 列数组 */
    uint64_t          *column_sizes;    /* 列大小 */
    void              *null_bitmap;     /* NULL 位图 */
    void              *selection_vector; /* 选择向量 */
    uint32_t           selected_count;  /* 选中行数 */
} ColumnarBatch;

/* 函数声明 */

/* 列存储引擎初始化和关闭 */
extern void columnar_engine_init(void);
extern void columnar_engine_shutdown(void);
extern void columnar_engine_configure(const char *config_json);

/* PostgreSQL 兼容的存储管理器接口 */
extern void columnar_smgr_init(void);
extern void columnar_smgr_shutdown(void);
extern void columnar_smgr_open(SMgrRelation reln);
extern void columnar_smgr_close(SMgrRelation reln, ForkNumber forknum);
extern void columnar_smgr_create(SMgrRelation reln, ForkNumber forknum, bool isRedo);
extern bool columnar_smgr_exists(SMgrRelation reln, ForkNumber forknum);
extern void columnar_smgr_unlink(RelFileLocatorBackend rlocator, ForkNumber forknum, bool isRedo);
extern void columnar_smgr_extend(SMgrRelation reln, ForkNumber forknum, BlockNumber blocknum, 
                                const void *buffer, bool skipFsync);
extern void columnar_smgr_zeroextend(SMgrRelation reln, ForkNumber forknum, BlockNumber blocknum, 
                                    int nblocks, bool skipFsync);
extern bool columnar_smgr_prefetch(SMgrRelation reln, ForkNumber forknum, BlockNumber blocknum, 
                                  int nblocks);
extern void columnar_smgr_readv(SMgrRelation reln, ForkNumber forknum, BlockNumber blocknum,
                               void **buffers, BlockNumber nblocks);
extern void columnar_smgr_writev(SMgrRelation reln, ForkNumber forknum, BlockNumber blocknum,
                                const void **buffers, BlockNumber nblocks, bool skipFsync);
extern void columnar_smgr_writeback(SMgrRelation reln, ForkNumber forknum, BlockNumber blocknum, 
                                   BlockNumber nblocks);
extern BlockNumber columnar_smgr_nblocks(SMgrRelation reln, ForkNumber forknum);
extern void columnar_smgr_truncate(SMgrRelation reln, ForkNumber forknum, BlockNumber nblocks);
extern void columnar_smgr_immedsync(SMgrRelation reln, ForkNumber forknum);
extern void columnar_smgr_registersync(SMgrRelation reln, ForkNumber forknum);

/* 列存储文件操作 */
extern ColumnarFile* columnar_file_create(const char *file_path, void *schema);
extern ColumnarFile* columnar_file_open(const char *file_path, bool readonly);
extern void columnar_file_close(ColumnarFile *file);
extern void columnar_file_flush(ColumnarFile *file);
extern void columnar_file_sync(ColumnarFile *file);

/* 列存储扫描操作 */
extern ColumnarScanDesc* columnar_begin_scan(EpiphanySmgrRelation *relation, 
                                            void *projection, void *filter);
extern ColumnarBatch* columnar_scan_next_batch(ColumnarScanDesc *scan);
extern void columnar_end_scan(ColumnarScanDesc *scan);
extern ColumnarScanDesc* columnar_begin_parallel_scan(EpiphanySmgrRelation *relation, 
                                                     int num_workers, int worker_id);

/* 列存储插入操作 */
extern ColumnarInsertState* columnar_begin_insert(EpiphanySmgrRelation *relation);
extern bool columnar_insert_batch(ColumnarInsertState *state, ColumnarBatch *batch);
extern void columnar_flush_chunk(ColumnarInsertState *state);
extern void columnar_end_insert(ColumnarInsertState *state);
extern void columnar_bulk_insert(EpiphanySmgrRelation *relation, void **tuples, int num_tuples);

/* 向量化操作 */
extern ColumnarBatch* columnar_create_batch(uint32_t row_count, uint32_t column_count);
extern void columnar_free_batch(ColumnarBatch *batch);
extern void columnar_batch_add_column(ColumnarBatch *batch, uint32_t column_idx, 
                                     ColumnarDataType type, void *data, uint64_t size);
extern void columnar_batch_set_null_bitmap(ColumnarBatch *batch, void *null_bitmap);
extern void columnar_batch_apply_filter(ColumnarBatch *batch, void *filter);

/* 压缩和编码 */
extern void* columnar_compress_data(void *data, uint64_t size, ColumnarCompressionType type, 
                                   uint64_t *compressed_size);
extern void* columnar_decompress_data(void *compressed_data, uint64_t compressed_size, 
                                     ColumnarCompressionType type, uint64_t *decompressed_size);
extern void* columnar_encode_column(void *data, uint64_t count, ColumnarDataType type, 
                                   ColumnarEncodingType encoding, uint64_t *encoded_size);
extern void* columnar_decode_column(void *encoded_data, uint64_t encoded_size, 
                                   ColumnarDataType type, ColumnarEncodingType encoding, 
                                   uint64_t *decoded_size);

/* 统计信息和优化 */
extern ColumnarStats* columnar_collect_stats(EpiphanySmgrRelation *relation);
extern void columnar_reset_stats(EpiphanySmgrRelation *relation);
extern void columnar_optimize_storage(EpiphanySmgrRelation *relation);
extern void columnar_rebuild_indexes(EpiphanySmgrRelation *relation);

/* 事务支持 */
extern void columnar_begin_transaction(EpiphanySmgrRelation *relation);
extern void columnar_commit_transaction(EpiphanySmgrRelation *relation);
extern void columnar_abort_transaction(EpiphanySmgrRelation *relation);
extern void columnar_prepare_transaction(EpiphanySmgrRelation *relation, const char *gid);

/* 备份和恢复 */
extern void columnar_backup_relation(EpiphanySmgrRelation *relation, const char *backup_path);
extern void columnar_restore_relation(EpiphanySmgrRelation *relation, const char *backup_path);

/* 监控和诊断 */
extern void columnar_get_health_status(EpiphanySmgrRelation *relation, char **status_json);
extern void columnar_get_performance_metrics(EpiphanySmgrRelation *relation, char **metrics_json);

/* 配置管理 */
extern ColumnarEngineConfig* columnar_get_default_config(void);
extern void columnar_set_config(ColumnarEngineConfig *config);
extern ColumnarEngineConfig* columnar_get_config(void);

/* 工具函数 */
extern EpiphanyStorageEngine* create_columnar_storage_engine(void);
extern void register_columnar_storage_engine(void);
extern const char* columnar_data_type_name(ColumnarDataType type);
extern const char* columnar_compression_type_name(ColumnarCompressionType type);
extern const char* columnar_encoding_type_name(ColumnarEncodingType type);

/* 常量定义 */
#define COLUMNAR_FILE_MAGIC         0x434F4C52  /* "COLR" */
#define COLUMNAR_STRIPE_MAGIC       0x53545250  /* "STRP" */
#define COLUMNAR_CHUNK_MAGIC        0x43484E4B  /* "CHNK" */
#define COLUMNAR_DEFAULT_CHUNK_SIZE 10000       /* 1万行 */
#define COLUMNAR_DEFAULT_STRIPE_SIZE 64         /* 64MB */
#define COLUMNAR_DEFAULT_BATCH_SIZE 1024        /* 1024行 */
#define COLUMNAR_MAX_COLUMNS        1000        /* 最大列数 */
#define COLUMNAR_MAX_STRING_LENGTH  65536       /* 最大字符串长度 */

/* 文件格式版本 */
#define COLUMNAR_FORMAT_VERSION_MAJOR 1
#define COLUMNAR_FORMAT_VERSION_MINOR 0

/* 压缩级别 */
#define COLUMNAR_COMPRESSION_LEVEL_FAST    1
#define COLUMNAR_COMPRESSION_LEVEL_DEFAULT 6
#define COLUMNAR_COMPRESSION_LEVEL_BEST    9

#endif /* EPIPHANYDB_COLUMNAR_STORAGE_H */