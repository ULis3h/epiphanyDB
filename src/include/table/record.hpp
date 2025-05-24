namespace epiphany    {
namespace record {


/**
 * @brief 记录ID, 记录的唯一标识. 
 * 
 */
class Rid {


private:
    int logic_number_;
};


/**
 * @brief 记录.
 *        对于Record的设计一个主要的原则就
 *        是要符合集合元素的特性，那就是记录的数据是相互独立的，不会互相影响，
 *        也不会有任何的依赖关系; 两个同类型的记录可以进行赋值、比较、交换等操作,
 * 
 *        记录的分类： 1. 定长记录（int、short、float等） 2. 可变长记录（varchar等）.
 */
class Record {



private:
    Rid rid_; ///< 记录ID.

    /** 
     * 记录数据指针，指向记录存储的数据.
     */
    char* data_;
};



}
}