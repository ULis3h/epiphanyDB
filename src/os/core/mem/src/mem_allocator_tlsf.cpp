#include "include/mem_allocator_tlsf.hpp"

#if defined(MEM_MANAGER_64)
#define ALIGN_SIZE_LOG2 3
#define FL_INDEX_MAX 32
#else 
#define ALIGN_SIZE_LOG2 2
#define FL_INDEX_MAX 30
#endif //! MEM_MANAGER_64

/** Alignment byte count. */
#define ALIGN_SIZE (1 << ALIGN_SIZE_LOG2)

/** Base-2 logarithm of SL_INDEX_COUNT. */
#define SL_INDEX_COUNT_LOG2 5

/** Number of second-level indices. */
#define SL_INDEX_COUNT (1 << SL_INDEX_COUNT_LOG2)

/** First-level index shift value. */
#define FL_INDEX_SHIFT (SL_INDEX_COUNT_LOG2 + ALIGN_SIZE_LOG2)

#define FL_INDEX_COUNT (FL_INDEX_MAX - FL_INDEX_SHIFT + 1)

/** Small memory block threshold. */
#define SMALL_BLOCK_SIZE (1 << FL_INDEX_SHIFT)

#define tlsf_cast(t, exp) ((t) (exp))


#if defined (MEM_MANAGER_64)
int tlsf_fls_sizet(size_t size) {
    int high = (int) ( size >> 32);
    int bits = 0;

    if ( high ) {
        bits = 32 + 
    }
}

#endif //! MEM_MANAGER_64

/**
 * @brief Free block definition.
 */
typedef struct block_header_t
{
    u_int32_t size_;

    /** Pointer to the previous physical block header. */
    block_header_t* prev_phy_block_;

    /** Next free block. */
    block_header_t* next_free_; 
    
    /** Previous free block. */
    block_header_t* prev_free_;
} block_header_t;

/**
 * The size_ field is always a multiple of 4, so the last two bits are always free and used to store the busy or free status of the current block and the previous block.
 */
static const size_t block_header_free_bit = 1 << 0;
static const size_t block_header_prev_free_bit = 1 << 1;

MemAllocatorTlsf::MemAllocatorTlsf(void* mem, size_t size)
{
}

MemAllocatorTlsf::~MemAllocatorTlsf()
{
}

void 
MemAllocatorTlsf::mapping_insert(size_t size, int* fli, int* sli)
{
    int fl = 0;
    int sl = 0;
    
    /** Handle small blocks. */    
    if ( size < SMALL_BLOCK_SIZE ) {

        /**
         * Calculate second-level index.
         * 
         * Principle:
         *  - Assuming SMALL_BLOCK_SIZE = 256, SL_INDEX_COUNT = 32
         *  - Then SMALL_BLOCK_SIZE / SL_INDEX_COUNT = 8
         *  - This gives us 8 bytes as one interval.
         */
        sl = tlsf_cast(int, size) / (SMALL_BLOCK_SIZE / SL_INDEX_COUNT);
    } 
    
    /** Regular block handling. */
    else { 
    }
}

