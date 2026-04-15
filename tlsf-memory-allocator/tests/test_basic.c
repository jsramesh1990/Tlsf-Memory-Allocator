#include "tlsf.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define POOL_SIZE (1024 * 1024)  /* 1MB pool */
#define TEST_ITERATIONS 1000

static void test_basic_allocation(void) {
    printf("Test 1: Basic allocation\n");
    
    void* memory = malloc(POOL_SIZE);
    tlsf_t pool = tlsf_create(memory, POOL_SIZE);
    assert(pool != NULL);
    
    /* Allocate and free a simple block */
    void* ptr = tlsf_malloc(pool, 256);
    assert(ptr != NULL);
    assert(tlsf_block_size(pool, ptr) >= 256);
    
    memset(ptr, 0x42, 256);
    tlsf_free(pool, ptr);
    
    tlsf_destroy(pool);
    free(memory);
    
    printf("  Passed\n");
}

static void test_multiple_allocations(void) {
    printf("Test 2: Multiple allocations\n");
    
    void* memory = malloc(POOL_SIZE);
    tlsf_t pool = tlsf_create(memory, POOL_SIZE);
    assert(pool != NULL);
    
    void* pointers[100];
    
    /* Allocate many blocks */
    for (int i = 0; i < 100; i++) {
        pointers[i] = tlsf_malloc(pool, (i + 1) * 10);
        assert(pointers[i] != NULL);
        memset(pointers[i], i, (i + 1) * 10);
    }
    
    /* Verify and free */
    for (int i = 0; i < 100; i++) {
        assert(tlsf_block_size(pool, pointers[i]) >= (i + 1) * 10);
        tlsf_free(pool, pointers[i]);
    }
    
    tlsf_destroy(pool);
    free(memory);
    
    printf("  Passed\n");
}

static void test_reallocation(void) {
    printf("Test 3: Reallocation\n");
    
    void* memory = malloc(POOL_SIZE);
    tlsf_t pool = tlsf_create(memory, POOL_SIZE);
    assert(pool != NULL);
    
    /* Allocate initial block */
    void* ptr = tlsf_malloc(pool, 100);
    assert(ptr != NULL);
    
    memset(ptr, 0xAA, 100);
    
    /* Reallocate to larger size */
    void* new_ptr = tlsf_realloc(pool, ptr, 200);
    assert(new_ptr != NULL);
    
    /* Verify data was copied */
    for (int i = 0; i < 100; i++) {
        assert(((unsigned char*)new_ptr)[i] == 0xAA);
    }
    
    /* Reallocate to smaller size */
    void* small_ptr = tlsf_realloc(pool, new_ptr, 50);
    assert(small_ptr != NULL);
    
    tlsf_free(pool, small_ptr);
    tlsf_destroy(pool);
    free(memory);
    
    printf("  Passed\n");
}

static void test_aligned_allocation(void) {
    printf("Test 4: Aligned allocation\n");
    
    void* memory = malloc(POOL_SIZE);
    tlsf_t pool = tlsf_create(memory, POOL_SIZE);
    assert(pool != NULL);
    
    /* Test various alignments */
    size_t alignments[] = {8, 16, 32, 64, 128, 256, 512, 1024};
    
    for (size_t i = 0; i < sizeof(alignments)/sizeof(alignments[0]); i++) {
        void* ptr = tlsf_memalign(pool, alignments[i], 100);
        assert(ptr != NULL);
        
        /* Verify alignment */
        assert(((uintptr_t)ptr & (alignments[i] - 1)) == 0);
        
        memset(ptr, 0x55, 100);
        tlsf_free(pool, ptr);
    }
    
    tlsf_destroy(pool);
    free(memory);
    
    printf("  Passed\n");
}

static void test_fragmentation(void) {
    printf("Test 5: Fragmentation test\n");
    
    void* memory = malloc(POOL_SIZE);
    tlsf_t pool = tlsf_create(memory, POOL_SIZE);
    assert(pool != NULL);
    
    void* small_blocks[100];
    void* large_block;
    
    /* Allocate many small blocks */
    for (int i = 0; i < 100; i++) {
        small_blocks[i] = tlsf_malloc(pool, 16);
        assert(small_blocks[i] != NULL);
    }
    
    /* Allocate a large block (should fail) */
    large_block = tlsf_malloc(pool, POOL_SIZE / 2);
    assert(large_block == NULL);
    
    /* Free every other small block */
    for (int i = 0; i < 100; i += 2) {
        tlsf_free(pool, small_blocks[i]);
        small_blocks[i] = NULL;
    }
    
    /* Now should be able to allocate medium block */
    large_block = tlsf_malloc(pool, POOL_SIZE / 4);
    assert(large_block != NULL);
    
    /* Free remaining blocks */
    for (int i = 0; i < 100; i++) {
        if (small_blocks[i]) {
            tlsf_free(pool, small_blocks[i]);
        }
    }
    
    tlsf_free(pool, large_block);
    tlsf_destroy(pool);
    free(memory);
    
    printf("  Passed\n");
}

static void test_pool_integrity(void) {
    printf("Test 6: Pool integrity\n");
    
    void* memory = malloc(POOL_SIZE);
    tlsf_t pool = tlsf_create(memory, POOL_SIZE);
    assert(pool != NULL);
    
    /* Check pool validity */
    assert(tlsf_check(pool) == 1);
    
    /* Perform allocations */
    for (int i = 0; i < TEST_ITERATIONS; i++) {
        size_t size = (rand() % 4096) + 1;
        void* ptr = tlsf_malloc(pool, size);
        assert(ptr != NULL);
        
        if (rand() % 2 == 0) {
            tlsf_free(pool, ptr);
        }
    }
    
    /* Check pool validity again */
    assert(tlsf_check(pool) == 1);
    
    tlsf_destroy(pool);
    free(memory);
    
    printf("  Passed\n");
}

static void print_block_info(void* ptr, size_t size, int used, void* data) {
    int* count = (int*)data;
    (*count)++;
    
    if (used) {
        printf("    Block %p: %zu bytes (used)\n", ptr, size);
    } else {
        printf("    Block %p: %zu bytes (free)\n", ptr, size);
    }
}

static void test_walk_function(void) {
    printf("Test 7: Walk function\n");
    
    void* memory = malloc(POOL_SIZE);
    tlsf_t pool = tlsf_create(memory, POOL_SIZE);
    assert(pool != NULL);
    
    /* Allocate some blocks */
    void* ptr1 = tlsf_malloc(pool, 100);
    void* ptr2 = tlsf_malloc(pool, 200);
    void* ptr3 = tlsf_malloc(pool, 300);
    
    /* Free middle block */
    tlsf_free(pool, ptr2);
    
    /* Walk through blocks */
    int block_count = 0;
    printf("  Walking pool...\n");
    tlsf_walk_pool(pool, print_block_info, &block_count);
    
    printf("  Total blocks: %d\n", block_count);
    
    tlsf_free(pool, ptr1);
    tlsf_free(pool, ptr3);
    tlsf_destroy(pool);
    free(memory);
    
    printf("  Passed\n");
}

int main(void) {
    printf("=== TLSF Basic Tests   ===\n\n");
    
    test_basic_allocation();
    test_multiple_allocations();
    test_reallocation();
    test_aligned_allocation();
    test_fragmentation();
    test_pool_integrity();
    test_walk_function();
    
    printf("\n=== All tests passed! ===\n");
    return 0;
}
