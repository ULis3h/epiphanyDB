/*-------------------------------------------------------------------------
 *
 * heap_storage.h
 *    EpiphanyDB 堆存储引擎 (PostgreSQL 兼容)
 *
 * 基于 PostgreSQL 17 的堆存储实现，提供与原生 PostgreSQL 完全兼容的
 * 行存储功能，同时扩展支持 EpiphanyDB 的多模态查询接口。
 *
 * Copyright (c) 2024, EpiphanyDB Team
 *
 * IDENTIFICATION
 *    src/storage/heap_engine/heap_storage.h
 *
 *-------------------------------------------------------------------------
 */
#ifndef EPIPHANYDB_HEAP_STORAGE_H
#define EPIPHANYDB_HEAP_STORAGE_H

#include "../../../include/epiphanydb/storage/storage_manager.h"

/* 堆存储引擎配置 */
typedef struct HeapEngineConfig
{
    int         page_size;              /* 页面大小 (默认 8KB) */
    int         max_tuple_size;         /* 最大元组大小 */
    bool        enable_compression;     /* 是否启用压缩 */
    bool        enable_checksums;       /* 是否启用校验和 */
    int         fillfactor;            /* 填充因子 (0-100) */
    int         vacuum_threshold;       /* 垃圾回收阈值 */
    bool        enable_hot_updates;     /* 是否启用 HOT 更新 */
    bool        enable_parallel_scan;   /* 是否启用并行扫描 */
} HeapEngineConfig;

/* 堆页面头部 */
typedef struct HeapPageHeader
{
    uint32_t    page_magic;            /* 页面魔数 */
    uint16_t    page_version;          /* 页面版本 */
    uint16_t    page_flags;            /* 页面标志 */
    uint32_t    page_size;             /* 页面大小 */
    uint16_t    tuple_count;           /* 元组数量 */
    uint16_t    free_space_start;      /* 空闲空间起始位置 */
    uint16_t    free_space_end;        /* 空闲空间结束位置 */
    uint32_t    checksum;              /* 页面校验和 */
    uint64_t    lsn;                   /* 日志序列号 */
    uint32_t    transaction_id;        /* 事务 ID */
} HeapPageHeader;

/* 堆元组头部 */
typedef struct HeapTupleHeader
{
    uint32_t    tuple_magic;           /* 元组魔数 */
    uint16_t    tuple_length;          /* 元组长度 */
    uint16_t    tuple_flags;           /* 元组标志 */
    uint32_t    xmin;                  /* 插入事务 ID */
    uint32_t    xmax;                  /* 删除事务 ID */
    uint32_t    cmin;                  /* 插入命令 ID */
    uint32_t    cmax;                  /* 删除命令 ID */
    uint16_t    natts;                 /* 属性数量 */
    uint16_t    null_bitmap_offset;    /* NULL 位图偏移 */
} HeapTupleHeader;

/* 堆元组 */
typedef struct HeapTuple
{
    HeapTupleHeader *header;           /* 元组头部 */
    void           *data;              /* 元组数据 */
    uint16_t        data_length;       /* 数据长度 */
    BlockNumber     block_number;      /* 块号 */
    uint16_t        offset_number;     /* 偏移号 */
} HeapTuple;

/* 堆扫描描述符 */
typedef struct HeapScanDesc
{
    EpiphanySmgrRelation *relation;    /* 关系 */
    BlockNumber     start_block;       /* 起始块 */
    BlockNumber     end_block;         /* 结束块 */
    BlockNumber     current_block;     /* 当前块 */
    uint16_t        current_offset;    /* 当前偏移 */
    bool            forward_scan;      /* 是否正向扫描 */
    bool            parallel_scan;     /* 是否并行扫描 */
    void           *scan_key;          /* 扫描键 */
    void           *page_buffer;       /* 页面缓冲区 */
} HeapScanDesc;

/* 堆插入状态 */
typedef struct HeapInsertState
{
    EpiphanySmgrRelation *relation;    /* 关系 */
    BlockNumber     target_block;      /* 目标块 */
    void           *page_buffer;       /* 页面缓冲区 */
    bool            use_wal;           /* 是否使用 WAL */
    bool            use_fsm;           /* 是否使用 FSM */
} HeapInsertState;

/* 堆更新状态 */
typedef struct HeapUpdateState
{
    EpiphanySmgrRelation *relation;    /* 关系 */
    HeapTuple      *old_tuple;         /* 旧元组 */
    HeapTuple      *new_tuple;         /* 新元组 */
    bool            hot_update;        /* 是否 HOT 更新 */
    bool            use_wal;           /* 是否使用 WAL */
} HeapUpdateState;

/* 堆统计信息 */
typedef struct HeapStats
{
    uint64_t        total_pages;       /* 总页面数 */
    uint64_t        total_tuples;      /* 总元组数 */
    uint64_t        live_tuples;       /* 活跃元组数 */
    uint64_t        dead_tuples;       /* 死元组数 */
    uint64_t        free_space;        /* 空闲空间 */
    uint64_t        avg_tuple_size;    /* 平均元组大小 */
    double          table_bloat;       /* 表膨胀率 */
    uint64_t        seq_scans;         /* 顺序扫描次数 */
    uint64_t        seq_tuples;        /* 顺序扫描元组数 */
    uint64_t        idx_scans;         /* 索引扫描次数 */
    uint64_t        idx_tuples;        /* 索引扫描元组数 */
    uint64_t        inserts;           /* 插入次数 */
    uint64_t        updates;           /* 更新次数 */
    uint64_t        deletes;           /* 删除次数 */
    uint64_t        hot_updates;       /* HOT 更新次数 */
} HeapStats;

/* 函数声明 */

/* 堆存储引擎初始化和关闭 */
extern void heap_engine_init(void);
extern void heap_engine_shutdown(void);
extern void heap_engine_configure(const char *config_json);

/* PostgreSQL 兼容的存储管理器接口 */
extern void heap_smgr_init(void);
extern void heap_smgr_shutdown(void);
extern void heap_smgr_open(SMgrRelation reln);
extern void heap_smgr_close(SMgrRelation reln, ForkNumber forknum);
extern void heap_smgr_create(SMgrRelation reln, ForkNumber forknum, bool isRedo);
extern bool heap_smgr_exists(SMgrRelation reln, ForkNumber forknum);
extern void heap_smgr_unlink(RelFileLocatorBackend rlocator, ForkNumber forknum, bool isRedo);
extern void heap_smgr_extend(SMgrRelation reln, ForkNumber forknum, BlockNumber blocknum, 
                            const void *buffer, bool skipFsync);
extern void heap_smgr_zeroextend(SMgrRelation reln, ForkNumber forknum, BlockNumber blocknum, 
                                int nblocks, bool skipFsync);
extern bool heap_smgr_prefetch(SMgrRelation reln, ForkNumber forknum, BlockNumber blocknum, 
                              int nblocks);
extern void heap_smgr_readv(SMgrRelation reln, ForkNumber forknum, BlockNumber blocknum,
                           void **buffers, BlockNumber nblocks);
extern void heap_smgr_writev(SMgrRelation reln, ForkNumber forknum, BlockNumber blocknum,
                            const void **buffers, BlockNumber nblocks, bool skipFsync);
extern void heap_smgr_writeback(SMgrRelation reln, ForkNumber forknum, BlockNumber blocknum, 
                               BlockNumber nblocks);
extern BlockNumber heap_smgr_nblocks(SMgrRelation reln, ForkNumber forknum);
extern void heap_smgr_truncate(SMgrRelation reln, ForkNumber forknum, BlockNumber nblocks);
extern void heap_smgr_immedsync(SMgrRelation reln, ForkNumber forknum);
extern void heap_smgr_registersync(SMgrRelation reln, ForkNumber forknum);

/* 堆元组操作 */
extern HeapTuple* heap_form_tuple(int natts, void **values, bool *nulls);
extern void heap_deform_tuple(HeapTuple *tuple, int natts, void **values, bool *nulls);
extern HeapTuple* heap_copy_tuple(HeapTuple *tuple);
extern void heap_free_tuple(HeapTuple *tuple);
extern bool heap_tuple_is_valid(HeapTuple *tuple);
extern uint32_t heap_tuple_size(HeapTuple *tuple);

/* 堆页面操作 */
extern void heap_init_page(void *page, uint32_t page_size);
extern bool heap_page_is_valid(void *page);
extern uint16_t heap_page_get_free_space(void *page);
extern bool heap_page_can_fit_tuple(void *page, uint16_t tuple_size);
extern uint16_t heap_page_add_tuple(void *page, HeapTuple *tuple);
extern HeapTuple* heap_page_get_tuple(void *page, uint16_t offset);
extern void heap_page_delete_tuple(void *page, uint16_t offset);
extern void heap_page_compact(void *page);
extern uint32_t heap_page_checksum(void *page);

/* 堆扫描操作 */
extern HeapScanDesc* heap_begin_scan(EpiphanySmgrRelation *relation, 
                                    BlockNumber start_block, BlockNumber end_block);
extern HeapTuple* heap_scan_next(HeapScanDesc *scan);
extern void heap_end_scan(HeapScanDesc *scan);
extern HeapScanDesc* heap_begin_parallel_scan(EpiphanySmgrRelation *relation, 
                                             int num_workers, int worker_id);

/* 堆插入操作 */
extern HeapInsertState* heap_begin_insert(EpiphanySmgrRelation *relation);
extern bool heap_insert_tuple(HeapInsertState *state, HeapTuple *tuple, 
                             BlockNumber *block, uint16_t *offset);
extern void heap_end_insert(HeapInsertState *state);
extern void heap_bulk_insert(EpiphanySmgrRelation *relation, HeapTuple **tuples, int num_tuples);

/* 堆更新操作 */
extern HeapUpdateState* heap_begin_update(EpiphanySmgrRelation *relation);
extern bool heap_update_tuple(HeapUpdateState *state, BlockNumber block, uint16_t offset, 
                             HeapTuple *new_tuple);
extern void heap_end_update(HeapUpdateState *state);

/* 堆删除操作 */
extern bool heap_delete_tuple(EpiphanySmgrRelation *relation, BlockNumber block, uint16_t offset);
extern void heap_bulk_delete(EpiphanySmgrRelation *relation, void **keys, int num_keys);

/* 堆维护操作 */
extern void heap_vacuum(EpiphanySmgrRelation *relation, bool full_vacuum);
extern void heap_analyze(EpiphanySmgrRelation *relation);
extern void heap_reindex(EpiphanySmgrRelation *relation);
extern void heap_cluster(EpiphanySmgrRelation *relation, Oid index_oid);

/* 堆统计信息 */
extern HeapStats* heap_collect_stats(EpiphanySmgrRelation *relation);
extern void heap_reset_stats(EpiphanySmgrRelation *relation);
extern void heap_update_stats(EpiphanySmgrRelation *relation, HeapStats *stats);

/* 堆事务支持 */
extern void heap_begin_transaction(EpiphanySmgrRelation *relation);
extern void heap_commit_transaction(EpiphanySmgrRelation *relation);
extern void heap_abort_transaction(EpiphanySmgrRelation *relation);
extern void heap_prepare_transaction(EpiphanySmgrRelation *relation, const char *gid);

/* 堆备份和恢复 */
extern void heap_backup_relation(EpiphanySmgrRelation *relation, const char *backup_path);
extern void heap_restore_relation(EpiphanySmgrRelation *relation, const char *backup_path);

/* 堆监控和诊断 */
extern void heap_get_health_status(EpiphanySmgrRelation *relation, char **status_json);
extern void heap_get_performance_metrics(EpiphanySmgrRelation *relation, char **metrics_json);

/* 堆配置管理 */
extern HeapEngineConfig* heap_get_default_config(void);
extern void heap_set_config(HeapEngineConfig *config);
extern HeapEngineConfig* heap_get_config(void);

/* 工具函数 */
extern EpiphanyStorageEngine* create_heap_storage_engine(void);
extern void register_heap_storage_engine(void);

/* 常量定义 */
#define HEAP_PAGE_MAGIC         0x48454150  /* "HEAP" */
#define HEAP_TUPLE_MAGIC        0x54555045  /* "TUPE" */
#define HEAP_DEFAULT_PAGE_SIZE  8192        /* 8KB */
#define HEAP_MAX_TUPLE_SIZE     2048        /* 2KB */
#define HEAP_DEFAULT_FILLFACTOR 90          /* 90% */
#define HEAP_MIN_FREE_SPACE     64          /* 64 字节 */

/* 页面标志 */
#define HEAP_PAGE_COMPRESSED    0x0001
#define HEAP_PAGE_CHECKSUMMED   0x0002
#define HEAP_PAGE_FULL          0x0004
#define HEAP_PAGE_DIRTY         0x0008

/* 元组标志 */
#define HEAP_TUPLE_LIVE         0x0001
#define HEAP_TUPLE_DEAD         0x0002
#define HEAP_TUPLE_HOT_UPDATED  0x0004
#define HEAP_TUPLE_COMPRESSED   0x0008

#endif /* EPIPHANYDB_HEAP_STORAGE_H */