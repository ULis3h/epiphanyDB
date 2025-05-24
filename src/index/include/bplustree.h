#ifndef AGILORE_BTREE_H
#define AGILORE_BTREE_H

#include <memory>
#include <queue> /*Temporarily using STL queue*/
#include <list>
#include <cstring>
#include <cassert>
#include <cstdio>
#include <algorithm>
#include <sstream>
#include <stack>
#include <vector>

/** Graphical debugging */
#ifdef GRAPHICAL_DEBUG
#include <emscripten.h>
#endif //! GRAPHICAL_DEBUG

#include "../../../include/epiphany.h"
// #include "../../src/cache/include/EpiphanyPager.h"
#include "../../../include/util/EpiphanyComparator.h"
#include "util/EpiphanyString.h"
#include "os/time/EpiphanyTime.h"

#include "cache/page.h"

/** 
 * B+ tree depends on the cache component.
 */
class Pager;

/// @brief 
namespace BtreeSpace {

/* Memory allocation method, 0 for getting memory from Mem manager, 1 for allocation from heap. */
#define AllocType 0    

/** Default size of memory pool. */
#define MemPoolSize 1024 * 1024 * 16

template<bool>
struct CompAssert{
};
template <>
struct CompAssert<true>{
};
#define Comp_Assert(expr) do{ (CompAssert<(expr) != 0>()); } while(0)

/** Whether the B+ tree node is full. */
#ifndef BptreeFull
#define BptreeFull(n, degree) (n >= ((degree) - 1))
#endif //! BptreeFull

/// A default comparison function used as the comparison function during lookup. 
/// If the user does not specify one explicitly, 
/// this function is called as the comparison function.
template<typename Type>
static int DefaultComparator(Type x, Type y){
    return x - y;
}

#ifndef PARAMS_TYPES
#define PARAMS_TYPES\
    typedef Params params_type;\
    typedef typename Params::key_type key_type;\
    typedef typename Params::data_type data_type;\
    typedef typename Params::pointer_type pointer_type;
#endif //! PARAMS_TYPES

/// SFINAE.
#define HasMethod(MethodName, return_value)\
template <typename T>\
class Has##MethodName {\
private:\
typedef char yesType[1];\
typedef char noType[2];\
template <typename U, U> struct ReallyHas;\
template <typename C> static yesType&\
Test(ReallyHas<return_value (C::*)() const, &C::MethodName>*);\
template <typename C> static noType&\
Test(...);\
public:\
    enum { value = sizeof(Test<T>(0)) == sizeof(yesType) }; \
}

/// prev define for use.
template <typename Params>
class EpiphanyBplusTree;
template <typename Params>
class BptBaseNode;
template <typename Params>
class BptLeafNode;
template <typename Params>
class BptInternalNode;
template <typename T>
struct HasSizeMethod;
template <typename T>
struct HasrecSize;
template <typename T>
struct HasrecBuf;

template <typename Key,
          typename Value,
          typename Ptr,
          int TargetNodeSize = TZDB_FILE_PAGE_SIZE,
          bool ValueHas = HasrecSize<Value>::value && HasrecBuf<Value>::value>
struct bplustree_params;

enum BptRet {
    /** Set initial value to 600 for easy conversion to EpiphanyRet type. */
    Btree_Ok = 600,

    /** B+ tree initialization not completed. */
    Btree_NotInitialized,

    /** Operating on an invalid page. */
    Btree_OpInvalidPage,

    /** Null pointer operation. */
    Btree_OpNullPtr,
    
    /** Array access overflow. */
    Btree_ReadArrayOfl,

    /** Inserting an already existing key. */
    Btree_InsertExistKey,    

    /** Specified key not found. */
    Btree_SearchNotFoundKey,

    /** Key not found during deletion. */
    Btree_DeleteNotFoundKey,

    /** Processing incorrect page number. */
    Btree_OpErrorPgno,

    /** B+ tree structure error. */
    Btree_StructError,

    /** Leaf node imbalance. */
    Btree_LeafUnbalanced, 
};

enum BptSearchOp 
{
    BptOp_Lt = 1, ///< Less than.
    BptOp_Le = 2, ///< Less than or equal to.
    BptOp_Eq = 3, ///< Equal to.
    BptOp_Ge = 4, ///< Greater than or equal to.
    BptOp_Gt = 5, ///< Greater than.
};

// rebuild.
/*-------------------------BptInternalPage define-----------------------------*/
using epiphany::IPage;
using epiphany::PageFactory;

struct _BptInternalPage
{
    size_t m_key_num_;
};

class BptInternalPage : public IPage 
{
public: // overwrites.
    /// @brief 序列化函数.
    /// @return 
    virtual void serializable(void* buf);

    /// @brief 反序列化函数.
    /// @return 
    virtual void deserializable(void* buf);
public:
    /// @brief Convert BptInternalPage to BptInternalNode.
    /// @param  
    // void convertInternalNode(BptBaseNode*);

private:
    _BptInternalPage m_internal_data_;
};
/*-----------------------BptInternalPage define End---------------------------*/

/*-----------------------BptInternalPage define Impl--------------------------*/
inline void 
BptInternalPage::serializable(void* buf)
{
    EpiphanyAssert(buf);
    epiphany_int8* buf_base = (epiphany_int8*)buf;
    size_t offset = 0;

    ndbMemCpy(buf_base, &(this->m_data_.m_type_), sizeof(this->m_data_.m_type_));
    offset += sizeof(this->m_data_.m_type_);

    ndbMemCpy(buf_base + offset, &(this->m_data_.m_flag_), 
              sizeof(this->m_data_.m_flag_));
    offset += sizeof(this->m_data_.m_flag_);

    ndbMemCpy(buf_base + offset, &(this->m_data_.m_state_),
              sizeof(this->m_data_.m_state_));
    offset += sizeof(this->m_data_.m_state_);

    ndbMemCpy(buf_base + offset, &(this->m_data_.m_dirty_), 
              sizeof(this->m_data_.m_dirty_));
    offset += sizeof(this->m_data_.m_dirty_);
}

inline void 
BptInternalPage::deserializable(void* buf)
{
}

// inline void 
// BptInternalPage::convertInternalNode(BptBaseNode* node)
// {
// }
/*---------------------BptInternalPage define Impl End------------------------*/



/*----------------------BptInternalPageBuilder define-------------------------*/
class BptInternalPageBuilder : public PageFactory
{
public:
    virtual IPage* createPage();
};
/*--------------------BptInternalPageBuilder define End-----------------------*/

/*-----------------------BptInternalPageBuilder Impl--------------------------*/
inline IPage* BptInternalPageBuilder::createPage()
{
    return new BptInternalPage();
}
/*---------------------BptInternalPageBuilder Impl End------------------------*/

struct BasePageDescriptor {
    bool  is_leaf_;
    Pgno  page_no_;
    Pgno  parent_no_;
    bool  is_dirty_; 
    size_t pos_;
};

struct InternalPageDescriptor {
    BasePageDescriptor attr_;
    size_t key_num_;
};

/*---------------------------BptLeafPage define-------------------------------*/
struct _BptLeafPage 
{
    Pgno m_prev_; ///< Page number of the previous sibling page of the leaf node.
    Pgno m_next_; ///< Page number of the next sibling page of the leaf node.
};

class BptLeafPage : public IPage
{
public: // overwrites.
    /// @brief 序列化函数.
    /// @return 
    virtual void serializable(void* buf);

    /// @brief 反序列化函数.
    /// @return 
    virtual void deserializable(void* buf);

private:
    _BptLeafPage m_leaf_data_;
};

inline void
BptLeafPage::serializable(void* buf)
{
}

inline void
BptLeafPage::deserializable(void* buf)
{
}



/*---------------------------BptLeafPage define-------------------------------*/


/*-----------------------------BptMeta define---------------------------------*/

/// B+ tree metadata class, stores information about the entire tree.
class BptMeta {
public:
    BptMeta();
public:
    /// @brief btree's order.
    /// @return order.
    char order();

    /// @brief btree's order reference.
    /// @return order reference.
    char& rOrder();

    /// @brief btree's height.
    /// @return height.
    size_t height();

    /// @brief btree's height reference.
    /// @return height reference.
    size_t& rHeight();

    /// @brief pgno for root.
    /// @return root's pgno.
    Pgno root();

    /// @brief root's reference.
    /// @return root's pgno reference.
    Pgno& rRoot();

    /// @brief Overload the equals operator.
    /// @param other 
    void operator=(const BptMeta& other);
private:
    char m_order_;
    size_t m_height_;
    Pgno m_root_no_;
};

/*------------------------------BptMeta impl----------------------------------*/

inline 
BptMeta::BptMeta() : m_order_(0), m_height_(0), m_root_no_(0) 
{  
}

inline char 
BptMeta::order() 
{ 
    return this->m_order_; 
}

inline char& 
BptMeta::rOrder() 
{ 
    return this->m_order_; 
}

inline size_t 
BptMeta::height() 
{ 
    return this->m_height_; 
}

inline size_t& 
BptMeta::rHeight() 
{ 
    return this->m_height_; 
}

inline Pgno 
BptMeta::root() 
{ 
    return this->m_root_no_; 
}

inline Pgno& 
BptMeta::rRoot() 
{ 
    return this->m_root_no_; 
}

inline void
BptMeta::operator=(const BptMeta& other)
{
    this->m_height_ = other.m_height_;
    this->m_order_ = other.m_order_;
    this->m_root_no_ = other.m_root_no_;
}
/*-----------------------------BptMeta impl end-------------------------------*/

/*-------------------------BPlustreeContainer define--------------------------*/
/**
 * BPlustreeContainer is basic container class implemented using bplustree.
 */
template <typename Tree>
class BptContainer {
    typedef BptContainer<Tree> self_type;

public:
    typedef typename Tree::key_type key_type;
    typedef typename Tree::data_type data_type;
    typedef typename Tree::pointer pointer;
    typedef typename Tree::const_pointer const_pointer;
    typedef typename Tree::reference reference;
    typedef typename Tree::const_reference const_reference;
    typedef typename Tree::iterator iterator;
    typedef typename Tree::const_iterator const_iterator;
    typedef typename Tree::reverse_iterator reverse_iterator;
    typedef typename Tree::const_reverse_iterator const_reverse_iterator;

public:
    BptContainer ();
    BptContainer(const self_type &x);
public:
    /// @brief begin of iterator.
    /// @return begin.
    iterator begin();

    /// @brief const begin of iterator.
    /// @return const begin.
    const_iterator begin() const;

    /// @brief end of iterator.
    /// @return end.
    iterator end();

    /// @brief const end of iterator.
    /// @return const end.
    const_iterator end() const;

    /// @brief rebegin of iterator.
    /// @return rebegin.
    reverse_iterator rebegin();

    /// @brief const rebegin of iterator.
    /// @return const rebegin.
    const_reverse_iterator rebegin() const;

    /// @brief rend of iterator.
    /// @return rend.
    reverse_iterator rend();

    /// @brief const rend of iterator.
    /// @return const rend.
    const_reverse_iterator rend() const;
protected:
    Tree tree_;
};

/*-------------------------BPlustreeContainer impl----------------------------*/
template <typename Tree>
BptContainer<Tree>::BptContainer() 
{ 
}

template <typename Tree>
BptContainer<Tree>::BptContainer(const self_type &x) 
{ 
}

template <typename Tree>
typename BptContainer<Tree>::iterator 
BptContainer<Tree>::begin() 
{ 
    return tree_.begin(); 
}

template <typename Tree>
typename BptContainer<Tree>::const_iterator 
BptContainer<Tree>::begin() const 
{ 
    return tree_.begin(); 
}

template <typename Tree>
typename BptContainer<Tree>::iterator 
BptContainer<Tree>::end() 
{ 
    return tree_.end(); 
}

template <typename Tree>
typename BptContainer<Tree>::const_iterator 
BptContainer<Tree>::end() const 
{ 
    return tree_.end(); 
}

template <typename Tree>
typename BptContainer<Tree>::reverse_iterator 
BptContainer<Tree>::rebegin() 
{ 
    return tree_.rebegin(); 
}  

template <typename Tree>
typename BptContainer<Tree>::const_reverse_iterator 
BptContainer<Tree>::rebegin() const 
{ 
    return tree_.rebegin(); 
}

template <typename Tree>
typename BptContainer<Tree>::reverse_iterator 
BptContainer<Tree>::rend() 
{ 
    return tree_.rend(); 
}

template <typename Tree>
typename BptContainer<Tree>::const_reverse_iterator 
BptContainer<Tree>::rend() const 
{ 
    return tree_.rend(); 
}  
/*--------------------------BPlustreeContainer end----------------------------*/

/*----------------------BPlustreeUniqueContainer define-----------------------*/
/**
 * 
 */
template <typename Tree> 
class BptUniqueContainer : public BptContainer<Tree> {
    typedef BptUniqueContainer<Tree> self_type;
    typedef BptContainer<Tree> super_type;
public:
    typedef typename Tree::key_type key_type;
    typedef typename Tree::value_type value_type;
    typedef typename Tree::size_type size_type;
    typedef typename Tree::iterator iterator;
    typedef typename Tree::const_iterator const_iterator;
public:
    BptUniqueContainer();
    BptUniqueContainer(const self_type& x);
};

template <typename Tree> 
BptUniqueContainer<Tree>::BptUniqueContainer() 
{ 
}

template <typename Tree> 
BptUniqueContainer<Tree>::BptUniqueContainer(const self_type& x) 
{ 
}
/*-----------------------BPlustreeUniqueContainer end-------------------------*/

/*---------------------------BptComparator define-----------------------------*/
class BptComparator {
public:
    /// @brief 
    /// @param  
    /// @param  
    void init(EpiphanyFieldType, size_t);

    /// @brief 
    /// @return 
    int attrLength() const;

    /// @brief 
    /// @param v1 
    /// @param v2 
    /// @return 
    int operator() (const char* v1, const char* v2) const;
private:
    EpiphanyFieldType m_attr_type_;
    int m_attr_length_;
};

/*----------------------------BptComparator impl------------------------------*/
inline void 
BptComparator::init(EpiphanyFieldType type, size_t len) 
{
    this->m_attr_type_ = type;
    this->m_attr_length_ = len;
} 

inline int 
BptComparator::attrLength() const 
{
    return this->m_attr_length_;
}

inline int 
BptComparator::operator()(const char* v1, const char* v2) const 
{
    switch (this->m_attr_type_)
    {
    case EpiphanyU8:
        return compareInt((void *)v1, (void*)v2);
    case EpiphanyFloat:
        // compareFloat(((EpiphanyCast(void*, v1)),   EpiphanyCast(void* v2));
        return compareFloat(EpiphanyCast(void*, v1), EpiphanyCast(void*, v2));
    case EpiphanyString:
        return compareString(EpiphanyCast(void*, v1), attrLength(), 
                             EpiphanyCast(void*, v2), attrLength());
    default:
        EpiphanyAssert(false);
        break;
    }
    return 0;
}
/*-----------------------------BptComparator end------------------------------*/

class BptKeyComparator 
{
public:
    BptKeyComparator();
    ~BptKeyComparator();
public:
    /// @brief 
    /// @param  
    /// @param  
    void init(EpiphanyFieldType, size_t);

    /// @brief 
    /// @return 
    const BptComparator &attrComparator() const;

    /// @brief 
    /// @param  
    /// @param  
    /// @return 
    int operator() (const char*, const char*) const;

private:
    BptComparator m_attr_comparator_;
};

inline 
BptKeyComparator::BptKeyComparator() 
{ 
}

inline 
BptKeyComparator::~BptKeyComparator() 
{
}

inline void
BptKeyComparator::init(EpiphanyFieldType type, size_t length) 
{
    m_attr_comparator_.init(type, length);
}

inline const BptComparator &
BptKeyComparator::attrComparator() const {
    return m_attr_comparator_;
}

inline int 
BptKeyComparator::operator() (const char* v1, const char* v2) const 
{
    int result = m_attr_comparator_(v1, v2);
    if (result != 0) {
        return result;
    }
    return 0;
}

class BptFileHeader 
{
public:
    BptFileHeader();
public:

    /// @brief 
    /// @return 
    const std::string toString();
private:
    Pgno         m_root_;
    size_t       m_internal_max_size_;
    size_t       m_leaf_max_size_;
    size_t       m_attr_length_;
    size_t       m_key_length_;
    EpiphanyFieldType m_key_type_;
};

inline 
BptFileHeader::BptFileHeader() 
{ 
}

inline const std::string
BptFileHeader::toString() 
{
    std::stringstream ss;
    ss << "m_attr_length" << m_attr_length_ << ","
       << "m_key_length"  << m_key_length_  << "," 
       << "m_key_type"    << m_key_type_    << ","
       << "m_root_"       << m_root_        << ","
       << "m_internal_max_size_" << m_internal_max_size_ << ","
       << "m_leaf_max_size_"     << m_leaf_max_size_     << ";";
    return ss.str();
}

template <typename Key, int TargetNodeSize, int ValueSize>
struct bplustree_common_params 
{
    typedef Key key_type;
    typedef size_t size_type;
    
    enum {
        kTrargetNodeSize = TargetNodeSize,
        kNodeValueSpace = TargetNodeSize - 2 * sizeof(void*),
    };
};

/// @brief This definition restricts that the Value type used 
///        in Btree must implement the recSize function.
HasMethod(recSize, int);

/// @brief This definition restricts that the Value type used 
///        in Btree must implement the recBuf function.
HasMethod(recBuf, const char*);

/**
 * A specialized version that compiles only 
 * when Value implements specific functions.
 */
template <typename Key, typename Value, 
          typename Ptr, int TargetNodeSize>
struct bplustree_params<Key, Value, Ptr, TargetNodeSize, true> : 
bplustree_common_params<Key, TargetNodeSize, sizeof(Key)> {
    typedef Key key_type;
    typedef Value data_type;
    typedef Value mapped_type;
    typedef Ptr pointer_type;
    typedef std::pair<const key_type, data_type> value_type;
    typedef std::pair<key_type, data_type>  mutable_value_type;
    typedef value_type* pointer;
    typedef const value_type const_pointer;
    typedef value_type& reference;
    typedef const value_type& const_reference;

    enum {
        kValueSize = sizeof(Key),
    };
    
    static const Key& key(const value_type& x) { return x; }
    static void swap(mutable_value_type *a, mutable_value_type *b) {
    } 
};

/// @brief Abstract class for serialization and deserialization interface.
/// @tparam Params
template <typename Params>
class ISerializable 
{
public:
    PARAMS_TYPES
public:
    virtual ~ISerializable();
public:

    /// @brief 
    /// @param pt 
    virtual void serializable(pointer_type pt) = 0;

    /// @brief 
    /// @param pt 
    virtual void deserializable(pointer_type pt) = 0;
};

template <typename Params>
ISerializable<Params>::~ISerializable() { } 

/// BptBaseNode needs to instantiate objects, cannot contain pure virtual functions. 
template <typename Params>
class BptBaseNode : public ISerializable<Params>
{
public:
    PARAMS_TYPES

public:
    BptBaseNode(Pager* pager) : m_pager_(pager), xcompare_(DefaultComparator) {
        ndbMemSet(keys_, 0, sizeof(key_type) * (BtDegree - 1));
    }
    virtual ~BptBaseNode() {}

public:
    class BaseAttributes {
    public:
        BaseAttributes() : pos_(0), count_(0), is_leaf_(true), parent_(0), 
                           degree_(BtDegree), self_no_(0), is_root_(false) {}

    public:
        bool            is_leaf_; ///< Whether it is a leaf node.
        bool            is_root_; ///< leaf / internal may be root.
        size_t          pos_    ; ///< Position in the parent node.
        size_t          count_  ; ///< Number of keys in the current node.
        size_t          degree_ ; ///< Degree of the node, representing the maximum number of members a node can hold.
        pointer_type    parent_ ; ///< Parent node page number.
        pointer_type    self_no_; ///< Current node page number (one page corresponds to one node).
    };

    enum {
        /** Data size **/
        kValueSize = params_type::kValueSize,     
        /** Node size **/                                  
        kTrargetNodeSize = params_type::kTrargetNodeSize,     
        /** Calculated degree value **/                     
        BtCompDegree = (kTrargetNodeSize - sizeof(BaseAttributes)) / kValueSize,    
        
        /** BtDegree.
         *  For binary search, the degree must be at least 3 
         *  Default degree is 16, this choice directly affects B+ tree performance and file size.
         **/
        BtDegree = 3,
        // BtDegree = ((BtCompDegree >= 3 && BtCompDegree <= 128) 
        //            ? BtCompDegree : 3),
    };

public: // options.

    /// @brief inert key's value.
    /// @param pos 
    /// @param _key 
    /// @return 
    BptRet insert_key(size_t pos, key_type _key);

    /// @brief 
    /// @param pos 
    /// @param _key 
    /// @return 
    BptRet pushHeadKey(size_t pos, key_type _key);

    /// @brief rm k.
    /// @param pos 
    /// @return 
    virtual BptRet remove(size_t) { return Btree_Ok; };

public: // attributes.
    
    // read onlys.

    /// @brief return node type.
    /// @return bool
    /// true : current node is root.
    /// false : current node isnt root.
    bool is_root() { return attr_.is_root_; }

    /// @brief return node type.
    /// @return bool
    /// true : current node is leaf.
    /// false : current node isnt leaf.
    bool is_leaf() { return attr_.is_leaf_; }

    /// @brief return pos in parent.
    /// @return sizt_t
    /// (0, size_t max).
    size_t pos() { return attr_.pos_; }

    /// @brief return current node key number.
    /// @return size_t
    /// (0, size_t max).
    size_t count() { return attr_.count_; }

    /// @brief return parent pointer.
    /// @return pointer_type
    /// pointer_type legal value rang.
    pointer_type parent() { return attr_.parent_; }

    pointer_type self_pgno() { return attr_.self_no_; } 

    /// @brief return b+ tree's degree.
    /// @return size_t
    /// 3 < ret < size max.
    size_t degree() { return attr_.degree_; }

    /// @brief node min size, min_size > (n - 1) / 2. So the minimum value is: n / 2.
    /// @return 
    size_t min_size();

    /// @brief node max size, m_size <= n - 1
    /// @return 
    size_t max_size();

    /// @brief read key at pos.
    /// @param pos [in] position.
    /// @param key [out] key's value.
    /// @return BTREE_RET
    /// BTREE_OK : read success.
    /// BTREE_READ_KEY_OVERFLOW : pos > arr len.
    BptRet key(size_t pos, key_type& key);

    /// @brief 
    /// @param pos 
    /// @return 
    key_type* keyAt(size_t);

    /// @brief Append key to node.
    /// @param  
    /// @param num 
    /// @return 
    BptRet appendKeys(key_type*, size_t num);

    /// @brief remove point key. 
    /// @param 
    /// @return 
    BptRet remove_key(size_t);

    /// @brief return bound for key.
    /// @param key 
    /// @param is_upper 
    /// @param found 
    /// @return 
    int bound(key_type, bool, bool*);

    /// @brief return upper bound.
    /// @param _key 
    /// @return 
    int upper_bound(key_type _key, bool* found = EpiphanyNull);

    /// @brief return lower bound.
    /// @param _key 
    /// @return 
    int lower_bound(key_type _key, bool* found = EpiphanyNull);

    /// @brief search key if exist.
    /// @param k 
    /// @param start 
    /// @param end 
    /// @return 
    int binary_search(const key_type k, int start, int end);

    /// @brief get comparator.
    /// @param  
    /// @return 
    int (*comparator(void)) (key_type, key_type){ return xcompare_; }

public: // read writes.
    
    /// @brief return field is_root_'s reference.
    /// @return bool&
    bool& reference_is_root() { return attr_.is_root_; }

    /// @brief 
    /// @return 
    pointer_type& reference_self_pgno() { return attr_.self_no_; }

    /// @brief return field is_leaf_'s reference.
    /// @return bool&
    bool& reference_is_leaf() { return attr_.is_leaf_; }  

    /// @brief return field pos_'s reference.
    /// @return size_t&
    size_t& reference_pos() { return attr_.pos_; }  

    /// @brief return field count_'s reference.
    /// @return size_t&
    size_t& reference_count() { return attr_.count_; } 

    /// @brief return field parent_'s reference.
    /// @return pointer_type&
    pointer_type& reference_parent() { return attr_.parent_; }

    /// @brief return field degree_'s reference.
    /// @return size_t&
    size_t& reference_degree() { return attr_.degree_; }

    /// @brief return key's reference.
    /// @param pos [in] position.
    /// @param key [out] key's reference.
    /// @return BTREE_RET
    /// BTREE_OK : read success.
    /// BTREE_READ_KEY_OVERFLOW : pos > arr len.
    key_type& reference_key(size_t pos);

    void reference_key(size_t pos, key_type*&_key) {
        EpiphanyAssert(pos < this->degree() - 1);   
        _key = &keys_[pos];
    }

    /// @brief return null, need internal node to impl.
    /// @return EpiphanyNull.
    virtual BptInternalNode<Params>* to_internal_node() { return EpiphanyNull; }

    /// @brief return null, need leaf node to impl.
    /// @return EpiphanyNull.
    virtual BptLeafNode<Params>* to_leaf_node() { return EpiphanyNull; }

    /// @brief Move all members to the specified node, pure virtual function, needs to be manually implemented by subclasses.
    /// @param  
    /// @return 
    virtual BptRet move_to(BptBaseNode*) { return Btree_Ok; };

    /// @brief When adding new data, in non-ideal situations, 
    ///        it may be necessary to remove an empty slot 
    ///        like [x][x][x]v[x][x]][x] when inserting in the middle.
    ///        Use this function to move the elements and create an empty 
    ///        slot like [x][x][x][_][x][x]][x].
    /// @tparam BidirIt1 
    /// @tparam BidirIt2 
    /// @param first The starting iterator position.
    /// @param last The current ending iterator position.
    /// @param d_last The final ending iterator position.
    /// @return 
    template<typename BidirIt1, typename BidirIt2>
    BidirIt2 copy_backward(BidirIt1 first, BidirIt2 last, BidirIt2 d_last);

    /// @brief 
    /// @param  
    /// @return 
    virtual BptRet moveHead2Tail(BptBaseNode*) { return Btree_Ok; };

    /// @brief 
    /// @param  
    /// @return 
    virtual BptRet moveTail2Head(BptBaseNode*) { return Btree_Ok; };    

    /// @brief 
    virtual void keyDump() { };
public: // override 
    virtual void serializable(pointer_type pt);

    virtual void deserializable(pointer_type pt);
public:
    Pager* m_pager_; 
public:
    BaseAttributes attr_;

    /** the leaf / internal node 's key max num is degree - 1. 
     *  The minimum value for leaf nodes and internal nodes is the same, which is: min : [m / 2] - 1.
     *                                 max : m - 1.
     *  TODO: To improve performance, it's not necessary to store a complete key array in memory, as repeated serialization will affect overall performance,
     *  modify this array to be a pointer, and use offsets to access it.
     * **/
    key_type keys_[BtDegree - 1];               

    /** a function pointer for compare support user define. **/
    int (*xcompare_)(key_type p1, key_type p2); 
};

template <typename Params> BptRet 
BptBaseNode<Params>::appendKeys(key_type* tail_key, size_t num)
{
    EpiphanyAssertAndCheckRtCode(tail_key != EpiphanyNull, Btree_OpNullPtr);
    ndbMemCpy(this->keyAt(this->count()), tail_key, num * sizeof(key_type));
    return Btree_Ok;
}

template <typename Params> BptRet 
BptBaseNode<Params>::remove_key(size_t pos) 
{
    // EpiphanyAssert(pos < this->count());
    ndbMemmove(keyAt(pos), keyAt(pos + 1), 
              (this->count() - pos) * sizeof(key_type) );
    this->reference_count()--;
    return Btree_Ok;
}

template <typename Params>
int BptBaseNode<Params>::bound(key_type key, bool is_upper, bool* found) 
{
    int start = 0;
    int end = this->count();
    int mid = 0;
    BptRet rc = Btree_Ok;

    // Use binary search algorithm to locate the position of the key
    while (start < end) {
        mid = start + (end - start) / 2;
        
        // key_type may be a simple data type or a composite data type
        key_type current_key;
        rc = this->key(mid, current_key);
        EpiphanyAssert(rc == Btree_Ok);
        // Determine comparison method based on is_upper parameter

        int result = this->comparator()(current_key, key);
        if ((result == 0) && found) {
            *found = true;
            return mid;
        }
        // if ((is_upper ? (result > 0) : (result <= 0))) {
        if ((result > 0)) {
            end = mid;
        } else { 
           start = mid + 1;
        }
    }
    return start;
}

template <typename Params> int 
BptBaseNode<Params>::upper_bound(key_type _key, bool* found) 
{ 
    return bound(_key, true, found); 
} 

template <typename Params> int 
BptBaseNode<Params>::lower_bound(key_type _key, bool* found) 
{ 
    return bound(_key, false, found); 
}

template <typename Params>
BptRet BptBaseNode<Params>::key(size_t pos, key_type& key) 
{
    EpiphanyAssertAndCheckRtCode(pos < this->degree() - 1, Btree_ReadArrayOfl);   
    key = keys_[pos];
    return Btree_Ok;
}

template <typename Params> typename BptBaseNode<Params>::key_type* 
BptBaseNode<Params>::keyAt(size_t pos)
{
    return &keys_[pos];
}

template <typename Params>
typename BptBaseNode<Params>::key_type& 
BptBaseNode<Params>::reference_key(size_t pos) 
{
    EpiphanyAssert(pos < this->degree() - 1);   
    return keys_[pos];
}

template <typename Params>
BptRet BptBaseNode<Params>::insert_key(size_t pos, key_type _key)
{
    keys_[pos] = _key;
    reference_count()++;
    return Btree_Ok;
}

template <typename Params>
BptRet BptBaseNode<Params>::pushHeadKey(size_t pos, key_type _key) {
    keys_[pos + 1] = keys_[pos];
    keys_[pos] = _key;
    reference_count()++;
    return Btree_Ok;
}

template <typename Params> int 
BptBaseNode<Params>::binary_search(const key_type k, int start, int end) 
{
    key_type lkey;
    BptRet rc = Btree_Ok;

    while (start != end) {
        int mid = (start + end) / 2;
        rc = this->key(mid, lkey);
        EpiphanyAssertAndCheckRtCode(rc == Btree_Ok, -1);
        if (lkey == k) {
            return mid;
        }

        if (xcompare_(lkey, k) > 0) {
            start = mid + 1;
        } else {
            end = mid;
        }
    }
    return -1;
}

template <typename Params>
template<typename BidirIt1, typename BidirIt2> BidirIt2 
BptBaseNode<Params>::copy_backward(BidirIt1 first, 
                                   BidirIt2 last, 
                                   BidirIt2 d_last) 
{
    while (last != first) {
        *(--d_last) = *(--last);
    }
    return d_last;
}

template <typename Params> size_t 
BptBaseNode<Params>::min_size() 
{
    return (degree() / 2);
}

template <typename Params> size_t 
BptBaseNode<Params>::max_size() {
    return (degree() - 1);
}

template <typename Params> void 
BptBaseNode<Params>::serializable(pointer_type pt) {

}

template <typename Params> void 
BptBaseNode<Params>::deserializable(pointer_type pt) {
    EpiphanyAssert(pt > 0);

    void* buf = this->m_pager_->getPageBuffer(pt);
    EpiphanyAssert(buf);
    
    // need to judge whether its value is reasonable.
    InternalPageDescriptor *page_desc = (InternalPageDescriptor *)buf;
    this->reference_is_leaf() = page_desc->attr_.is_leaf_;
    this->reference_self_pgno() = page_desc->attr_.page_no_;
    this->reference_count() = page_desc->key_num_;
    this->reference_parent() = page_desc->attr_.parent_no_;
    this->reference_pos() = page_desc->attr_.pos_;
}


template <typename Params>
class BptInternalNode : 
public BptBaseNode<Params> {
public:
    PARAMS_TYPES
public:
    BptInternalNode(Pager* pager) : BptBaseNode<Params>(pager) {
        this->reference_is_leaf() = false;
        childs_ = new pointer_type[this->degree()];
    }

    ~BptInternalNode() {
        if (childs_) {
            delete [] childs_;
            childs_ = EpiphanyNull;
        }
    }

    /**
     * In the original implementation of the B+ tree, both internal and leaf 
     * nodes have the same number of keys and the same maximum and minimum 
     * values. Therefore, the keys were placed in the base class for unified 
     * management. However, this led to complex operations and messy code. 
     * To simplify the code, the keys have been moved to the subclass, where 
     * they are managed uniformly. However, the base class still retains methods 
     * for key operations. In this version of the B+ tree, where k = n - 1 
     * and v = n, the internal nodes have n keys and n+1 child pointers when 
     * the degree is n.
     */
    struct InternalItem {
        key_type key_;
        pointer_type child_; 
    };

    /** Define an iterator for iterating through internal nodes. */
    class Iterator {
    public:
        Iterator(InternalItem* ptr, size_t position) : ptr_(ptr)  {}

        InternalItem& operator*() const { return ptr_; }
        InternalItem* operator->() const { return ptr_; }

        Iterator& operator++() { 
            ++ptr_;
            return *this;
        }

        Iterator operator++(int) { 
            Iterator tmp = *this; 
            ++ptr_;
            return tmp;
        }

        // Overload equality operator
        bool operator==
        (const Iterator& other) const { return ptr_ == other.ptr_; }

        // Overload inequality operator
        bool operator!=
        (const Iterator& other) const { return ptr_ != other.ptr_; }
    private:
        InternalItem* ptr_;
    };
public: // override BtBaseNode functions.
    
    /// @brief remove key childs.
    /// @param _key 
    /// @return 
    virtual BptRet remove(size_t);

    /// @brief return internal node pointer.
    /// @return 
    virtual BptInternalNode<Params>* to_internal_node();

    /// @brief 
    /// @param  
    /// @return 
    virtual BptRet move_to(BptBaseNode<Params>*);

    /// @brief 
    virtual void keyDump();

    /// @brief 
    /// @param  
    /// @return 
    BptRet moveHead2Tail(BptBaseNode<Params>*);

    /// @brief 
    /// @param  
    /// @return 
    BptRet moveTail2Head(BptBaseNode<Params>*);  
public: // override ISerializable.

    /// @brief 
    /// @param pt 
    void serializable(pointer_type pt);

    /// @brief 
    /// @param pt 
    void deserializable(pointer_type pt);

public: // read only.

    /// @brief return child at position.
    /// @param _pos 
    /// @param child 
    /// @return 
    BptRet child(size_t _pos, pointer_type& child);

    pointer_type* childAt(size_t index);
public: // read writes.

    /// @brief 
    /// @param _pos 
    /// @param child 
    /// @return 
    BptRet reference_child(size_t _pos, pointer_type*& child);

    /// @brief insert key.
    /// @param key 
    /// @param old_pgno 
    /// @param new_leaf_pgno 
    /// @return 
    BptRet insert_key_to_index(BptMeta&, key_type, pointer_type, pointer_type);

    /// @brief insert key no split.
    /// @param _key 
    /// @param _new_internal_pgno 
    /// @return 
    BptRet insert_key_to_index_no_split(key_type, pointer_type);

    /// @brief split internal node.
    /// @param _pos 
    /// @param rnode 
    /// @return 
    BptRet split(size_t, BptInternalNode<Params> &);
    
    /// @brief 
    /// @param  
    /// @param num 
    /// @return 
    BptRet append(pointer_type*, size_t num);

    /// @brief 
    /// @return 
    Iterator begin();

    /// @brief 
    /// @return 
    Iterator end();
private:
    pointer_type *childs_;
    
    InternalItem *items_;
    pointer_type rchild_; /** Internal node stores N+1 children. */
};

template<typename Params> BptRet 
BptInternalNode<Params>::remove(size_t index) 
{
    EpiphanyAssert(index >=0);
    pointer_type current = *childAt(index);

    bool is_leaf = ((InternalPageDescriptor*)
    this->m_pager_->getPageBuffer(current))->attr_.is_leaf_;

    if (is_leaf)
    {
        BptLeafNode<Params> current_leaf_node(this->m_pager_);
        current_leaf_node.deserializable(current);

        pointer_type pre_no = current_leaf_node.prev();
        if (pre_no != 0)
        {
            BptLeafNode<Params> pre_leaf_node(this->m_pager_);
            pre_leaf_node.deserializable(pre_no);
            pre_leaf_node.reference_next() = current_leaf_node.next();
            pre_leaf_node.serializable(pre_no);
        }

        if (current_leaf_node.next() != 0)
        {
            BptLeafNode<Params> next_leaf_node(this->m_pager_);
            next_leaf_node.deserializable(current_leaf_node.next());
            next_leaf_node.reference_prev() = pre_no;
            next_leaf_node.serializable(current_leaf_node.next());
        }  
    } 
    
    // Deleting an internal node is the same as deleting a leaf.
    ndbMemmove(childAt(index), childAt(index + 1), 
              (this->count() - index) * sizeof(pointer_type) );
    
    this->remove_key( (index == 0) ? index : index - 1);
    return Btree_Ok;
}

template<typename Params> BptInternalNode<Params>* 
BptInternalNode<Params>::to_internal_node()  
{ 
    return this; 
}

template<typename Params> BptRet 
BptInternalNode<Params>::move_to(BptBaseNode<Params>* other) 
{
    EpiphanyAssert(other);
    
    BptInternalNode<Params>* other_internal = other->to_internal_node();
    EpiphanyAssert(other_internal);

    other_internal->append(this->childAt(0), this->count() + 1);
    other_internal->appendKeys(this->keyAt(0), this->count());
    other_internal->reference_count() += this->count();
    this->reference_count() -= this->count();
    return Btree_Ok;
}

template<typename Params> void 
BptInternalNode<Params>::keyDump() 
{
    printf("\033[33m Internal Node:p(%lld)", this->self_pgno());
    for (int i = 0; i < this->count(); i++)
    {   
        /** Can only print integer types. */
        printf("\033[0m %d", *(int*)(this->keyAt(i)));
    }
    printf("\n");
}

template<typename Params> BptRet 
BptInternalNode<Params>::moveHead2Tail(BptBaseNode<Params>* other) 
{
    EpiphanyAssert(other);
    BptRet rc = other->to_internal_node()->append((this->childAt(0)), 1);
    // this->remove(0);
    return rc;
}

template<typename Params> BptRet 
BptInternalNode<Params>::moveTail2Head(BptBaseNode<Params>*) 
{
    return Btree_Ok;
}  

template<typename Params> void 
BptInternalNode<Params>::serializable(pointer_type pt) 
{
    EpiphanyAssert(pt > 0);

    void* page_buffer = this->m_pager_->getPageBuffer(pt);
    PageManager::init(page_buffer, pt);
    EpiphanyAssert(page_buffer);

    InternalPageDescriptor *page_desc = (InternalPageDescriptor *)page_buffer;
    page_desc->attr_.is_leaf_ = false;
    page_desc->attr_.page_no_ = pt;
    page_desc->attr_.is_dirty_ = true;
    page_desc->key_num_ = this->count();
    page_desc->attr_.parent_no_ = this->parent();
    page_desc->attr_.pos_ = this->pos();

    size_t offs = sizeof(InternalPageDescriptor);

    // write key to page.
    for (int i = 0; i < this->count(); i++) {
        key_type k;
        this->key(i, k);
        memcpy((char*)page_buffer + offs, &k, sizeof(key_type));
        offs += sizeof(key_type);
    }

    // write child's pgno to page.
    for (int j = 0; j < this->count() + 1; j++) {
        pointer_type ch;
        this->child(j, ch);
        memcpy((char*)page_buffer + offs, &ch, sizeof(pointer_type));
        offs += sizeof(pointer_type);
    }
}

template<typename Params> void 
BptInternalNode<Params>::deserializable(pointer_type pt) 
{
    EpiphanyAssert(pt > 0);

    void* buf = this->m_pager_->getPageBuffer(pt);
    EpiphanyAssert(buf);
    
    // need to judge whether its value is reasonable.
    InternalPageDescriptor *page_desc = (InternalPageDescriptor *)buf;
    this->reference_self_pgno() = page_desc->attr_.page_no_;
    this->reference_count() = page_desc->key_num_;
    this->reference_parent() = page_desc->attr_.parent_no_;
    this->reference_pos() = page_desc->attr_.pos_;

    // copy part keys.
    size_t offs = sizeof(InternalPageDescriptor);
    for (int i = 0; i < this->count(); i++) {
        this->reference_key(i) = *((key_type*)((char*)buf + offs));
        int p = *((int*)((char*)buf + offs));
        offs += sizeof(key_type);
    }

    // copy part childs.
    for (int j = 0; j < this->count() + 1; j++) {
        pointer_type *v = EpiphanyNull;
        this->reference_child(j, v);
        *v = *((pointer_type*)((char*)buf + offs));
        offs += sizeof(pointer_type);
    }
}

template <typename Params> BptRet 
BptInternalNode<Params>::child(size_t _pos, pointer_type& child) 
{
    EpiphanyAssertAndCheckRtCode(_pos <= this->degree(), Btree_ReadArrayOfl);   
    child = childs_[_pos];
    return Btree_Ok;
}

template <typename Params> typename BptInternalNode<Params>::pointer_type*
BptInternalNode<Params>::childAt(size_t index) 
{
    return &childs_[index];
}

template <typename Params> BptRet 
BptInternalNode<Params>::reference_child(size_t _pos, pointer_type*& child) 
{
    EpiphanyAssertAndCheckRtCode(_pos < this->degree(), Btree_ReadArrayOfl);   
    child = &childs_[_pos];
    return Btree_Ok;   
}


/**
 * Implementation steps:
 *  1. First, find the correct position for the inserted key in the current node.
 *  2. If the node is not full, directly insert the key into the corresponding position and return.
 *  3. If the node is full, splitting is needed.
 *   1). Determine the split point, divide the split point into two parts, and promote the middle key to the parent node (overflow).
 *   2). Create a new node, move the second half of the keys and child node pointers from the original node to the new node.
 *   3). Insert the promoted key into the parent node. If the parent node is empty (current node is the root node), create a new parent node,
 *       insert the promoted key into it, and connect the original node and the new node as child nodes to the parent node.
 */
template <typename Params> BptRet 
BptInternalNode<Params>::insert_key_to_index(BptMeta &meta, key_type key, 
                                             pointer_type old_pgno, 
                                             pointer_type new_pgno) 
{
    key_type rk;
    BptRet rc = Btree_Ok;

    // full.
    if ( BptreeFull(this->count(), this->degree()) ) {
        BptInternalNode<Params> new_node(this->m_pager_);

        pointer_type new_node_pgno = this->m_pager_->allocatePage();
        
        new_node.reference_is_leaf() = false;
        new_node.reference_self_pgno() = new_node_pgno;

        size_t point = (this->count() -1) / 2;

        rc = this->key(point, rk);
        EpiphanyAssertAndCheckRtCode(rc == Btree_Ok, rc);

        bool place_right = this->comparator()(key, rk) > 0;

        if (place_right) {
            ++point;
        }

        rc = this->key(point, rk);
        EpiphanyAssertAndCheckRtCode(rc == Btree_Ok, rc);

        if (place_right && this->comparator()(key, rk) < 0) {
            point--;
        }

        key_type middle_key;
        rc = this->key(point, middle_key);
        EpiphanyAssertAndCheckRtCode(rc == Btree_Ok, rc);

        rc = this->split(point, new_node);
        EpiphanyAssertAndCheckRtCode(rc == Btree_Ok, rc);

        if ( place_right ) {
            new_node.insert_key_to_index_no_split(key, new_pgno);
        } else {
            this->insert_key_to_index_no_split(key, new_pgno);
        }

        this->serializable(this->self_pgno());
        new_node.serializable(new_node_pgno);
        
        BptInternalNode<Params> parent(this->m_pager_);
        if ( this->parent() != 0) {
            parent.deserializable(this->parent());
        }
        parent.insert_key_to_index(meta, middle_key, 
                                   this->self_pgno(), new_node_pgno);
    } else {
        this->insert_key_to_index_no_split(key, new_pgno);
        this->serializable(this->self_pgno());
    }

    return Btree_Ok;
}

template <typename Params> BptRet 
BptInternalNode<Params>::insert_key_to_index_no_split(key_type _key, 
                                                      pointer_type new_pgno) 
{
    pointer_type *lchild = EpiphanyNull;

    BptRet rc = Btree_Ok;
    size_t pos = this->upper_bound(_key);

    this->reference_key(pos) = _key;
    EpiphanyAssertAndCheckRtCode(rc == Btree_Ok, rc);

    rc = this->reference_child(this->count() + 1, lchild);
    EpiphanyAssertAndCheckRtCode(rc == Btree_Ok, rc);

    *lchild = new_pgno;
    this->reference_count() = this->count() + 1;

    bool is_leaf = ((InternalPageDescriptor*)
    this->m_pager_->getPageBuffer(new_pgno))->attr_.is_leaf_;
    if (is_leaf) {
        BptLeafNode<Params> leaf_node(this->m_pager_);
        leaf_node.deserializable(new_pgno);
        leaf_node.reference_parent() = this->self_pgno();
        leaf_node.reference_pos() =  + 1;
        leaf_node.serializable(new_pgno);
    } else {
        BptInternalNode<Params> internal_node(this->m_pager_);
        internal_node.deserializable(new_pgno);
        internal_node.reference_parent() = this->self_pgno();
        internal_node.reference_pos() = pos + 1;
        internal_node.serializable(new_pgno);
    }
    return Btree_Ok;
}

template <typename Params> BptRet 
BptInternalNode<Params>::split(size_t _pos, BptInternalNode<Params> &rnode) 
{
    key_type rk;
    pointer_type *lchild;
    pointer_type child;

    BptRet rc = Btree_Ok;
    
    for(int i = _pos + 1; i < this->count(); i++){
        rc = this->key(i, rk);
        rnode.reference_key(i - (_pos + 1)) = rk;
        EpiphanyAssertAndCheckRtCode(rc == Btree_Ok, rc);
    }

    for(int i = _pos ; i < this->count(); i++){

        rc = rnode.reference_child(i - (_pos ), lchild);
        EpiphanyAssertAndCheckRtCode(rc == Btree_Ok, rc);

        rc = this->child(i + 1, child);
        EpiphanyAssertAndCheckRtCode(rc == Btree_Ok, rc);
        *lchild = child;

        // update child parent.

        bool is_leaf = *((bool*)this->m_pager_->getPageBuffer(child));

        if (is_leaf) {
            BptLeafNode<Params> leaf_node(this->m_pager_);
            leaf_node.deserializable(child);
            leaf_node.reference_parent() = rnode.self_pgno();
            leaf_node.reference_pos() = (i - _pos);
            leaf_node.serializable(child);
        } else {
            BptInternalNode<Params> internal_node(this->m_pager_);
            internal_node.deserializable(child);
            internal_node.reference_parent() = rnode.self_pgno();
            internal_node.reference_pos() = (i - _pos);
            internal_node.serializable(child);
        }
    }
    rnode.reference_count() = this->count() - _pos - 1;
    this->reference_count() = _pos;
    return Btree_Ok;
}

template <typename Params> BptRet 
BptInternalNode<Params>::append(pointer_type* tail_child, size_t num) 
{
    EpiphanyAssertAndCheckRtCode(tail_child, Btree_OpNullPtr);
    ndbMemCpy(childAt(this->count() == 0 ? 1 : this->count()), 
    tail_child, num * sizeof(pointer_type));
    return Btree_Ok;
}

template <typename Params> typename BptInternalNode<Params>::Iterator
BptInternalNode<Params>::begin() {
    return Iterator(this->items_, 0);
}

template <typename Params> typename BptInternalNode<Params>::Iterator
BptInternalNode<Params>::end() {
    return Iterator(this->items_, 0);
}

/// @brief Leaf node definition.
/// @tparam Params 
template <typename Params>
class BptLeafNode : public BptBaseNode<Params>
{
public:
    PARAMS_TYPES
public:
    BptLeafNode(Pager* pager) : BptBaseNode<Params>(pager), prev_(0), next_(0) {
        data_ = new data_type[this->degree()];
    }
    ~BptLeafNode() {
        if (data_) {
            delete [] data_;
            data_ = EpiphanyNull;
        }
    }

    struct LeafItem {
        key_type key_;
        data_type data_;
    };

    class Iterator {
    public:
        Iterator(LeafItem* ptr, size_t position) : ptr_(ptr) { }

        LeafItem& operator*() const { return ptr_; }
        LeafItem* operator->() const { return ptr_; }

        Iterator& operator++() {
            ++ptr_;
            return *this;
        }

        Iterator operator++(int) {
            Iterator tmp = *this;
            ++ptr_;
            return tmp;
        }

        bool operator==(const Iterator& other) const {
            return ptr_ == other.ptr_;
        }

        bool operator!=(const Iterator& other) const {
            return ptr_ != other.ptr_;
        }
    private:
        LeafItem* ptr_;
    };
    
public: // override BtBaseNode functions.
       
    /// @brief remove key and data.
    /// @param _key 
    /// @return 
    BptRet remove(size_t);

    /// @brief convert leaf node.
    /// @return leaf node pointer.
    BptLeafNode<Params>* to_leaf_node();

    /// @brief 
    /// @param  
    /// @return 
    virtual BptRet move_to(BptBaseNode<Params> *);

    /// @brief 
    /// @param  
    /// @return 
    virtual BptRet moveHead2Tail(BptBaseNode<Params>*);

    /// @brief 
    /// @param  
    /// @return 
    virtual BptRet moveTail2Head(BptBaseNode<Params>*);  

    /// @brief 
    virtual void keyDump();
public: // override ISerializable.

    /// @brief override parent class.
    /// @param pt 
    void serializable(pointer_type pt);

    /// @brief disk to node.
    /// @param pt 
    void deserializable(pointer_type pt);

public: // read onlys.
    
    /// @brief return leaf node prev pointer.
    /// @return 
    pointer_type prev() { return prev_; }

    /// @brief return next node pointer.
    /// @return 
    pointer_type next() { return next_; }

    /// @brief return data at position.
    /// @param _pos [in] position.
    /// @param data [out] read's data.
    /// BTREE_OK : read success.
    /// BTREE_READ_KEY_OVERFLOW : pos > arr len.
    BptRet data(size_t _pos, data_type& data);

public: // read writes.
    BptRet insert(size_t, key_type*, data_type*);

    /// @brief return prev reference.
    /// @return 
    pointer_type& reference_prev() { return this->prev_; }

    /// @brief return next reference.
    /// @return 
    pointer_type& reference_next() { return this->next_; }

    /// @brief return reference data.
    /// @param _pos 
    /// @param data 
    /// @return 
    BptRet reference_data(size_t _pos, data_type*& data);

    /// @brief init data.
    /// @param _pos 
    /// @param _key 
    /// @param _pdata 
    BptRet data_init(size_t _pos, key_type _key, data_type _pdata);

    /// @brief split leaf node to rnode. Splitting always starts from the middle, so there's no need to pass in a split point.
    /// @param rnode 
    BptRet split(BptLeafNode<Params> &rnode);

    /// @brief insert a record not need split.
    /// @param _key 
    /// @param _data 
    /// @return 
    BptRet insert_record_no_split(key_type _key, data_type _data);    

    /// @brief 
    /// @param  
    /// @param num 
    /// @return 
    BptRet append(data_type*, size_t num);

    /// @brief 
    /// @param  
    /// @return 
    BptRet append(data_type*);

    /// @brief 
    /// @param  
    /// @return 
    BptRet preappend(data_type*);

    /// @brief 
    /// @param  
    /// @return 
    data_type* dataAt(size_t);

    /// @brief 
    /// @return 
    Iterator begin();

    /// @brief 
    /// @return 
    Iterator end();
public:
    pointer_type prev_;
    pointer_type next_;

    data_type *data_;
    
    LeafItem* items_;
    data_type* rdata_;
};

template <typename Params> BptRet 
BptLeafNode<Params>::remove(size_t index) 
{
    EpiphanyAssertAndCheckRtCode(index < this->count(), Btree_ReadArrayOfl);

    /// Delete the data pointed to by index in the leaf node
    /// | data1 | data2 | data3 | data4 | dataN | 
    ///     0       1       2       3       n
    /// example delete index is 2.
    /// | data1 | data3 | data4 | dataN |
    /// copy num is n - index
    memcpy(this->dataAt(index), this->dataAt(index + 1),
          (this->count() - index) * sizeof(data_type));

    this->remove_key(index);
    return Btree_Ok;
}

template <typename Params> BptLeafNode<Params>* 
BptLeafNode<Params>::to_leaf_node() 
{ 
    return this; 
}

template <typename Params> BptRet 
BptLeafNode<Params>::move_to(BptBaseNode<Params> *other) 
{
    EpiphanyAssert(other);
    
    /** This must be a leaf node. */
    EpiphanyAssert(other->is_leaf());
    BptLeafNode<Params>* other_leaf = other->to_leaf_node();

    /** Copy all data from the current node to another 
     * node and update the next pointer. */
    other_leaf->append(this->dataAt(0), this->count());
    other_leaf->appendKeys(this->keyAt(0), this->count());
    other_leaf->reference_next() = this->next();
    other_leaf->reference_prev() = this->prev();
    other_leaf->reference_count() = this->count();
    this->reference_count() -= this->count();
    return Btree_Ok;
}

template <typename Params> BptRet
BptLeafNode<Params>::moveHead2Tail(BptBaseNode<Params>* other) 
{
    EpiphanyAssertAndCheckRtCode(other, Btree_OpNullPtr);
    other->to_leaf_node()->append(this->dataAt(0));
    other->appendKeys(this->keyAt(0), 1);
    other->reference_count()++;
    return this->remove(0);
}

template <typename Params> BptRet 
BptLeafNode<Params>::moveTail2Head(BptBaseNode<Params>* other) 
{
    EpiphanyAssertAndCheckRtCode(other, Btree_OpNullPtr);
    other->to_leaf_node()->preappend(dataAt(this->count() - 1));
    return this->remove(0);
}

template <typename Params> void 
BptLeafNode<Params>::keyDump() 
{
    printf("\033[32m Leaf Node:p(%lld)", this->self_pgno());
    
    for (int i = 0; i < this->count(); i++)
    {
        printf("\033[0m %d", *(int*)(this->keyAt(i)));
    }
    printf("\033[0m\n");
}

template <typename Params> BptRet 
BptLeafNode<Params>::data(size_t _pos, data_type& data) 
{
    EpiphanyAssertAndCheckRtCode(_pos < this->degree(), Btree_ReadArrayOfl);   
    data = data_[_pos];
    return Btree_Ok;
}

template <typename Params> void 
BptLeafNode<Params>::serializable(pointer_type pt) 
{
    EpiphanyAssert(pt > 0);

    void* page = this->m_pager_->getPageBuffer(pt);

    PageManager::init(page, pt);
    EpiphanyAssert(page);

    PageDescriptor *page_desc = (PageDescriptor *)page;
    page_desc->is_leaf_ = true;
    page_desc->page_no_ = pt;
    page_desc->is_dirty_ = true;
    page_desc->next_ = this->next();
    page_desc->prev_ = this->prev();
    page_desc->key_num_ = this->count();
    page_desc->parent_no_ = this->parent();
    page_desc->pos_ = this->pos();

    for (int i = 0; i < this->count(); i++) {
        data_type d;
        this->data(i, d);
        unsigned int sz = d.recSize() + sizeof(key_type) + sizeof(size_t);
        key_type k;
        this->key(i, k);
        int offs = PageManager::allocate(sz, page);
        EpiphanyAssert(offs >= 0);
        if (offs <= 0) {
            // create overflow page.
        }
        const char* b = d.recBuf();

        /** Copy key. */
        memcpy((char*)page + offs, &k, sizeof(key_type));
        
        size_t data_len = d.recSize();
        memcpy((char*)page + offs + sizeof(key_type), 
               &data_len, sizeof(size_t));

        memcpy((char*)page + offs + sizeof(key_type) +  sizeof(size_t), 
                b, d.recSize());
    }
}

template <typename Params> void 
BptLeafNode<Params>::deserializable(pointer_type pt)
{
    EpiphanyAssert(pt > 0);

    void *buf = this->m_pager_->getPageBuffer(pt);
    EpiphanyAssert(buf);

    // Get the PageDescriptor pointer
    PageDescriptor *page_desc = (PageDescriptor *)(buf);
    EpiphanyAssert(page_desc);
    EpiphanyCheckRtVoid(page_desc);

    // Set reference to the previous page
    this->reference_prev() = page_desc->prev_;

    // Set reference to the next page
    this->reference_next() = page_desc->next_;

    // Set reference to the current page number
    this->reference_self_pgno() = page_desc->page_no_;

    // Set the reference to the parent page
    this->reference_parent() = page_desc->parent_no_;

    this->reference_count() = page_desc->key_num_;

    this->reference_pos() = page_desc->pos_;

    this->reference_is_leaf() = page_desc->is_leaf_;

    size_t offs = sizeof(PageDescriptor);
    for (size_t i = 0; i < page_desc->key_num_; i++) {
        this->reference_key(i) = *(key_type*)((char*)buf + offs);
        offs += sizeof(key_type);
        data_type *data;
        this->reference_data(i, data);
        size_t len = *(size_t*)((char*)buf + offs);
        offs += sizeof(size_t);
        (*data).setRecBuf(((char*)buf + offs));
        (*data).setRecSize(len);
        offs += len;
    }
}

template <typename Params> BptRet 
BptLeafNode<Params>::insert(size_t, key_type*, data_type*) 
{
    return Btree_Ok;
}

template <typename Params> BptRet 
BptLeafNode<Params>::reference_data(size_t _pos, data_type*& data) 
{
    data = &(data_[_pos]);
    return Btree_Ok; 
}

template <typename Params> BptRet 
BptLeafNode<Params>::data_init(size_t _pos, key_type _key, data_type _pdata) 
{
    EpiphanyAssertAndCheckRtCode(_pos < this->degree(), Btree_ReadArrayOfl);
    
    data_type *ldata = EpiphanyNull;
    BptRet rc = Btree_Ok;
    
    this->reference_key(_pos) = _key;
    EpiphanyAssertAndCheckRtCode(rc == Btree_Ok, rc);

    rc = this->reference_data(_pos, ldata);
    EpiphanyAssertAndCheckRtCode(rc == Btree_Ok, rc);
    *ldata = _pdata;
    this->reference_count() = this->count() + 1;
    return Btree_Ok;
}

template <typename Params> BptRet 
BptLeafNode<Params>::split(BptLeafNode<Params> &rnode) 
{
    key_type rk;
    data_type data;

    BptRet rc = Btree_Ok;
    
    for(int i = this->count() / 2; i < this->degree() - 1; i++){

        rc = this->key(i, rk);
        EpiphanyAssertAndCheckRtCode(rc == Btree_Ok, rc);

        rc = this->data(i, data);
        EpiphanyAssertAndCheckRtCode(rc == Btree_Ok, rc);

        rnode.insert_record_no_split(rk, data);
    }
    rnode.reference_pos() = this->pos() + 1;
    this->reference_count() = this->count() - rnode.count();
    return Btree_Ok;
}

template <typename Params> BptRet 
BptLeafNode<Params>::insert_record_no_split(key_type _key, data_type _data) 
{
    key_type *key1, *key2, *key3 = EpiphanyNull;
    data_type *record, *last, *d_last = EpiphanyNull;

    BptRet rc = Btree_Ok;
    size_t pos = this->lower_bound(_key);
    size_t current_count = this->count();

    rc = this->reference_data(pos, record);
    EpiphanyAssertAndCheckRtCode(rc == Btree_Ok, rc);

    rc = this->reference_data(current_count, last);
    EpiphanyAssertAndCheckRtCode(rc == Btree_Ok, rc);

    rc = this->reference_data(current_count == 1 ? current_count : 
                              current_count + 1, d_last);
    EpiphanyAssertAndCheckRtCode(rc == Btree_Ok, rc);

    /** Move an empty slot. */
    this->copy_backward(record, last, d_last);
    this->copy_backward(&this->keys_[pos], 
                        &this->keys_[this->count()], 
                        &this->keys_[this->count() + 1]);

    rc = this->data_init(pos, _key, _data);
    EpiphanyAssertAndCheckRtCode(rc == Btree_Ok, rc);

    return Btree_Ok;
}

template <typename Params> BptRet 
BptLeafNode<Params>::append(data_type* tail_data, size_t num) 
{
    EpiphanyAssertAndCheckRtCode(tail_data, Btree_OpNullPtr);
    ndbMemCpy(dataAt(this->count()), tail_data, num * sizeof(data_type));
    return Btree_Ok;
}

template <typename Params> BptRet 
BptLeafNode<Params>::append(data_type* item) 
{
    return append(item, 1);
}

template <typename Params> BptRet 
BptLeafNode<Params>::preappend(data_type* item) 
{
    return this->insert(0, (key_type*)item, item + (sizeof(key_type)));
}

template <typename Params> typename BptLeafNode<Params>::data_type* 
BptLeafNode<Params>::dataAt(size_t pos) 
{
    EpiphanyAssert(pos <= this->count());
    return &data_[pos];
}

template <typename Params> typename BptLeafNode<Params>::Iterator
BptLeafNode<Params>::begin() {
    return Iterator(this->items_, 0);
}

template <typename Params> typename BptLeafNode<Params>::Iterator
BptLeafNode<Params>::end() {
    return Iterator(this->items_, 0);
}

/// @brief B+ tree iterator definition.
template<typename Node, typename Reference, typename Pointer>
class BPlustreeIterator
{
public:
    typedef typename Node::params_type params_type;
    typedef typename Node::data_type data_type;
    typedef const Node const_node;
    typedef Reference reference;
    typedef typename params_type::reference normal_reference;
    typedef typename params_type::const_reference const_reference;
    typedef typename params_type::pointer normal_pointer;
    typedef typename params_type::const_pointer const_pointer;
    typedef BPlustreeIterator<const_node, const_reference, const_pointer> const_iterator;
    typedef BPlustreeIterator<Node, Reference, Pointer> self_type;

    BPlustreeIterator() : node_(EpiphanyNull), pos_(-1), m_pager_(EpiphanyNull) {}
    BPlustreeIterator(Node* node) : node_(node), pos_(-1), m_pager_(EpiphanyNull) {}
    BPlustreeIterator(int pos) : node_(EpiphanyNull), pos_(pos), m_pager_(EpiphanyNull) {}

    BPlustreeIterator(Node* node, int pos, Pager* pager) : 
    node_(node), pos_(pos), m_pager_(pager) {}

    BPlustreeIterator(const BPlustreeIterator<Node, Reference, Pointer> &other) 
    : node_(other.node_), pos_(other.pos_), m_pager_(other.m_pager_) {}
    
    ~BPlustreeIterator() { 
        // delete node_; 
    }

    void increment() {
        if (node_->is_leaf() && ++pos_ < node_->count()) {
            return;
        } else if (node_->is_leaf() && pos_ >= node_->count()) {
            move_to_next_leaf();
        }
    }

    data_type operator*() const {
        data_type data;
        node_->to_leaf_node()->data(pos_, data);
        return data;
    }
    data_type get(int &rid) {
        data_type data;
        node_->to_leaf_node()->data(pos_, data);

        rid = *(int*)(data.recBuf() + *(int*)data.recBuf() - sizeof(int));
        return data;
    }

    Node* operator->() const { return node_; }

    self_type operator++() {
        increment();
        return *this;
    }

    BPlustreeIterator operator++(int) {
        // BPlustreeIterator tmp = *this;
        // ++(*this);
        // return tmp;
        return ++(*this);
    }

    bool 
    operator==(const BPlustreeIterator &i) const { return node_ == i.node_; }

    bool 
    operator!=(const BPlustreeIterator &i) const { return node_ != i.node_; }

    void
    assign(BPlustreeIterator &src_iter) {
        this->node_ = src_iter.node_;
        this->pos_ = src_iter.pos_;
        this->m_pager_ = src_iter.m_pager_;
    }

    bool
    is_end() {
        // return (node_ == end_node_) && (pos_ == end_pos_);
        return node_ == EpiphanyNull;
    }

private:
    void move_to_next_leaf() {
        if (node_->to_leaf_node()->next() != 0) {
            Node* new_node = new BptLeafNode<params_type>(m_pager_);
            new_node->to_leaf_node()->deserializable(node_->to_leaf_node()->next());
            delete node_;
            node_ = new_node;
            pos_ = 0;
        } else {
            delete node_;
            node_ = EpiphanyNull;
        }
    }

    Node* node_; ///< Points to a node in the B-tree
    int pos_;    ///< Points to a position in the node

public:
    /** Temporarily added. */
    Node* begin_node_;
    Node* end_node_;

    int begin_pos_;
    int end_pos_;

    Pager* m_pager_;
};
      

/**
 * @brief Implementation of B+ tree. 
 * Params::key_type and value type must provid get size method.
 * B+ tree can be used as primary key index and secondary index. To support arbitrary types of keys and values, it is implemented using templates
 */
template<typename Params>
class EpiphanyBplusTree {
public:
    typedef typename Params::key_type key_type;
    typedef typename Params::data_type data_type;
    typedef typename Params::value_type value_type;
    typedef typename Params::size_type size_type;
    typedef typename Params::pointer pointer;
    typedef typename Params::const_pointer const_pointer;
    typedef typename Params::reference reference;
    typedef typename Params::const_reference const_reference;
    typedef typename Params::pointer_type  pointer_type;
    
    typedef BptBaseNode<Params> base_node;
    typedef BptInternalNode<Params> internal_node;
    typedef BptLeafNode<Params> leaf_node;
    
    typedef BPlustreeIterator<base_node, reference, pointer> iterator;
    typedef typename iterator::const_iterator const_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
    typedef std::reverse_iterator<iterator> reverse_iterator;

    struct NodeBackLog
    {
        pointer_type  cur_;
        int next_; ///< Backtracking point
    };

public:
    EpiphanyBplusTree(Pager* pager);
    EpiphanyBplusTree(const EpiphanyBplusTree & bt);

    /// Destructor
    ~EpiphanyBplusTree(){
        // write meta to data file.
        size_t arrlen = 0;      
        int h = meta_.height();
    }

public: // provid to user APIs (rebuild version).
    
    /// @brief  Calculate how many nodes a B-tree has with degree n (with consecutive primary keys).
    /// @param  degree
    /// @param  Number of data records.
    /// @return 
    static size_t bpt_calc_nodes(size_t, size_t);

    /// @brief Open B+ tree. You can view the EpiphanyBPlusTree class as a collection of B+ tree algorithms in memory, 
    ///        so for the Open operation, it is actually a process of initializing the root node and setting the necessary parameters of the B+ tree, 
    ///        and separating the expected file operations (which can be separated because the B+ tree and the file are not one-to-one).
    ///        操作预期分离(能分离的原因是B+树和文件不是一一对应的), 
    /// @return BptRet
    BptRet bpt_open(void);

    /// @brief create b+ tree.
    /// @tparam init_param 
    /// @tparam buffer_type 
    /// @param  
    /// @return 
    template<typename buffer_type>
    static EpiphanyBplusTree<Params>* create(buffer_type*);

    /// @brief Insert data.
    /// @param _key 
    /// @param _data 
    /// @return BptRet
    BptRet bpt_insert(key_type _key, data_type _data);
    
    /// @brief Query data (single result).
    /// @return 
    BptRet bpt_search(const key_type&, data_type*);

    /// @brief Single field query, returns an iterator.
    /// @param Value of the target key.
    /// @param Condition operator.
    /// @return A result iterator.
    iterator bpt_search(const key_type&, BptSearchOp);

    /// @brief Delete specified key,
    /// @param  
    /// @return 
    BptRet bpt_delete(const key_type &);

    /// @brief Update
    /// @return 
    BptRet bpt_update(const key_type&, data_type*);

    /// @brief DFS.
    void bpt_dump();

    /// @brief Overload the equals operator.
    /// @param void.s
    void operator=(const EpiphanyBplusTree &);
private: /// Functions used by the B+ tree itself.

    /// @brief Initialize and configure the B+ tree
    /// @return 
    BptRet _bpt_config(void);

    /// @brief Determine if the B+ tree has completed initialization.
    /// @return true / false.
    bool _bpt_initialized(void);

    /// @brief Determine if the tree is empty.
    /// @param  
    /// @return 
    bool _bpt_empty(void);

    /// @brief Initialize the root node.
    /// @return 
    BptRet _bpt_initialize_root();

    /// @brief Split node. Split one node into two nodes.
    /// @return 
    BptRet _bpt_split(pointer_type _split_no, pointer_type* new_leaf_no);

public:
    /// @brief b+ tree iterator begin.
    /// @return 
    iterator begin() { return iterator(locate_begin(), 0, m_pager_); }

    /// @brief b+ tree iterator end.
    /// @return 
    iterator end() { return iterator(locate_end(), base_node::BtDegree, m_pager_); }
    
    /// @brief init b+ tree 
    /// @return void.
    BptRet init();

    int restore(Pgno, epiphany_uint32);

public:
    // read onlys.
    
    /// @brief return b+tree root node page no.
    /// @return pointer_type
    pointer_type root() { return meta_.root(); }

    /// @brief Set the root node page number.
    /// @param _root 
    void set_root(Pgno _root) { meta_.rRoot() = _root; }

    /// @brief set btree height.
    /// @param _height 
    void set_height(int _height) { meta_.rHeight() = _height; }

    /// @brief Return tree height.
    /// @return 
    epiphany_int32 height() { return meta_.height(); }

    /// @brief locate the iterator.
    /// @param is_begin begin flag.
    /// @return 
    base_node* locate_both_ends(bool is_begin);

    /// @brief locate iterator to begin.
    /// @return 
    base_node* locate_begin() { return locate_both_ends(true); }

    /// @brief locate iterator to end.
    /// @return 
    base_node* locate_end() {  return locate_both_ends(false); }

    /// @brief search the parent node of the leaf
    ///        where the target key is located.
    /// @param key 
    /// @return 
    Pgno search_index(const key_type &);

    /// @brief search the leaf node where the target key is located.
    /// @param index 
    /// @param key 
    /// @return 
    Pgno search_leaf(Pgno, const key_type &);

    /// @brief 
    /// @tparam comparator 
    /// @param  
    /// @param  
    /// @param  
    /// @param  
    /// @return 
    template<typename comparator>
    int linearSearch(const key_type &, int, int, const comparator&);

    /// @brief Merge or borrow. Use Pgno as the handle for processing, 
    ///        because this may occur in different types of nodes.
    /// @param  Pgno The page number of the node to be processed.
    /// @return BptRet.
    template <typename HandleNodeType>
    BptRet mergeOrBorrow(pointer_type);

    /// @brief Merge two nodes, occurs when the sibling node cannot borrow.
    /// @param sibling_page_no The page number of the sibling node.
    /// @param current_page_no The page number of the current node.
    /// @param parent_page_no The page number of the parent node.
    /// @param index The position in the parent node.
    /// @return 
    template <typename HandleNodeType>
    BptRet merge(pointer_type, pointer_type, pointer_type, int index);

    /// @brief Borrow members from sibling node when the 
    ///        current node violates B+ tree properties.
    /// @param sibling_page_no The page number of the sibling node.
    /// @param current_page_no The page number of the current node.
    /// @param parent_page_no The page number of the parent node.
    /// @param index The position in the parent node.
    /// @return 
    template <typename HandleNodeType>
    BptRet borrow(pointer_type, pointer_type, pointer_type, int index);

    /// @brief Adjust the root node.
    /// @param  
    /// @return 
    BptRet adjustRoot(pointer_type);

protected:

    /// @brief Get the Node type based on the pointer.
    /// @param  
    /// @return 
    base_node* getNode(pointer_type);

    /// @brief 
    /// @param  
    /// @param  
    /// @param  
    void draw(base_node*, NodeBackLog*, int);
    
private:
    bool m_is_init_;    ///< Whether initialization is complete.
    BptKeyComparator m_key_comparator_;
    Pager* m_pager_;
    BptMeta meta_;       ///< Metadata
};

template<typename P>
template <typename buffer_type> EpiphanyBplusTree<P> *
EpiphanyBplusTree<P>::create(buffer_type *)
{
    return EpiphanyNull;
}

template<typename P>
EpiphanyBplusTree<P>::EpiphanyBplusTree(Pager* pager) 
{
    m_pager_ = pager; 
}

template<typename P> size_t 
EpiphanyBplusTree<P>::bpt_calc_nodes(size_t, size_t)
{
    return 0;
}

template<typename P> BptRet 
EpiphanyBplusTree<P>::bpt_open() 
{
    return _bpt_initialized() ? Btree_Ok : init();
}

/**
 * insert的实现主要分为以下几个步骤:
 *  1. 如果这是一棵空树, 创建一个叶子结点, 这个叶子结点同时也是根结点.
 *  2. 找到应该存放当前key的叶子结点.
 *  3. 判断上一步骤找到的结点是否小于n-1, 如果小于, 直接将key添加到这个结点中.(函数返回).
 *  4. 如果上一步骤没有返回,则说明结点已满, 则进行以下步骤:
 *     1). 创建一个新的结点.将当前结点的一半元素拷贝到新结点中.
 *     2). 将key放入合适的结点中(根据查询返回的pos).
 *     3). 将新结点中最小值和新结点的指针添加到父结点中(函数返回).
 */
template<typename P> BptRet 
EpiphanyBplusTree<P>::bpt_insert(key_type _key, data_type _data) 
{
    /** key是否存在于结点中. */
    bool key_exists = false;

    /** key存放的位置. */
    int insert_pos = 0;

    /** step1: 如果树是空的. 创建一个叶子节点, 并作为根结点. */
    if (_bpt_empty()) {
        _bpt_initialize_root();
    }

    /**
     * step2: 首先需要确定当前key应该存放在哪个叶子页面上.
     * Search leaf node's parent node from root node.
     */
    Pgno parent_pgno = search_index(_key);
    Pgno offset = search_leaf(parent_pgno, _key);

    /** 创建一个叶子页面的内存表示, 即leaf_node. */
    leaf_node leaf(m_pager_);
    leaf.deserializable(offset);

    /** 当拿到一个已经反序列化完成的叶子结点, 首要判断当前key在结点中是否存在, 这也取决与B+树
     *  的类型, 如果设置了约束(唯一), 则直接返回.
     */
    // geted target node, check the same key.
    insert_pos = leaf.upper_bound(_key, &key_exists);
    if (key_exists) {
        return Btree_InsertExistKey;
    }

    /**
     * step3: 处理叶子结点满了的情况(满的标志为(n >= d - 1)). 需要进行分裂.
     */
    if (BptreeFull(leaf.count(), leaf.degree())) {

        pointer_type new_leaf_pgno = InvalidPgno;

        /** 调用分裂函数, 待分裂结点的另一半内容分裂到新结点中. */
        _bpt_split(offset, &new_leaf_pgno);

        /** 如果新结点的页号仍然是无效的页号, 则说明分裂失败, 直接退出. */
        if (InvalidPgno == new_leaf_pgno) {
            return Btree_OpErrorPgno;
        }   

        /** 创建新的叶子结点, 此时结点中有了分裂之后的数据. */
        leaf_node new_leaf(m_pager_);
        new_leaf.deserializable(new_leaf_pgno);

        /** 根据查找到的下标选择应该存放的结点(左或右). */
        if (insert_pos < leaf.count()) {
            new_leaf.insert_record_no_split(_key, _data);
        } else {
            leaf.insert_record_no_split(_key, _data);
        }

        /** 操作完两个结点后, 进行序列化. */
        leaf.serializable(offset);
        new_leaf.serializable(new_leaf_pgno);
        
        /** 由于分裂的原因, 需要调整主结点. */
        internal_node parent(m_pager_);
        parent.deserializable(parent_pgno);
        parent.insert_key_to_index(meta_, *new_leaf.keyAt(0), 
                                   offset, leaf.next());

    } else { 
        /** 叶子结点没有占满, 直接insert. */
        leaf.insert_record_no_split(_key, _data);
        leaf.serializable(offset);
    }
    
    return Btree_Ok;
}

/// @brief TODO:这个接口参数设计是暂时的, 传入指针的方法可能导致内存越界, 后续查询接口应该
///        返回结果集结构,需要和查询引擎对接.
template<typename P> BptRet 
EpiphanyBplusTree<P>::bpt_search(const key_type& key, data_type* res) 
{
    Pgno parent_pgno = search_index(key);
    Pgno offset = search_leaf(parent_pgno, key);

    leaf_node leaf(m_pager_);
    leaf.deserializable(offset);

    bool found = false;
    size_t pos = leaf.upper_bound(key, &found);
    if (found) {
        ndbMemCpy(res, leaf.dataAt(pos), leaf.dataAt(pos)->recSize());
        return Btree_Ok;
    }
    return Btree_SearchNotFoundKey; 
}

template<typename P> 
typename EpiphanyBplusTree<P>::iterator 
EpiphanyBplusTree<P>::bpt_search(const key_type& key, BptSearchOp op)
{
    iterator ret;
    Pgno parent_pgno = search_index(key);
    Pgno offset = search_leaf(parent_pgno, key);

    leaf_node leaf(m_pager_);
    leaf.deserializable(offset);

    bool found = false;
    size_t pos = leaf.upper_bound(key, &found);
    if (found) {
        return Btree_Ok;
    }
    return ret;
}



/** 1. Search for the leaf node: Use the search_leaf function to find the 
 *     leaf node that contains the key to be deleted.
 * 
 *  2. Delete the key: Remove the key from the leaf node.
 * 
 *  3. Adjust the tree structure: If the number of keys in the leaf node is less 
 *     than the minimum value after deletion, 
 *     merge or borrow operations need to be performed to maintain the balance 
 *     of the B+ tree.
 *  4. 删除可能会导致在非叶结点中存在叶结点中并不存在的key.
 */
template<typename P> BptRet 
EpiphanyBplusTree<P>::bpt_delete(const key_type &key) 
{
    pointer_type parent_pgno = search_index(key);

    /** Search for the leaf node: Use the search_leaf function to find 
     *  the leaf node that contains the key to be deleted.  
     */
    pointer_type leaf_pgno = search_leaf(parent_pgno, key); 

    /** 没有查找到需要删除的key. */
    EpiphanyCheckRtCode(leaf_pgno != InvalidPgno, Btree_DeleteNotFoundKey);

    /** Deserialize the leaf node. */
    BptLeafNode<P> leaf(this->m_pager_);
    leaf.deserializable(leaf_pgno);
    
    /** Delete key, which may cause the B+ tree to become unbalanced. 
     *  For leaf nodes, only when the number of records in the leaf is 
     *  less than degree / 2, it does not satisfy the properties of the B+ tree. 
     *  In this case, it needs to borrow from sibling nodes to satisfy its 
     *  properties.
     * 
     * Handling this problem includes the following two cases:
     * 1. (Borrow): When the sibling node has enough members that 
     *     can be borrowed (this.childs + brother.childs > d)
     * 2. (Merge): When the sibling node cannot borrow anymore 
     *    (this.childs + brother.childs > d)
     */

    bool search_result = false;
    size_t remove_index = leaf.lower_bound(key, &search_result);
    
    if (search_result) {
        /** 在叶子结点中移除目标key. */
        leaf.remove(remove_index);
        leaf.serializable(leaf_pgno);
    } else { 
        return Btree_DeleteNotFoundKey;
    }

    /** 检查是否需要借用或者合并, 以满足B+树的性质. */
    this->mergeOrBorrow<leaf_node>(leaf_pgno);

    /** Check if underflow needs to be handled */
    return Btree_Ok;
}

template<typename P> BptRet 
EpiphanyBplusTree<P>::bpt_update(const key_type& target, data_type* new_data) {
    EpiphanyAssertAndCheckRtCode(new_data, Btree_OpNullPtr);
    BptRet rc = bpt_delete(target); 
    EpiphanyCheckRtCode(rc == Btree_Ok, rc);
    rc = bpt_insert(target, new_data);
    EpiphanyCheckRtCode(rc == Btree_Ok, rc);
    return Btree_Ok;
}

template<typename P> void EpiphanyBplusTree<P>::bpt_dump() 
{
    /** Initialize the level to 0 and start traversing from the root node. */
    int level = 0;
    pointer_type node_pt = this->root();
    EpiphanyAssert(node_pt);

    NodeBackLog *p_nbl = EpiphanyNull;

    /** Simulate the recursive call stack, 
     *  storing node and its next child node to be visited. */
    NodeBackLog nbl_stack[200];
    NodeBackLog *top = nbl_stack;

    /** Perform a level order traversal to ensure processing 
     *  of nodes in each level. */
    pointer_type *queue = new pointer_type[20000000];
    int front = 0;
    int rear = 0;
    queue[rear++] = this->root();

    /**BFS */
    while (front < rear) {
        base_node *tnode = getNode(queue[front++]);

        /** Add all child nodes of the current node to the queue. */
        for (int i = 0; i < tnode->count(); i++) {
            if (!tnode->is_leaf()) {
                queue[rear++] = *(tnode->to_internal_node()->childAt(i));
            }
        }
    }

    base_node* node;
    
    /**DFS */
    for(;;) {
        if (node_pt != 0) {
            node = getNode(node_pt);
            int sub_idx = p_nbl != EpiphanyNull ? p_nbl->next_ : 0;
            p_nbl = EpiphanyNull;
            if (node->is_leaf() || sub_idx + 1 > node->count()) {
                top->cur_ = 0;
                top->next_ = 0;
            } else {
                top->cur_ = node_pt;
                top->next_ = sub_idx + 1;
            }
            top++;
            level++;
            if (sub_idx == 0) {
                draw(node, nbl_stack, level);
            }
            if (!node->is_leaf())
                node_pt = *(node->to_internal_node()->childAt(sub_idx));
            else
                node_pt = 0;
        } else {
            p_nbl = top == nbl_stack ? EpiphanyNull : --top;
            if (p_nbl == EpiphanyNull) {
                break;
            }
            node_pt = p_nbl->cur_;
            level--;
        }
    }
    printf("-------------------------------------------\n");
}

template<typename P> inline void 
EpiphanyBplusTree<P>::operator=(const EpiphanyBplusTree &r)
{
    this->m_key_comparator_ = r.m_key_comparator_;
    this->meta_ = r.meta_;
    this->m_pager_ = r.m_pager_;
}

template<typename P> BptRet 
EpiphanyBplusTree<P>::_bpt_config()
{

    return Btree_Ok;
}

template<typename P> bool 
EpiphanyBplusTree<P>::_bpt_initialized(void)
{
    return m_is_init_;
}

template<typename P> bool 
EpiphanyBplusTree<P>::_bpt_empty(void)
{
    return InvalidPgno == root();
}

template<typename P> BptRet 
EpiphanyBplusTree<P>::_bpt_initialize_root()
{
    EpiphanyCheckRtCode(EpiphanyNull != m_pager_, Btree_OpNullPtr);

    leaf_node root_node(m_pager_);

    root_node.reference_count() = 0;

    root_node.reference_is_root() = 
    root_node.reference_is_leaf() = true;

    /** 设置root的父结点为无效的页面. */
    root_node.reference_parent() = InvalidPgno;
    
    /** 为root申请一个页面. */
    pointer_type root = m_pager_->allocatePage();
    EpiphanyAssertAndCheckRtCode(InvalidPgno != root, Btree_OpInvalidPage);

    /** 设置root节点自己的页号. */
    root_node.reference_self_pgno() = root;
    set_root(root);
    
    /** 将root结点的内容序列化到页面上. */
    root_node.serializable(meta_.root());

    return Btree_Ok;
}

template<typename P> BptRet 
EpiphanyBplusTree<P>::_bpt_split(pointer_type _split_no, pointer_type* new_leaf_no)
{
    BptRet rc = Btree_Ok;
    key_type k;
    pointer_type parent_pgno = InvalidPgno;

    /** 获取待分裂节点. */
    leaf_node leaf(m_pager_);
    leaf.deserializable(_split_no);
    
    parent_pgno = leaf.parent();

    /** 创建一个分裂出来的新叶子结点. */
    leaf_node new_leaf(m_pager_);

    /** 为新叶子结点申请页面. */
    *new_leaf_no = m_pager_->allocatePage();

    // start split node.
    leaf.split(new_leaf);
    leaf.reference_next() = *new_leaf_no;
    new_leaf.reference_prev() = _split_no;
    return Btree_Ok;
}

template<typename P> inline BptRet 
EpiphanyBplusTree<P>::init() 
{
    BptRet rc = Btree_Ok;
    internal_node* root_node = new internal_node(m_pager_);
    EpiphanyAssertAndCheckRtCode(root_node, Btree_OpNullPtr);
    meta_.rHeight() = 0;
    return rc;
}

template<typename P> inline int
EpiphanyBplusTree<P>::restore(Pgno root_no, epiphany_uint32 h) 
{
    EpiphanyAssert(root_no != 0);
    meta_.rRoot() = root_no;
    meta_.rHeight() = h;
    return 0;
}

template<typename P> Pgno 
EpiphanyBplusTree<P>::search_index(const key_type &key) 
{
    BptRet rc = Btree_Ok;
    pointer_type org = root();
    int height = meta_.height();

    while(height > 1) {
        internal_node node(m_pager_);
        node.deserializable(org);
        int pos = node.upper_bound(key);

        rc = node.child(pos, org);
        EpiphanyAssert(rc == Btree_Ok);
        if (Btree_Ok != rc ) { 
            return InvalidPgno;
        }       
        --height;
    }
    return org;
}

template<typename P> Pgno 
EpiphanyBplusTree<P>:: search_leaf(Pgno index, const key_type &key) 
{ 
    internal_node node(m_pager_);
    Pgno ret = -1;
    BptRet rc = Btree_Ok;
    node.deserializable(index);
    int pos = node.upper_bound(key);

    rc = node.child(pos, ret);
    EpiphanyAssert(rc == Btree_Ok);
    if ( Btree_Ok != rc ) {
        return InvalidPgno;
    }
    return ret;
}

template <typename P> BptBaseNode<P>*
EpiphanyBplusTree<P>::locate_both_ends(bool is_begin)
{
    BptRet rc = Btree_Ok;
    if (!is_begin)
    {
        return EpiphanyNull;
    }
    base_node *current = new internal_node(this->m_pager_);
    current->to_internal_node()->deserializable(root());

    size_t h = meta_.height();
    while (current != NULL)
    {
        if (h > 1)
        {
            pointer_type p;
            base_node *t = new internal_node(this->m_pager_);

            rc = current->to_internal_node()->child(is_begin ? 0 : 
                                                    current->count(), p);
            EpiphanyAssert(rc == Btree_Ok);
            t->to_internal_node()->deserializable(p);

            if (t)
            {
                delete current;
                current = t;
                --h;
            }
            else
            {
                break;
            }
        }
        else
        {
            pointer_type p;
            base_node *t = new leaf_node(this->m_pager_);
            rc = current->to_internal_node()->child(is_begin ? 0 : 
                                                    current->count(), p);
            EpiphanyAssert(rc == Btree_Ok);
            t->to_leaf_node()->deserializable(p);
            if (t)
            {
                /** 结点中没有数据的情况. */
                if (t->count() <= 0) {
                    delete t;
                    current = EpiphanyNull;
                    break;
                }
                delete current;
                current = t;
                break;
            }
            else
            {
                break;
            }
        }
    }
    // assert(current->is_leaf());
    return current;
}

template <typename P>
template <typename comparator> int 
EpiphanyBplusTree<P>::linearSearch(const typename EpiphanyBplusTree<P>::key_type &key, 
                              int start, int end, const comparator &comp)
{
    while (start < end)
    {
        int c = comp(start, key);
        if (c == 0)
        {
            return start;
        }
        else if (c > 0)
        {
            break;
        }
        ++start;
    }
    return start;
}

/// @brief 
template <typename P>
template <typename HandleNodeType> BptRet 
EpiphanyBplusTree<P>::mergeOrBorrow(pointer_type current_pt) 
{
    EpiphanyAssertAndCheckRtCode(current_pt > 0, Btree_OpErrorPgno);

    HandleNodeType node(m_pager_);
    node.deserializable(current_pt);

    /** 如果当前结点符合B+树的性质, 无需任何调整, 直接退出返回成功. */
    if (node.count() >= node.min_size()) {
        return Btree_Ok;
    }

    /**
     * 父结点是无效的指针,说明当前结点是根节点.
     * 根结点有一个特殊情况, 就是如果根结点没有任何key时, 需要进行调整.
     */
    if (InvalidPgno == node.parent()) {
        if (node.count() <= 1) {

            /** 调整根结点. */
            adjustRoot(current_pt);
        }
        return Btree_Ok;
    }

    /** Get the parent node. */
    internal_node parent_node(m_pager_);
    parent_node.deserializable(node.parent());
    
    /** Get the position of the current node in the parent node. */
    size_t index = node.pos();

    /** Find a sibling node. Unless the current node index is 0, 
     *  it always looks to the left. */
    pointer_type neighbor_pt;

    index == 0 ? neighbor_pt = *(parent_node.childAt(index + 1)) :
    neighbor_pt = *(parent_node.childAt(index - 1));

    if (neighbor_pt == InvalidPgno) {
        return Btree_Ok;
    }

    /** Get the sibling node. */
    HandleNodeType neighbor_node(m_pager_);
    neighbor_node.deserializable(neighbor_pt);

    if (node.count() + neighbor_node.count() >= node.max_size()) {
        /** The sibling node has enough members to be borrowed. */
        borrow<HandleNodeType>(neighbor_pt, current_pt, 
                               node.parent(), node.pos());
    } else {
        merge<HandleNodeType>(neighbor_pt, current_pt, 
                              node.parent(), node.pos());
    }
    return Btree_Ok;
}

template <typename P>
template <typename HandleNodeType> BptRet 
EpiphanyBplusTree<P>::merge(pointer_type neighbor, 
                       pointer_type current, 
                       pointer_type parent, int index) 
{
    EpiphanyAssertAndCheckRtCode(neighbor > 0 && current > 0 && parent > 0, 
                            Btree_OpErrorPgno);

    internal_node parent_node(m_pager_);
    parent_node.deserializable(parent);

    /** Since the B+ tree is a sorted tree, we need to distinguish between 
     *  left and right during merging. */
    pointer_type left_pt, right_pt;

    bool op_right = false;
    if (index == 0) {
        left_pt = current;
        right_pt = neighbor;
        index++;
        op_right = false;
    } else {
        op_right = true;
        left_pt = neighbor;
        right_pt = current;
    }
    
    HandleNodeType left_node(m_pager_);
    HandleNodeType right_node(m_pager_);

    left_node.deserializable(left_pt);
    right_node.deserializable(right_pt);

    BptRet rc = right_node.move_to(&left_node);
    
    /**
     * 处理非叶子结点.
     */
    if (!left_node.is_leaf()) {
        if (!op_right) {
            // 合并导致的key下溢.
            left_node.pushHeadKey((index - 1), *(parent_node.keyAt(index - 1)));
        } else {
            left_node.insert_key(left_node.count(), 
                                 *(parent_node.keyAt(index - 1)));
        }
    }
    left_node.serializable(left_pt);

    /**
     * 合并之后其父结点少一个child. 使用remove函数移除掉.
     */
    parent_node.remove(index); 
    parent_node.serializable(parent);

    /** Check upwards recursively. */
    return mergeOrBorrow<internal_node>(parent);
}

 /// 结点借用, 发生在所有类型的结点中,删除可能导致树结构的改变-"下溢", 
template <typename P>
template <typename HandleNodeType> BptRet 
EpiphanyBplusTree<P>::borrow(pointer_type neighbor, 
                        pointer_type current, 
                        pointer_type parent, int index) 
{
    EpiphanyAssertAndCheckRtCode(neighbor > 0 && current > 0 && parent > 0, 
                            Btree_OpErrorPgno);

    internal_node parent_node(m_pager_);
    parent_node.deserializable(parent);

    HandleNodeType current_node(m_pager_);
    HandleNodeType neighbor_node(m_pager_);

    current_node.deserializable(current);
    neighbor_node.deserializable(neighbor);

    if (neighbor_node.count() < current_node.count()) {
        return Btree_StructError;
    }

    /** The difference between the left neighbor and the right neighbor. */
    if (index == 0) {

        /** Right neighbor scenario, need to borrow the first element 
         *  from the right neighbor. */
        neighbor_node.moveHead2Tail(&current_node);

        /** Set the parent node key */
        parent_node.reference_key(index) = *(neighbor_node.keyAt(0));
    } else {

        /** Left neighbor scenario, need to borrow the last element 
         *  from the left neighbor. */
        neighbor_node.moveTail2Head(&current_node);
        
        /** Set the parent node key */
        parent_node.reference_key(index) = *(current_node.keyAt(0));
    }
    /**
     * Here we need to mark the page as dirty.
     */
    current_node.serializable(current);
    neighbor_node.serializable(neighbor);
    parent_node.serializable(parent);

    return Btree_Ok;
}

template <typename P> BptRet 
EpiphanyBplusTree<P>::adjustRoot(pointer_type root_pt) {
    EpiphanyAssert(root_pt != InvalidPgno);

    BptBaseNode<P> base_node(m_pager_);
    base_node.deserializable(root_pt);

    /** 根结点退化成叶, 根结点比较特殊它允许存在1成员的情况而忽略B+树的性质. */
    if (base_node.is_leaf() && base_node.count() > 0) {
        return Btree_Ok;
    }

    /** 调整根结点, 重新申请一个新的结点. */
    pointer_type new_root_pt;

    if (base_node.is_leaf()) {
        EpiphanyAssert(base_node.count() == 0);
        new_root_pt = InvalidPgno;
    } else {
        /** 当根结点只有一个子结点时, 违反了B+树的性质, 这个子结点就成为了根结点 */
        internal_node internal(m_pager_);
        internal.deserializable(root_pt);

        pointer_type child_pt = *(internal.childAt(0));
        BptBaseNode<P> child_node(m_pager_);
        child_node.deserializable(child_pt);

        child_node.reference_is_leaf() = false;
        child_node.reference_parent() = InvalidPgno;
        child_node.serializable(child_pt);
        new_root_pt = child_pt;
        meta_.rHeight()--;
    }

    meta_.rRoot() = new_root_pt;
    return Btree_Ok;
}

template <typename P>
typename EpiphanyBplusTree<P>::base_node* 
EpiphanyBplusTree<P>::getNode(pointer_type pt) 
{
    base_node *node;

    PageDescriptor* pbuf = (PageDescriptor*)(m_pager_->getPageBuffer(pt));
    if (pbuf->is_leaf_) {
        node = new leaf_node(m_pager_);
        node->to_leaf_node()->deserializable(pt);
    } else {
        node = new internal_node(m_pager_);
        node->to_internal_node()->deserializable(pt);
    }
    return node;
}

template <typename P> void 
EpiphanyBplusTree<P>::draw(base_node *node, NodeBackLog *stack, int level)
{
    int i = 0;
    for (i = 1; i < level; i++) {
        if (i == level - 1) {
            printf("%-2s", "+----");
        }
        else {
            if (stack[i - 1].cur_ != EpiphanyNull) {
                printf("%-2s", "|");
            }
            else {
                printf("%-2s", "");
            }
        }
    }
    node->keyDump();
}

/// B+树结构验证器.
/// 1.根节点的子节点数量是否满足要求.
/// 2.非叶子节点的子节点数量是否满足要求.
/// 3.所有叶子节点是否在同一层.
/// 4.所有叶子节点的指针是否正确链接.
/// 5.所有非叶子节点的关键字是否严格递增.
template<typename Params>
class EpiphanyBptValidator {
    typedef EpiphanyBplusTree < Params > epiphany_btree;
    typedef BptBaseNode < Params > base_node;
    typedef BptInternalNode < Params > internal_node;
    typedef BptLeafNode < Params > leaf_node;
    typedef typename Params::key_type key_type;
    typedef typename Params::pointer_type pointer_type;
    typedef typename internal_node::Iterator internal_iterator;
    typedef typename leaf_node::Iterator leaf_iterator;
public:
    EpiphanyBptValidator(epiphany_btree* bpt) : m_bpt_(bpt) {}
public:
    /// @brief BFS.
    /// @return 
    bool validate();

protected:
    /// @brief 验证叶子结点.
    /// @param  
    /// @return 
    bool verifyLeafNode(const leaf_node&);
    
    /// @brief 验证内部结点.
    /// @param  
    /// @return 
    bool verifyInternalNode(const internal_node &);

private:
    epiphany_btree* m_bpt_;
    int m_order_;
};

template <typename Params>
bool EpiphanyBptValidator<Params>::validate()
{
    std::stack<pointer_type> nodeStack;

    /** 根结点入栈. */
    nodeStack.push(m_bpt_->root());

    int leafLevel = 0;
    bool firstLeaf = true;
    pointer_type prevLeaf;

    /** 广度优先. */
    while (!nodeStack.empty())
    {
        pointer_type node_pt = nodeStack.top();
        nodeStack.pop();

        /** 第一次循环pop的node_pt是root结点. */
        base_node* node = m_bpt_->getNode(node_pt);

        if (node->isLeaf())
        {
            if (firstLeaf)
            {
                leafLevel = nodeStack.size();
                firstLeaf = false;
            }
            else if (nodeStack.size() != leafLevel)
            {
                return false; // Leaf nodes not at the same level
            }

            if (prevLeaf != EpiphanyNull && prevLeaf->next()!= node)
            {
                return false; // Leaf nodes not correctly linked
            }
            prevLeaf = node;
            
            leaf_node leaf;
            leaf.deserializable(node_pt);
            if (!verifyLeafNode(leaf))
            {
                return false;
            }
        }
        else
        {   
            internal_node internal;
            internal.deserializable(node_pt);
            if (!verifyInternalNode(internal))
            {
                return false;
            }

            for (internal_iterator it = internal.begin(); 
                 it != internal.end(); it++) 
            {
                nodeStack.push((*it)->child_);
            }
        }
    }
    return false;
}

template <typename Params>
bool EpiphanyBptValidator<Params>::verifyLeafNode(const leaf_node &node) 
{
    /** 叶子结点成员数量. */
    EpiphanyAssertAndCheckRtCode(node.count() >= node.min_size(), false);
    EpiphanyAssertAndCheckRtCode(node.count() <= node.max_size(), false);

    /** 验证叶子节点的key是否严格递增. */
    key_type k;
    for (leaf_iterator it = node.begin(); it != node.end(); it++) {
        if(k > (*it).key_) {
            return false;
        }
        k = (*it).key_;
    }

    /** 向后看一眼, 是否小于next结点的第一个成员. */
    base_node* next = m_bpt_->getNode(node.next());
    if (next != EpiphanyNull) {
        if (*(key_type*)(next->keyAt(0)) < k) {
            return false;
        }
    }
    return true;
}

template <typename Params>
bool EpiphanyBptValidator<Params>::verifyInternalNode(const internal_node &node) 
{ 
    EpiphanyAssertAndCheckRtCode(node.count() >= node.min_size(), false);
    EpiphanyAssertAndCheckRtCode(node.count() <= node.max_size(), false);

    /** 内部结点的key也是严格递增的. */
    key_type k;
    for (leaf_iterator it = node.begin(); it != node.end(); it++) {
        if(k > (*it).key_) {
            return false;
        }
        k = (*it).key_;
    }
    return true; 
}

/*------------------------End define Btree functions--------------------------*/
}

/**
 * graphical debug for browser.
 */
#ifdef GRAPHICAL_DEBUG
extern "C" {
    EMSCRIPTEN_KEEPALIVE
    void insert(int key) {
        tree.insert(key);
    }

    EMSCRIPTEN_KEEPALIVE
    void remove(int key) {
        tree.remove(key);
    }

    EMSCRIPTEN_KEEPALIVE
    const char* toJson() {
        static std::string json;
        json = tree.toJson();
        return json.c_str();
    }
}
#endif //! GRAPHICAL_DEBUG

/*-------------------------------Test Part------------------------------------*/
namespace BptTest {

class BptBenchMarker
{
public:
    BptBenchMarker(const char*, void (*)(int));

public:
    void run();
    void stop();
    void start();
    void reset();

    BptBenchMarker* next();
private:
    BptBenchMarker* m_next_bench_mark_;
    const char* m_name_;
    void (*benchmark_func)(int);
    long long m_accum_micros_;
    long long m_last_started_;
};

static BptBenchMarker *first_benchmark;
static BptBenchMarker *current_benchmark;

inline void 
stopBmTiming() 
{
    current_benchmark->stop();
}

inline void 
startBmTiming () 
{
    current_benchmark->start();
}

inline void 
runBenchMarks()
{
    for (BptBenchMarker* bm = first_benchmark; bm; bm = bm->next()) {
        bm->run();
    }
}

inline void 
BptBenchMarker::start()
{
    EpiphanyAssert(m_last_started_ != 0);
    //m_last_started_ = EpiphanyNow::msec();
}

inline void
BptBenchMarker::stop()
{
    if (0 == m_last_started_) {
        return;
    }
    //m_accum_micros_ += EpiphanyNow::msec() - m_last_started_;
    m_last_started_ = 0;
}

inline void
BptBenchMarker::reset()
{
    m_last_started_ = 0;
    m_accum_micros_ = 0;
}


inline void
BptBenchMarker::run()
{
    EpiphanyAssert(current_benchmark == EpiphanyNull);
    current_benchmark = this;
    // int iters = 

}

inline BptBenchMarker::BptBenchMarker(const char* name, void (*func)(int))
: m_next_bench_mark_(first_benchmark),
  m_name_(name),
  benchmark_func(func),
  m_accum_micros_(0),
  m_last_started_(0)
{
    first_benchmark = this;
}

/** Basic test for B+ tree. */
Pager* pager_constructor();
int bpt_base_test();

inline Pager* 
pager_constructor() {
    /** Construct a pager. */
    Pager* pager_ = new Pager();
    EpiphanyAssert(pager_);

    char* addr = (char*)ndbMalloc(256 * TZDB_FILE_PAGE_SIZE);

    PagerConfig pconf;
    pconf.SetDbName("test_bpt_int_char");
    pconf.SetNPages(256);
    pconf.SetPageSize(TZDB_FILE_PAGE_SIZE);
    pconf.SetPgsBaseAddr(addr);
    
    // Get the length of the path and name
    size_t path_len = strlen("/home/ulis/data/");
    size_t name_len = strlen("test_bpt_int_char");
    size_t ext_len = strlen(".xxx");

    size_t full_path_len = path_len + name_len + ext_len + 1;

    // Allocate memory
    char *filePath = (char *)ndbMalloc(full_path_len);

    // Copy the path, name, and extension
    epiphany_strncpy(filePath, "/home/ulis/data/", strlen("/home/ulis/data/"));
    epiphany_strncat(filePath, full_path_len, "test_bpt_int_char");
    epiphany_strncat(filePath, full_path_len, ".xxx");
    filePath[full_path_len - 1] = '\0';
    
    pconf.SetDataFilePath(filePath);
    pager_->init(pconf);
    return pager_;
}

/** Test key(int), value(char*) type.
 * But recSize and recBuf need to be implemented for any type,
 * otherwise it will not compile.
 */
class IntKey
{
public:
    int recSize() const { return sizeof(int); }

    const char *recBuf() const { return (const char *)&m_key_; }

private:
    int m_key_;
};

class CharPValue
{
public:
    CharPValue() {}
    CharPValue(char *_buf, size_t len)
    {
        m_buf_ = (char *)ndbMalloc(len);
        ndbMemCpy(m_buf_, _buf, len);
        m_len_ = len;
    }
    int recSize() const { return m_len_; }

    const char *recBuf() const { return m_buf_; }

    void setRecBuf(char *_buf) { m_buf_ = _buf; }

    void setRecSize(size_t _size) { m_len_ = _size; }

private:
    char *m_buf_;
    int m_len_;
};

inline unsigned long long rand_number_upto(int max)
{
#if defined(__linux__)
    srand(time(NULL));
    return rand() % (max + 1);
#else
    return 0;
#endif 
}

inline int
bpt_base_test() {
    printf("\n Start running BPlustree Base test.\n");

    /** insert数据默认10w条. */
    #define INSERT_RECORD_NUMBER 5

    /** 定义一个本次测试的B+树类型. */
    typedef BtreeSpace::EpiphanyBplusTree
    <BtreeSpace::bplustree_params
    /* keyType, ValueType, PointerType, PageSize */
    < int, CharPValue, Pgno, TZDB_FILE_PAGE_SIZE > > epiphany_btree;

    BtreeSpace::BptRet ret = BtreeSpace::Btree_Ok;

    /** 构造一个Pager给B+树. */
    Pager* pager = pager_constructor();

    epiphany_btree* bpt = new epiphany_btree(pager);

    /** B+树创建. */
    BtreeSpace::BptRet rc = bpt->bpt_open();
    EpiphanyAssert(BtreeSpace::Btree_Ok == rc);

    char buf[128];
    *(int*)buf = 128;

    bpt->bpt_dump();

    int i = 2;

    // test delete example1 delete first key when degree is 3.
    // ins:  i 5  #insert 5 record.
    //       d 2  #delete  key is '2' record.

    // result(i 5):
    //Internal Node:p(15) 4
    // +---- Internal Node:p(9) 3
    // | +---- Leaf Node:p(10) 2
    // | +---- Leaf Node:p(11) 3
    // +---- Internal Node:p(14) 5
    //   +---- Leaf Node:p(12) 4
    //   +---- Leaf Node:p(13) 5 6

    // true result(d 2):
    //  Internal Node:p(9) 4 5
    //   +---- Leaf Node:p(10) 3
    //   +---- Leaf Node:p(12) 4
    //   +---- Leaf Node:p(13) 5 6
    while (1) {
        char input[10];
        printf("%-5s%-5s",">>>please input key: \ninsert -> (i {num}).\n",
        "delete -> (d {num}).\n");

        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0';

        char command = input[0];
        int number = 0;

        if (strlen(input) > 2) {
            number = atoi(&input[2]);
        }

        switch (command)
        {
            case 'i':
            {
                printf("run insert %d records.\n", number);

                // 使用自增的i作为key来模拟主键.
                while(number--) {
                    bpt->bpt_insert(i, CharPValue(buf, 64));
                    i++;
                }
                bpt->bpt_dump();
                break;
            }
            case 'd':
            {
                printf("run delete key: %d.\n", number);
                bpt->bpt_delete(number);
                bpt->bpt_dump();
                break;
            }
        }
    }

    /** 删除测试. */
    /** 删除一个指定键. */
    int target = rand_number_upto(INSERT_RECORD_NUMBER);

    // 先查找有没有
    CharPValue single_search_res = CharPValue(buf, 64);
    ret = bpt->bpt_search(target, &single_search_res);

    printf("\033[32m[info]\033[0m: Delete Target Key : %d.\n", target);

    bpt->bpt_delete(target);

    bpt->bpt_dump();

    /** 进行一次查询(带结果的接口). */
    single_search_res = CharPValue(buf, 64);
    ret = bpt->bpt_search(target, &single_search_res);

    // EpiphanyAssert(BtreeSpace::Btree_SearchNotFoundKey == ret);

    bpt->bpt_dump();

    pager->closeBuffer();

    return 0;
}

};

namespace BptExamples {
/// 前提条件:
/**
 * 这是一个KV类型的B+树, 对于存储的key和value类型, 必须为其实现recSize和recBuf两个函数.并
 * 需要在必要时指定比较函数.
 */
/// 创建
inline void example_create() 
{
}

/// 插入
inline void example_insert()
{
}

/// 查询
inline void example_search()
{
}

/// 删除
inline void example_delete()
{
}

/// 更新.
inline void example_update()
{
}

};

#endif //AGILORE_BTREE_H
