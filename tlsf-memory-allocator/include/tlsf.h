#ifndef TLSF_H
#define TLSF_H

#include "tlsf_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Create a TLSF memory pool
 * 
 * @param mem Pointer to the memory block
 * @param bytes Size of the memory block in bytes
 * @return tlsf_t Handle to the memory pool
 */
tlsf_t tlsf_create(void* mem, size_t bytes);

/**
 * @brief Destroy a TLSF memory pool
 * 
 * @param tlsf Handle to the memory pool
 */
void tlsf_destroy(tlsf_t tlsf);

/**
 * @brief Allocate memory from the pool
 * 
 * @param tlsf Handle to the memory pool
 * @param size Number of bytes to allocate
 * @return void* Pointer to allocated memory, or NULL if failed
 */
void* tlsf_malloc(tlsf_t tlsf, size_t size);

/**
 * @brief Allocate aligned memory from the pool
 * 
 * @param tlsf Handle to the memory pool
 * @param align Alignment requirement (must be power of 2)
 * @param size Number of bytes to allocate
 * @return void* Pointer to allocated memory, or NULL if failed
 */
void* tlsf_memalign(tlsf_t tlsf, size_t align, size_t size);

/**
 * @brief Reallocate memory to a new size
 * 
 * @param tlsf Handle to the memory pool
 * @param ptr Pointer to previously allocated memory
 * @param size New size in bytes
 * @return void* Pointer to reallocated memory, or NULL if failed
 */
void* tlsf_realloc(tlsf_t tlsf, void* ptr, size_t size);

/**
 * @brief Free allocated memory
 * 
 * @param tlsf Handle to the memory pool
 * @param ptr Pointer to memory to free
 */
void tlsf_free(tlsf_t tlsf, void* ptr);

/**
 * @brief Get the size of an allocated block
 * 
 * @param tlsf Handle to the memory pool
 * @param ptr Pointer to allocated memory
 * @return size_t Size of the allocated block
 */
size_t tlsf_block_size(tlsf_t tlsf, void* ptr);

/**
 * @brief Check if the pool is valid (debug only)
 * 
 * @param tlsf Handle to the memory pool
 * @return int 1 if valid, 0 otherwise
 */
int tlsf_check(tlsf_t tlsf);

/**
 * @brief Get pool fragmentation (debug only)
 * 
 * @param tlsf Handle to the memory pool
 * @return float Fragmentation percentage (0-100)
 */
float tlsf_fragmentation(tlsf_t tlsf);

/**
 * @brief Walk through all blocks in the pool (debug only)
 * 
 * @param tlsf Handle to the memory pool
 * @param callback Function called for each block
 * @param data User data passed to callback
 */
void tlsf_walk_pool(tlsf_t tlsf, 
    void (*callback)(void* ptr, size_t size, int used, void* data), 
    void* data);

#ifdef __cplusplus
}
#endif

#endif /* TLSF_H */
