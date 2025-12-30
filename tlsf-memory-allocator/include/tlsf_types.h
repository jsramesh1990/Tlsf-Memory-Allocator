#ifndef TLSF_TYPES_H
#define TLSF_TYPES_H

#include <stddef.h>    /* size_t */
#include <stdint.h>    /* uint32_t, uint64_t */

/* 
 * Platform-independent type definitions
 * These ensure consistent behavior across different architectures
 */

#if defined(__GNUC__) || defined(__clang__)
#define TLSF_ATTR_PACKED __attribute__((packed))
#define TLSF_ATTR_ALIGNED(x) __attribute__((aligned(x)))
#define TLSF_ATTR_UNUSED __attribute__((unused))
#define TLSF_INLINE static inline __attribute__((always_inline))
#elif defined(_MSC_VER)
#define TLSF_ATTR_PACKED
#define TLSF_ATTR_ALIGNED(x) __declspec(align(x))
#define TLSF_ATTR_UNUSED
#define TLSF_INLINE static __inline
#else
#define TLSF_ATTR_PACKED
#define TLSF_ATTR_ALIGNED(x)
#define TLSF_ATTR_UNUSED
#define TLSF_INLINE static inline
#endif

/* TLSF handle - opaque pointer */
typedef struct tlsf_t* tlsf_t;

/* Block size type - must be unsigned */
typedef uint32_t bsize_t;

/* Block status flags */
typedef enum {
    BLOCK_FREE = 0,
    BLOCK_USED = 1
} block_status_t;

/* Block header structure */
typedef struct block_header_t {
    /* Size of previous block (if free) */
    bsize_t prev_phys_size;
    
    /* Size of this block (excluding header) */
    bsize_t size;
    
    /* Next free block (if free) */
    struct block_header_t* next_free;
    
    /* Previous free block (if free) */
    struct block_header_t* prev_free;
} TLSF_ATTR_PACKED block_header_t;

/* Area header structure */
typedef struct area_header_t {
    /* Magic number for validation */
    uint32_t magic;
    
    /* Pointer to the TLSF control structure */
    tlsf_t tlsf;
    
    /* Size of the entire memory area */
    size_t size;
} area_header_t;

/* Forward declaration of control structure */
struct tlsf_control_t;

/* Statistics structure (if enabled) */
#if TLSF_STATISTICS
typedef struct {
    size_t allocated_bytes;
    size_t free_bytes;
    size_t allocated_blocks;
    size_t free_blocks;
    size_t search_count;
    size_t merge_count;
    size_t split_count;
} tlsf_statistics_t;
#endif

/* Assertion macro */
#if TLSF_ASSERTIONS
#include <assert.h>
#define TLSF_ASSERT(cond, msg) assert((cond) && (msg))
#else
#define TLSF_ASSERT(cond, msg) ((void)0)
#endif

#endif /* TLSF_TYPES_H */
