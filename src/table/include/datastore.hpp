#ifndef EPIPHANY_DATASTORE_HPP_
#define EPIPHANY_DATASTORE_HPP_

#include <cstddef>

#include "record.hpp"
#include "epiphany_types.hpp"

namespace epiphany    {
namespace table {

/**
 * @brief 数据存储方式抽象类, 针对不同的应用场景, 存储引擎实现了如下文件组织方式:
 *  1. 堆表.  (default)
 *  2. 顺序表.
 *  3. B+树.
 *  4. 哈希表.
 */
class DataStore {
public:
    virtual ~DataStore();

    virtual Rid insert(const Record& record) = 0;
};

} /// table.
} /// epiphany.

#endif //! EPIPHANY_DATASTORE_HPP_