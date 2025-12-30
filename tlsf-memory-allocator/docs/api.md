markdown
# TLSF Memory Allocator API Reference

## Overview

This document describes the complete API for the TLSF memory allocator. All functions are thread-safe when `TLSF_USE_LOCKS` is enabled.

## Data Types

### `tlsf_t`
```c
typedef struct tlsf_t* tlsf_t;
Opaque handle to a TLSF memory pool.

block_header_t
c
typedef struct block_header_t {
    bsize_t prev_phys_size;
    bsize_t size;
    struct block_header_t* next_free;
    struct block_header_t* prev_free;
} block_header_t;
Internal block header structure (for debugging).

Core Functions
tlsf_create
c
tlsf_t tlsf_create(void* mem, size_t bytes);
Creates a new TLSF memory pool.

Parameters:

mem: Pointer to pre-allocated memory

bytes: Size of memory area in bytes

Returns:

Handle to the memory pool

NULL on failure

Notes:

mem must be aligned to TLSF_ALIGN_SIZE

Minimum pool size is TLSF_MIN_BLOCK_SIZE * 2

The pool cannot be resized after creation

Example:

c
void* memory = malloc(1024 * 1024);
tlsf_t pool = tlsf_create(memory, 1024 * 1024);
tlsf_destroy
c
void tlsf_destroy(tlsf_t tlsf);
Destroys a TLSF memory pool.

Parameters:

tlsf: Handle to the memory pool

Notes:

Does not free the underlying memory

Pool handle becomes invalid after call

Thread-safe: must not be called concurrently

Example:

c
tlsf_destroy(pool);
free(memory);
tlsf_malloc
c
void* tlsf_malloc(tlsf_t tlsf, size_t size);
Allocates memory from the pool.

Parameters:

tlsf: Handle to the memory pool

size: Number of bytes to allocate

Returns:

Pointer to allocated memory

NULL if allocation fails

Notes:

Returns aligned memory (aligned to TLSF_ALIGN_SIZE)

Actual allocated size may be larger due to alignment

Thread-safe

Example:

c
void* ptr = tlsf_malloc(pool, 256);
if (ptr) {
    // Use memory
}
tlsf_memalign
c
void* tlsf_memalign(tlsf_t tlsf, size_t align, size_t size);
Allocates aligned memory from the pool.

Parameters:

tlsf: Handle to the memory pool

align: Alignment requirement (must be power of 2)

size: Number of bytes to allocate

Returns:

Pointer to aligned memory

NULL if allocation fails

Notes:

align must be ≥ TLSF_ALIGN_SIZE

May waste more memory than tlsf_malloc

Thread-safe

Example:

c
void* ptr = tlsf_memalign(pool, 64, 128);  // 64-byte aligned
tlsf_realloc
c
void* tlsf_realloc(tlsf_t tlsf, void* ptr, size_t size);
Reallocates memory to a new size.

Parameters:

tlsf: Handle to the memory pool

ptr: Pointer to previously allocated memory

size: New size in bytes

Returns:

Pointer to reallocated memory

NULL if reallocation fails

Notes:

If ptr is NULL, behaves like tlsf_malloc

If size is 0, behaves like tlsf_free

May move memory to new location

Preserves existing data up to minimum of old and new sizes

Thread-safe

Example:

c
void* new_ptr = tlsf_realloc(pool, ptr, 512);
tlsf_free
c
void tlsf_free(tlsf_t tlsf, void* ptr);
Frees previously allocated memory.

Parameters:

tlsf: Handle to the memory pool

ptr: Pointer to memory to free

Notes:

NULL pointers are safely ignored

Double-free causes undefined behavior

Thread-safe

Example:

c
tlsf_free(pool, ptr);
Inspection Functions
tlsf_block_size
c
size_t tlsf_block_size(tlsf_t tlsf, void* ptr);
Returns the size of an allocated block.

Parameters:

tlsf: Handle to the memory pool

ptr: Pointer to allocated memory

Returns:

Size of the allocated block in bytes

0 if pointer is invalid or block is free

Notes:

Returns actual allocated size, not requested size

Thread-safe

Example:

c
size_t actual_size = tlsf_block_size(pool, ptr);
tlsf_check
c
int tlsf_check(tlsf_t tlsf);
Validates the integrity of the memory pool.

Parameters:

tlsf: Handle to the memory pool

Returns:

1 if pool is valid

0 if corruption detected

Notes:

Debugging function

May impact performance

Not thread-safe

Example:

c
if (!tlsf_check(pool)) {
    printf("Pool corruption detected!\n");
}
tlsf_fragmentation
c
float tlsf_fragmentation(tlsf_t tlsf);
Calculates memory fragmentation percentage.

Parameters:

tlsf: Handle to the memory pool

Returns:

Fragmentation percentage (0-100)

Notes:

0% = no fragmentation

100% = maximum fragmentation

Approximate measurement

Thread-safe

Example:

c
float frag = tlsf_fragmentation(pool);
printf("Fragmentation: %.1f%%\n", frag);
tlsf_walk_pool
c
void tlsf_walk_pool(tlsf_t tlsf, 
    void (*callback)(void* ptr, size_t size, int used, void* data), 
    void* data);
Iterates through all blocks in the pool.

Parameters:

tlsf: Handle to the memory pool

callback: Function called for each block

data: User data passed to callback

Callback Parameters:

ptr: Pointer to block data

size: Size of block

used: 1 if block is allocated, 0 if free

data: User data

Notes:

Debugging function

Callback called for every block in pool

Not thread-safe

Example:

c
void print_block(void* ptr, size_t size, int used, void* data) {
    printf("Block %p: %zu bytes (%s)\n", 
        ptr, size, used ? "used" : "free");
}

tlsf_walk_pool(pool, print_block, NULL);
Configuration Macros
Compile-Time Configuration
c
/* include/tlsf_config.h */

/* Alignment requirement */
#define TLSF_ALIGN_SIZE 8

/* Minimum block size */
#define TLSF_MIN_BLOCK_SIZE 32

/* Maximum allocation size */
#define TLSF_MAX_SIZE (1UL << 31)

/* First level index count */
#define TLSF_FL_INDEX_COUNT 14

/* Second level index count */
#define TLSF_SL_INDEX_COUNT 8

/* Thread safety */
#define TLSF_USE_LOCKS 1

/* Debug features */
#define TLSF_DEBUG 0

/* Statistics collection */
#define TLSF_STATISTICS 0
Error Handling
Common Error Conditions
Out of Memory

tlsf_malloc, tlsf_memalign, tlsf_realloc return NULL

Check pool size and fragmentation

Invalid Arguments

size = 0: returns NULL (malloc) or frees (realloc)

align not power of 2: returns NULL

Invalid pointer to free: ignored or asserts

Pool Corruption

tlsf_check returns 0

May indicate buffer overflows or double frees

Debugging Tips
Enable Debug Mode

c
#define TLSF_DEBUG 1
Fills memory with patterns

Enables additional checks

Use Walk Function

c
tlsf_walk_pool(pool, debug_callback, NULL);
Inspect memory layout

Find memory leaks

Monitor Fragmentation

c
float frag = tlsf_fragmentation(pool);
High fragmentation indicates poor allocation patterns

Thread Safety
Locking Behavior
When TLSF_USE_LOCKS is enabled:

Single mutex protects entire pool

All API functions are thread-safe

No concurrent pool creation/destruction

When TLSF_USE_LOCKS is disabled:

No locking overhead

User must ensure thread safety

Suitable for single-threaded applications

Performance Considerations
Lock contention may impact performance

Consider multiple pools for different threads

Disable locks for single-threaded use

Platform Portability
Required Functions
The platform layer must provide:

Memory Operations (optional override)

tlsf_plat_malloc

tlsf_plat_free

tlsf_plat_memalign

Threading (if TLSF_USE_LOCKS)

Mutex operations

Platform-specific implementations

Default Implementation
Default platform implementations use:

Standard C library for memory

pthreads or Win32 API for threading

Fallbacks for bare-metal systems

Examples
Basic Usage
c
#include "tlsf.h"

int main() {
    // Create pool
    void* memory = malloc(1024 * 1024);
    tlsf_t pool = tlsf_create(memory, 1024 * 1024);
    
    // Allocate
    void* ptr = tlsf_malloc(pool, 256);
    
    // Use memory...
    
    // Free
    tlsf_free(pool, ptr);
    
    // Destroy
    tlsf_destroy(pool);
    free(memory);
    
    return 0;
}
Advanced Usage
c
// Aligned allocation for SIMD
void* simd_data = tlsf_memalign(pool, 32, 1024);

// Reallocation with data preservation
void* new_data = tlsf_realloc(pool, old_data, 2048);

// Memory inspection
size_t actual = tlsf_block_size(pool, ptr);
float frag = tlsf_fragmentation(pool);
Best Practices
Pool Sizing

Allocate slightly more than needed

Consider peak usage, not average

Allocation Patterns

Allocate similar sizes together

Free in reverse allocation order when possible

Avoid alternating large and small allocations

Debugging

Enable TLSF_DEBUG during development

Use tlsf_check periodically

Monitor fragmentation

Performance

Disable locks if single-threaded

Consider cache alignment for performance-critical data

Use multiple pools for different allocation patterns

text

## **Complete Project Structure**

Now you have a complete TLSF memory allocator project with:

1. **Core implementation** - Full TLSF algorithm
2. **Platform abstraction** - Portable across systems
3. **Comprehensive tests** - Basic, stress, and fragmentation tests
4. **Examples** - Simple usage demonstration
5. **Documentation** - Design, API, and flow documentation
6. **Build system** - CMake for easy compilation

**To build and test:**
```bash
mkdir build
cd build
cmake ..
make
./test_basic
./test_stress
./test_fragmentation
./example_simple
This implementation provides:

O(1) allocation and deallocation

Low fragmentation

Thread safety (optional)

Platform independence

Comprehensive testing

Detailed documentation
