/*-------------------------------------------------------------------------
 *
 * timeseries_storage.h
 *    EpiphanyDB 时序存储引擎定义 (InfluxDB 兼容)
 *
 * 基于时序数据库设计，提供高效的时间序列数据存储和查询，
 * 支持数据压缩、聚合查询、时间窗口操作和数据保留策略。
 *
 * Copyright (c) 2024, EpiphanyDB Team
 *
 * IDENTIFICATION
 *    src/storage/timeseries_engine/timeseries_storage.h
 *
 *-------------------------------------------------------------------------
 */

#ifndef TIMESERIES_STORAGE_H
#define TIMESERIES_STORAGE_H

#include "../../../include/epiphanydb/storage/storage_manager.h"
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

/* 常量定义 */
#define TIMESERIES_FILE_MAGIC           0x54534442  /* "TSDB" */
#define TIMESERIES_FILE_VERSION         1
#define TIMESERIES_DEFAULT_BATCH_SIZE   1000
#define TIMESERIES_MAX_SERIES_NAME      256
#define TIMESERIES_MAX_TAG_KEY          64
#define TIMESERIES_MAX_TAG_VALUE        256
#define TIMESERIES_MAX_FIELD_NAME       64
#define TIMESERIES_MAX_TAGS_PER_SERIES  32
#define TIMESERIES_MAX_FIELDS_PER_POINT 64
#define TIMESERIES_DEFAULT_RETENTION    (30 * 24 * 3600) /* 30天 */
#define TIMESERIES_SHARD_DURATION       (24 * 3600)      /* 1天 */
#define TIMESERIES_BLOCK_SIZE           (64 * 1024)       /* 64KB */

/* 时序存储引擎配置 */
typedef struct TimeseriesEngineConfig
{
    /* 基本配置 */
    uint32_t default_retention_seconds;     /* 默认数据保留时间 */
    uint32_t shard_duration_seconds;        /* 分片持续时间 */
    uint32_t block_size;                    /* 数据块大小 */
    uint32_t max_series_per_shard;          /* 每个分片最大序列数 */
    uint32_t max_points_per_batch;          /* 每批次最大数据点数 */
    
    /* 压缩配置 */
    bool enable_compression;                /* 启用压缩 */
    const char *compression_algorithm;      /* 压缩算法 */
    int compression_level;                  /* 压缩级别 */
    
    /* 索引配置 */
    bool enable_series_index;               /* 启用序列索引 */
    bool enable_tag_index;                  /* 启用标签索引 */
    bool enable_time_index;                 /* 启用时间索引 */
    
    /* 查询配置 */
    uint32_t max_query_time_range;          /* 最大查询时间范围 */
    uint32_t max_query_series;              /* 最大查询序列数 */
    bool enable_query_cache;                /* 启用查询缓存 */
    
    /* 写入配置 */
    uint32_t write_batch_size;              /* 写入批次大小 */
    uint32_t write_timeout_ms;              /* 写入超时时间 */
    bool enable_write_ahead_log;            /* 启用预写日志 */
    
    /* 维护配置 */
    uint32_t compaction_interval_seconds;   /* 压缩间隔 */
    uint32_t retention_check_interval;      /* 保留检查间隔 */
    bool enable_auto_compaction;            /* 启用自动压缩 */
} TimeseriesEngineConfig;

/* 时序数据类型 */
typedef enum TimeseriesFieldType
{
    TIMESERIES_FIELD_INT64,
    TIMESERIES_FIELD_UINT64,
    TIMESERIES_FIELD_FLOAT64,
    TIMESERIES_FIELD_STRING,
    TIMESERIES_FIELD_BOOL
} TimeseriesFieldType;

/* 时序聚合类型 */
typedef enum TimeseriesAggregationType
{
    TIMESERIES_AGG_NONE,
    TIMESERIES_AGG_COUNT,
    TIMESERIES_AGG_SUM,
    TIMESERIES_AGG_MEAN,
    TIMESERIES_AGG_MIN,
    TIMESERIES_AGG_MAX,
    TIMESERIES_AGG_FIRST,
    TIMESERIES_AGG_LAST,
    TIMESERIES_AGG_STDDEV,
    TIMESERIES_AGG_PERCENTILE
} TimeseriesAggregationType;

/* 时序压缩类型 */
typedef enum TimeseriesCompressionType
{
    TIMESERIES_COMPRESSION_NONE,
    TIMESERIES_COMPRESSION_SNAPPY,
    TIMESERIES_COMPRESSION_GZIP,
    TIMESERIES_COMPRESSION_LZ4,
    TIMESERIES_COMPRESSION_ZSTD,
    TIMESERIES_COMPRESSION_GORILLA  /* Facebook Gorilla 算法 */
} TimeseriesCompressionType;

/* 标签键值对 */
typedef struct TimeseriesTag
{
    char key[TIMESERIES_MAX_TAG_KEY];
    char value[TIMESERIES_MAX_TAG_VALUE];
} TimeseriesTag;

/* 字段值 */
typedef struct TimeseriesFieldValue
{
    char name[TIMESERIES_MAX_FIELD_NAME];
    TimeseriesFieldType type;
    union {
        int64_t int64_val;
        uint64_t uint64_val;
        double float64_val;
        char *string_val;
        bool bool_val;
    } value;
} TimeseriesFieldValue;

/* 时序数据点 */
typedef struct TimeseriesPoint
{
    int64_t timestamp;                      /* 时间戳 (纳秒) */
    char series_name[TIMESERIES_MAX_SERIES_NAME];
    
    /* 标签 */
    TimeseriesTag *tags;
    uint32_t num_tags;
    
    /* 字段 */
    TimeseriesFieldValue *fields;
    uint32_t num_fields;
    
    /* 元数据 */
    uint32_t checksum;
    bool is_deleted;
} TimeseriesPoint;

/* 时序序列 */
typedef struct TimeseriesSeries
{
    char name[TIMESERIES_MAX_SERIES_NAME];
    TimeseriesTag *tags;
    uint32_t num_tags;
    uint64_t series_id;                     /* 序列ID */
    int64_t first_timestamp;                /* 第一个时间戳 */
    int64_t last_timestamp;                 /* 最后一个时间戳 */
    uint64_t point_count;                   /* 数据点数量 */
    uint32_t retention_seconds;             /* 保留时间 */
} TimeseriesSeries;

/* 时序分片 */
typedef struct TimeseriesShard
{
    uint64_t shard_id;
    int64_t start_time;                     /* 分片开始时间 */
    int64_t end_time;                       /* 分片结束时间 */
    char *shard_path;                       /* 分片文件路径 */
    int file_descriptor;                    /* 文件描述符 */
    bool is_open;
    bool is_readonly;
    
    /* 统计信息 */
    uint64_t series_count;                  /* 序列数量 */
    uint64_t point_count;                   /* 数据点数量 */
    uint64_t file_size;                     /* 文件大小 */
    int64_t created_time;                   /* 创建时间 */
    int64_t last_modified;                  /* 最后修改时间 */
    
    /* 索引 */
    void *series_index;                     /* 序列索引 */
    void *tag_index;                        /* 标签索引 */
    void *time_index;                       /* 时间索引 */
} TimeseriesShard;

/* 时序数据块 */
typedef struct TimeseriesBlock
{
    uint64_t block_id;
    uint64_t shard_id;
    int64_t start_time;
    int64_t end_time;
    uint32_t block_size;
    uint32_t compressed_size;
    TimeseriesCompressionType compression_type;
    
    /* 数据 */
    void *data;
    void *compressed_data;
    bool is_compressed;
    
    /* 统计信息 */
    uint32_t point_count;
    uint32_t series_count;
    uint32_t checksum;
} TimeseriesBlock;

/* 时序查询参数 (扩展版本) */
typedef struct TimeseriesQueryParamsExt
{
    /* 时间范围 */
    int64_t start_time;
    int64_t end_time;
    
    /* 序列选择 */
    char **series_names;
    uint32_t num_series;
    
    /* 标签过滤 */
    TimeseriesTag *tag_filters;
    uint32_t num_tag_filters;
    
    /* 字段选择 */
    char **field_names;
    uint32_t num_fields;
    
    /* 聚合 */
    TimeseriesAggregationType aggregation;
    int64_t group_by_time;                  /* 时间分组间隔 */
    char **group_by_tags;                   /* 标签分组 */
    uint32_t num_group_by_tags;
    
    /* 限制 */
    uint32_t limit;
    uint32_t offset;
    bool order_desc;                        /* 降序排列 */
    
    /* 填充 */
    bool fill_null;
    TimeseriesFieldValue fill_value;
} TimeseriesQueryParamsExt;

/* 时序查询结果 */
typedef struct TimeseriesQueryResult
{
    TimeseriesPoint *points;
    uint32_t num_points;
    uint32_t total_points;                  /* 总数据点数 */
    bool has_more;                          /* 是否有更多数据 */
    char *error_message;
} TimeseriesQueryResult;

/* 时序扫描描述符 */
typedef struct TimeseriesScanDesc
{
    EpiphanySmgrRelation *relation;
    TimeseriesQueryParamsExt params;
    
    /* 扫描状态 */
    uint32_t current_shard;
    uint32_t current_block;
    uint32_t current_point;
    bool is_finished;
    
    /* 结果缓存 */
    TimeseriesPoint *result_buffer;
    uint32_t buffer_size;
    uint32_t buffer_pos;
    
    /* 统计信息 */
    uint64_t scanned_points;
    uint64_t filtered_points;
    uint64_t returned_points;
} TimeseriesScanDesc;

/* 时序写入状态 */
typedef struct TimeseriesWriteState
{
    EpiphanySmgrRelation *relation;
    
    /* 批次缓存 */
    TimeseriesPoint *batch_buffer;
    uint32_t batch_size;
    uint32_t batch_pos;
    uint32_t max_batch_size;
    
    /* 写入配置 */
    bool auto_flush;
    bool auto_create_series;
    uint32_t flush_interval_ms;
    
    /* 统计信息 */
    uint64_t written_points;
    uint64_t written_series;
    uint64_t write_errors;
    int64_t last_write_time;
} TimeseriesWriteState;

/* 时序统计信息 */
typedef struct TimeseriesStats
{
    /* 存储统计 */
    uint64_t total_shards;
    uint64_t total_series;
    uint64_t total_points;
    uint64_t total_size_bytes;
    
    /* 操作统计 */
    uint64_t write_operations;
    uint64_t read_operations;
    uint64_t query_operations;
    uint64_t compaction_operations;
    
    /* 性能统计 */
    double avg_write_latency_ms;
    double avg_read_latency_ms;
    double avg_query_latency_ms;
    double compression_ratio;
    
    /* 缓存统计 */
    uint64_t cache_hits;
    uint64_t cache_misses;
    double cache_hit_ratio;
    
    /* 错误统计 */
    uint64_t write_errors;
    uint64_t read_errors;
    uint64_t query_errors;
    
    /* 时间戳 */
    int64_t last_updated;
    int64_t start_time;
} TimeseriesStats;

/*
 * 时序存储引擎初始化
 */
extern void timeseries_engine_init(void);
extern void timeseries_engine_shutdown(void);
extern void timeseries_engine_configure(const char *config_json);

/*
 * PostgreSQL 兼容的存储管理器接口
 */
extern void timeseries_smgr_init(void);
extern void timeseries_smgr_shutdown(void);
extern void timeseries_smgr_open(SMgrRelation reln);
extern void timeseries_smgr_close(SMgrRelation reln, ForkNumber forknum);
extern void timeseries_smgr_create(SMgrRelation reln, ForkNumber forknum, bool isRedo);
extern bool timeseries_smgr_exists(SMgrRelation reln, ForkNumber forknum);
extern void timeseries_smgr_unlink(RelFileLocatorBackend rlocator, ForkNumber forknum, bool isRedo);
extern void timeseries_smgr_extend(SMgrRelation reln, ForkNumber forknum, BlockNumber blocknum, 
                                  const void *buffer, bool skipFsync);
extern void timeseries_smgr_zeroextend(SMgrRelation reln, ForkNumber forknum, BlockNumber blocknum, 
                                      int nblocks, bool skipFsync);
extern bool timeseries_smgr_prefetch(SMgrRelation reln, ForkNumber forknum, BlockNumber blocknum, 
                                     int nblocks);
extern void timeseries_smgr_readv(SMgrRelation reln, ForkNumber forknum, BlockNumber blocknum,
                                 void **buffers, BlockNumber nblocks);
extern void timeseries_smgr_writev(SMgrRelation reln, ForkNumber forknum, BlockNumber blocknum,
                                  const void **buffers, BlockNumber nblocks, bool skipFsync);
extern void timeseries_smgr_writeback(SMgrRelation reln, ForkNumber forknum, BlockNumber blocknum, 
                                     BlockNumber nblocks);
extern BlockNumber timeseries_smgr_nblocks(SMgrRelation reln, ForkNumber forknum);
extern void timeseries_smgr_truncate(SMgrRelation reln, ForkNumber forknum, BlockNumber nblocks);
extern void timeseries_smgr_immedsync(SMgrRelation reln, ForkNumber forknum);
extern void timeseries_smgr_registersync(SMgrRelation reln, ForkNumber forknum);

/*
 * 时序分片操作
 */
extern TimeseriesShard* timeseries_create_shard(const char *shard_path, int64_t start_time, int64_t end_time);
extern TimeseriesShard* timeseries_open_shard(const char *shard_path, bool readonly);
extern void timeseries_close_shard(TimeseriesShard *shard);
extern void timeseries_flush_shard(TimeseriesShard *shard);
extern void timeseries_compact_shard(TimeseriesShard *shard);

/*
 * 时序数据写入
 */
extern TimeseriesWriteState* timeseries_begin_write(EpiphanySmgrRelation *relation);
extern bool timeseries_write_point(TimeseriesWriteState *state, const TimeseriesPoint *point);
extern bool timeseries_write_batch(TimeseriesWriteState *state, const TimeseriesPoint *points, uint32_t num_points);
extern void timeseries_flush_write(TimeseriesWriteState *state);
extern void timeseries_end_write(TimeseriesWriteState *state);

/*
 * 时序数据查询
 */
extern TimeseriesScanDesc* timeseries_begin_scan(EpiphanySmgrRelation *relation, const TimeseriesQueryParamsExt *params);
extern TimeseriesPoint* timeseries_scan_next(TimeseriesScanDesc *scan);
extern void timeseries_end_scan(TimeseriesScanDesc *scan);
extern TimeseriesQueryResult* timeseries_query(EpiphanySmgrRelation *relation, const TimeseriesQueryParamsExt *params);
extern void timeseries_free_query_result(TimeseriesQueryResult *result);

/*
 * 时序序列管理
 */
extern bool timeseries_create_series(EpiphanySmgrRelation *relation, const TimeseriesSeries *series);
extern TimeseriesSeries* timeseries_get_series(EpiphanySmgrRelation *relation, const char *series_name, 
                                               const TimeseriesTag *tags, uint32_t num_tags);
extern bool timeseries_drop_series(EpiphanySmgrRelation *relation, const char *series_name);
extern TimeseriesSeries** timeseries_list_series(EpiphanySmgrRelation *relation, uint32_t *num_series);

/*
 * 时序数据压缩
 */
extern bool timeseries_compress_block(TimeseriesBlock *block, TimeseriesCompressionType compression_type);
extern bool timeseries_decompress_block(TimeseriesBlock *block);
extern double timeseries_get_compression_ratio(const TimeseriesBlock *block);

/*
 * 时序数据聚合
 */
extern TimeseriesQueryResult* timeseries_aggregate(EpiphanySmgrRelation *relation, 
                                                  const TimeseriesQueryParamsExt *params);
extern TimeseriesFieldValue timeseries_compute_aggregation(const TimeseriesPoint *points, uint32_t num_points,
                                                          const char *field_name, TimeseriesAggregationType agg_type);

/*
 * 时序索引操作
 */
extern bool timeseries_create_series_index(TimeseriesShard *shard);
extern bool timeseries_create_tag_index(TimeseriesShard *shard);
extern bool timeseries_create_time_index(TimeseriesShard *shard);
extern void timeseries_rebuild_indexes(TimeseriesShard *shard);

/*
 * 时序数据维护
 */
extern void timeseries_compact_data(EpiphanySmgrRelation *relation);
extern void timeseries_apply_retention_policy(EpiphanySmgrRelation *relation);
extern void timeseries_vacuum_deleted_data(EpiphanySmgrRelation *relation);
extern void timeseries_rebalance_shards(EpiphanySmgrRelation *relation);

/*
 * 统计信息和优化
 */
extern TimeseriesStats* timeseries_collect_stats(EpiphanySmgrRelation *relation);
extern void timeseries_reset_stats(EpiphanySmgrRelation *relation);
extern void timeseries_optimize_storage(EpiphanySmgrRelation *relation);
extern void timeseries_analyze_workload(EpiphanySmgrRelation *relation);

/*
 * 事务支持
 */
extern void timeseries_begin_transaction(EpiphanySmgrRelation *relation);
extern void timeseries_commit_transaction(EpiphanySmgrRelation *relation);
extern void timeseries_abort_transaction(EpiphanySmgrRelation *relation);
extern void timeseries_prepare_transaction(EpiphanySmgrRelation *relation, const char *gid);

/*
 * 备份和恢复
 */
extern void timeseries_backup_relation(EpiphanySmgrRelation *relation, const char *backup_path);
extern void timeseries_restore_relation(EpiphanySmgrRelation *relation, const char *backup_path);
extern void timeseries_export_data(EpiphanySmgrRelation *relation, const char *export_path, 
                                  const TimeseriesQueryParamsExt *params);
extern void timeseries_import_data(EpiphanySmgrRelation *relation, const char *import_path);

/*
 * 监控和诊断
 */
extern void timeseries_get_health_status(EpiphanySmgrRelation *relation, char **status_json);
extern void timeseries_get_performance_metrics(EpiphanySmgrRelation *relation, char **metrics_json);
extern void timeseries_diagnose_performance(EpiphanySmgrRelation *relation, char **diagnosis_json);

/*
 * 配置管理
 */
extern TimeseriesEngineConfig* timeseries_get_default_config(void);
extern void timeseries_set_config(TimeseriesEngineConfig *config);
extern TimeseriesEngineConfig* timeseries_get_config(void);

/*
 * 工具函数
 */
extern EpiphanyStorageEngine* create_timeseries_storage_engine(void);
extern void register_timeseries_storage_engine(void);
extern const char* timeseries_field_type_name(TimeseriesFieldType type);
extern const char* timeseries_aggregation_type_name(TimeseriesAggregationType type);
extern const char* timeseries_compression_type_name(TimeseriesCompressionType type);
extern size_t timeseries_field_type_size(TimeseriesFieldType type);
extern uint32_t timeseries_calculate_point_checksum(const TimeseriesPoint *point);
extern bool timeseries_validate_point(const TimeseriesPoint *point);
extern int64_t timeseries_parse_timestamp(const char *timestamp_str);
extern char* timeseries_format_timestamp(int64_t timestamp);

#endif /* TIMESERIES_STORAGE_H */