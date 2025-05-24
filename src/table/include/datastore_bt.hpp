#ifndef EPIPHANY_DATASTORE_BTREE_HPP_
#define EPIPHANY_DATASTORE_BTREE_HPP_

#include <cstddef>

#include "page.hpp"
#include "epiphany_types.hpp"
#include "record.hpp"
#include "datastore.hpp"
#include "mutex.hpp"

using epiphany::os::EpiphanyMutex;


namespace epiphany    {
namespace table {

class DataStoreBtree : public DataStore {
public:
    DataStoreBtree();
    ~DataStoreBtree();

    enum {
        BTREE_COL_FIX = 1,
        BTREE_COL_VAR = 2,
        BTREE_ROW = 3,
    } type_;

    enum DsBtreeFlags {
        BULK = 0x01,
        EXCLUSIVE = 0x02,
        NO_LOCK = 0x04,
        OPEN = 0x08,
        SALVAGE = 0x10,
        UPGRADE = 0x20,
        VERIFY = 0x40,
    };

private:
    EpiphanyMutex rwlock_; ///< 读写锁.
    size_t refcnt_; ///< 引用计数.

    const char* name_;
    const char* filename_;
    const char* config_;

    const char* key_format_;
    const char* key_plan_;
    const char* idxkey_format_;
    const char* value_format_;
    const char* value_plan_;

    epiphany_uint8 bitcnt_; ///< 固定长度字段大小.

    epiphany_uint32 key_gap_; ///< row store prefix key gap.
    epiphany_uint32 alloc_size_;
    epiphany_uint32 max_intl_page_;
    epiphany_uint32 max_intl_item_;
    epiphany_uint32 max_leaf_page_;
    epiphany_uint32 max_leaf_item_;

    void* huffman_key_;
    void* huffman_value_;

    Pgno last_page_;
    Rid last_rec_id_;

    Pgno root_page_;

    int root_update_;

    void* block_;
    size_t block_header_len_;

    epiphany_uint32 flag_;
};

} /// table.
} /// epiphany.

#endif //! EPIPHANY_DATASTORE_BTREE_HPP_