/*
 * EpiphanyDB - Multi-modal storage engine based on PostgreSQL 17
 * 
 * Main header file for EpiphanyDB core functionality
 */

#ifndef EPIPHANYDB_H
#define EPIPHANYDB_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* Version information */
#define EPIPHANYDB_VERSION_MAJOR 1
#define EPIPHANYDB_VERSION_MINOR 0
#define EPIPHANYDB_VERSION_PATCH 0
#define EPIPHANYDB_VERSION_STRING "1.0.0"

/* Storage engine types */
typedef enum {
    EPIPHANYDB_STORAGE_HEAP = 0,
    EPIPHANYDB_STORAGE_COLUMNAR,
    EPIPHANYDB_STORAGE_VECTOR,
    EPIPHANYDB_STORAGE_TIMESERIES,
    EPIPHANYDB_STORAGE_GRAPH,
    EPIPHANYDB_STORAGE_MAX
} EpiphanyDBStorageType;

/* Forward declarations */
typedef struct EpiphanyDBContext EpiphanyDBContext;
typedef struct EpiphanyDBTable EpiphanyDBTable;
typedef struct EpiphanyDBIndex EpiphanyDBIndex;
typedef struct EpiphanyDBTransaction EpiphanyDBTransaction;

/* Error codes */
typedef enum {
    EPIPHANYDB_SUCCESS = 0,
    EPIPHANYDB_ERROR_INVALID_PARAM = -1,
    EPIPHANYDB_ERROR_NOT_FOUND = -2,
    EPIPHANYDB_ERROR_ALREADY_EXISTS = -3,
    EPIPHANYDB_ERROR_INDEX = -4,
    EPIPHANYDB_ERROR_TRANSACTION = -5,
    EPIPHANYDB_ERROR_STORAGE = -6,
    EPIPHANYDB_ERROR_MEMORY = -7,
    EPIPHANYDB_ERROR_IO = -8,
    EPIPHANYDB_ERROR_UNKNOWN = -99
} EpiphanyDBError;

/* Configuration structure */
typedef struct {
    char *data_directory;
    char *log_directory;
    size_t shared_memory_size;
    int max_connections;
    bool enable_logging;
    bool enable_compression;
    EpiphanyDBStorageType default_storage_type;
} EpiphanyDBConfig;

/* Core API functions */

/**
 * Initialize EpiphanyDB context
 */
EpiphanyDBError epiphanydb_init(EpiphanyDBContext **ctx, const EpiphanyDBConfig *config);

/**
 * Cleanup EpiphanyDB context
 */
void epiphanydb_cleanup(EpiphanyDBContext *ctx);

/**
 * Get EpiphanyDB version string
 */
const char *epiphanydb_version(void);

/**
 * Get error message for error code
 */
const char *epiphanydb_error_message(EpiphanyDBError error);

/**
 * Check if storage engine is available
 */
bool epiphanydb_storage_engine_available(EpiphanyDBStorageType type);

/**
 * Get storage engine name
 */
const char *epiphanydb_storage_engine_name(EpiphanyDBStorageType type);

/* Table management */

/**
 * Create table with specified storage engine
 */
EpiphanyDBError epiphanydb_create_table(EpiphanyDBContext *ctx,
                                       const char *table_name,
                                       EpiphanyDBStorageType storage_type,
                                       const char *schema_definition,
                                       EpiphanyDBTable **table);

/**
 * Open existing table
 */
EpiphanyDBError epiphanydb_open_table(EpiphanyDBContext *ctx,
                                     const char *table_name,
                                     EpiphanyDBTable **table);

/**
 * Close table
 */
void epiphanydb_close_table(EpiphanyDBTable *table);

/**
 * Drop table
 */
EpiphanyDBError epiphanydb_drop_table(EpiphanyDBContext *ctx, const char *table_name);

/* Transaction management */

/**
 * Begin transaction
 */
EpiphanyDBError epiphanydb_begin_transaction(EpiphanyDBContext *ctx,
                                           EpiphanyDBTransaction **txn);

/**
 * Commit transaction
 */
EpiphanyDBError epiphanydb_commit_transaction(EpiphanyDBTransaction *txn);

/**
 * Rollback transaction
 */
EpiphanyDBError epiphanydb_rollback_transaction(EpiphanyDBTransaction *txn);

/* Data operations */

/**
 * Insert data into table
 */
EpiphanyDBError epiphanydb_insert(EpiphanyDBTable *table,
                                 EpiphanyDBTransaction *txn,
                                 const void *data,
                                 size_t data_size);

/**
 * Update data in table
 */
EpiphanyDBError epiphanydb_update(EpiphanyDBTable *table,
                                 EpiphanyDBTransaction *txn,
                                 const void *key,
                                 size_t key_size,
                                 const void *data,
                                 size_t data_size);

/**
 * Delete data from table
 */
EpiphanyDBError epiphanydb_delete(EpiphanyDBTable *table,
                                 EpiphanyDBTransaction *txn,
                                 const void *key,
                                 size_t key_size);

/**
 * Select data from table
 */
EpiphanyDBError epiphanydb_select(EpiphanyDBTable *table,
                                 EpiphanyDBTransaction *txn,
                                 const void *key,
                                 size_t key_size,
                                 void **data,
                                 size_t *data_size);

/* Index management */

/**
 * Create index on table
 */
EpiphanyDBError epiphanydb_create_index(EpiphanyDBTable *table,
                                       const char *index_name,
                                       const char *column_names[],
                                       int num_columns,
                                       EpiphanyDBIndex **index);

/**
 * Drop index
 */
EpiphanyDBError epiphanydb_drop_index(EpiphanyDBIndex *index);

/* Utility functions */

/**
 * Get table statistics
 */
EpiphanyDBError epiphanydb_get_table_stats(EpiphanyDBTable *table,
                                          uint64_t *num_rows,
                                          uint64_t *table_size);

/**
 * Vacuum table (cleanup and optimize)
 */
EpiphanyDBError epiphanydb_vacuum_table(EpiphanyDBTable *table);

/**
 * Analyze table (update statistics)
 */
EpiphanyDBError epiphanydb_analyze_table(EpiphanyDBTable *table);

#endif /* EPIPHANYDB_H */