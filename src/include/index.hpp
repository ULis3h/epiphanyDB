#ifndef EPIPHANY_INDEX_H_
#define EPIPHANY_INDEX_H_

#include "epiphany.h"
#include "epiphany_types.hpp"

typedef struct
{
    char type_;
    size_t offs_;
}EpiphanyIndexField;

/// @brief The meta information of an index.
typedef struct {
    Iid    m_id_;                 ///< The index id.
    size_t m_size_;               ///< The length of index.

    /**: 索引的元信息中不打算再存储数据库名称. */
    // EpiphanyVarType m_database_name_;  ///< The database_name of index.
    // EpiphanyVarType m_table_name_;     ///< The table_name of index.
    const char* table_name_;
    const char* m_index_name_;     ///< The name of index.
    const char* m_type_;         ///< The index type(Hash, Btree, skipList).
    size_t m_field_num_;          ///< The field num of index.
    size_t m_first_field_offs_;

    /** 索引字段列表. */
    EpiphanyIndexField m_fields_[20];
    char index_constraint_;       ///< the constraint of index.
} EpiphanyIndexMeta;

/*----------------------------EpiphanyIndexMeta impl-------------------------------*/

/// @brief determine whether it is Unique.
/// @param  EpiphanyIndexMeta operand.
/// @return success: false/true / failed: no exist.
bool isUnique(EpiphanyIndexMeta*);

/// @brief determine whether it is Primary.
/// @param EpiphanyIndexMeta operand. 
/// @return success: false/true / failed: no exist.
bool isPrimary(EpiphanyIndexMeta*);

/// @brief determine whether it is Foreign.
/// @param  EpiphanyIndexMeta operand. 
/// @return success: false/true / failed: no exist.
bool isForeign(EpiphanyIndexMeta*);

/// @brief get a EpiphanyIndexMeta name.
/// @param  EpiphanyIndexMeta Object.
/// @return success: index name. / failed: EpiphanyNull.
const char* epiphany_index_meta_name(EpiphanyIndexMeta*);

/// @brief get index's fields.
/// @param  EpiphanyIndexMeta Object.
/// @return success: index's fields. / failed: EpiphanyNull.
const char* epiphany_index_meta_field(EpiphanyIndexMeta*);

/// @brief 将一个EpiphanyIndexMeta对象赋值给另一个EpiphanyIndexMeta对象.
/// @param  左操作数.
/// @param  右操作数.
/// @return void.
void epiphany_index_meta_assign(EpiphanyIndexMeta*, EpiphanyIndexMeta*);

/*--------------------------EpiphanyIndexMeta impl end-----------------------------*/

/** Deleted : len | value, len | value, ...*/

/** 应为EpiphanyIndex会保存当前索引的元信息, EpiphanyIndexKey修改为:
 *  New -> | value | value | ...|
 */
typedef void* EpiphanyIndexKey;

/// @brief In Epiphany, the value of the index record is always Rid.
typedef Rid EpiphanyIndexValue;

/// @brief base class of index, index may have multiple implementations.
///        such as Btree, Hash...
class EpiphanyIndex {
public:
    EpiphanyIndex() { };

    EpiphanyIndex(EpiphanyIndexMeta*);

    virtual ~EpiphanyIndex();
public: // virtuals.
    
    /// @brief insert a record into index.
    /// @param  EpiphanyIndexKey key.
    /// @param  EpiphanyIndexValue value.
    /// @return succsss: Epiphany_Ok. / failed: as appropriate.
    virtual EPIPHANY_RET insert(const EpiphanyIndexKey, const EpiphanyIndexValue) = 0;

    /// @brief delete point key record.
    /// @param  EpiphanyIndexKey key.
    /// @return succsss: Epiphany_Ok. / failed: as appropriate.
    virtual EPIPHANY_RET deleteEntry(const EpiphanyIndexKey) = 0;

    /// @brief find key record.
    /// @param  EpiphanyIndexKey key.
    /// @return succsss: Epiphany_Ok. / failed: as appropriate.
    virtual EPIPHANY_RET find(const EpiphanyIndexKey, EpiphanyIndexValue &) = 0;

    /// @brief update a record into index.
    /// @param  EpiphanyIndexKey key.
    /// @param  EpiphanyIndexValue value.
    /// @return succsss: Epiphany_Ok. / failed: as appropriate.
    virtual EPIPHANY_RET update(const EpiphanyIndexKey, const EpiphanyIndexValue) = 0;

    /// @brief 根据索引的元信息来构造一个EpiphanyIndeKey. 所有派生类需要实现这个函数.
    /// @param 原始记录.
    /// @return 
    EpiphanyIndexKey makeIndexKey(void*);

    /// @brief 销毁由makeIndexKey创建的EpiphanyIndexKey. 
    /// @param  EpiphanyIndexKey
    /// @return void.
    void destroyIndexKey(EpiphanyIndexKey key) {
        // EpiphanyAssert(key != EpiphanyNull);
        // char* free_target = (char*)key;
        // epiphanyFree(free_target);
    }

protected:
    
    /// @brief initialize index.
    /// @param const EpiphanyIndexMeta& info
    /// @return succsss: Epiphany_Ok. / failed: as appropriate.
    EPIPHANY_RET init(const EpiphanyIndexMeta&);
protected:
    EpiphanyIndexMeta* m_index_meta_;
};

#endif //! EPIPHANY_INDEX_H_