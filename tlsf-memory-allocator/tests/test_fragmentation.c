#include "tlsf.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#define POOL_SIZE (4 * 1024 * 1024)  /* 4MB pool */

static void fragmentation_measurement(void) {
    printf("Fragmentation Test 1: Measurement\n");
    
    void* memory = malloc(POOL_SIZE);
    tlsf_t pool = tlsf_create(memory, POOL_SIZE);
    assert(pool != NULL);
    
    void* blocks[1000];
    int block_count = 0;
    
    /* Initial fragmentation */
    float frag_start = tlsf_fragmentation(pool);
    printf("  Initial fragmentation: %.2f%%\n", frag_start);
    
    /* Allocate many small blocks */
    for (int i = 0; i < 500; i++) {
        blocks[block_count] = tlsf_malloc(pool, 64);
        assert(blocks[block_count] != NULL);
        block_count++;
    }
    
    float frag_after_small = tlsf_fragmentation(pool);
    printf("  After small allocations: %.2f%%\n", frag_after_small);
    
    /* Free every other block */
    for (int i = 0; i < block_count; i += 2) {
        tlsf_free(pool, blocks[i]);
        blocks[i] = NULL;
    }
    
    float frag_after_free = tlsf_fragmentation(pool);
    printf("  After freeing every other: %.2f%%\n", frag_after_free);
    
    /* Allocate medium blocks in the gaps */
    for (int i = 0; i < 100; i++) {
        void* medium = tlsf_malloc(pool, 256);
        assert(medium != NULL);
        
        /* Add to list for cleanup */
        for (int j = 0; j < block_count; j++) {
            if (blocks[j] == NULL) {
                blocks[j] = medium;
                break;
            }
        }
    }
    
    float frag_after_medium = tlsf_fragmentation(pool);
    printf("  After medium allocations: %.2f%%\n", frag_after_medium);
    
    /* Try to allocate a large block */
    void* large = tlsf_malloc(pool, POOL_SIZE / 2);
    if (large) {
        printf("  Large allocation succeeded (fragmentation low)\n");
        tlsf_free(pool, large);
    } else {
        printf("  Large allocation failed (high fragmentation)\n");
    }
    
    /* Cleanup */
    for (int i = 0; i < block_count; i++) {
        if (blocks[i]) {
            tlsf_free(pool, blocks[i]);
        }
    }
    
    float frag_end = tlsf_fragmentation(pool);
    printf("  Final fragmentation: %.2f%%\n", frag_end);
    
    tlsf_destroy(pool);
    free(memory);
    
    printf("  Passed\n");
}

static void worst_case_fragmentation(void) {
    printf("Fragmentation Test 2: Worst-case scenario\n");
    
    void* memory = malloc(POOL_SIZE);
    tlsf_t pool = tlsf_create(memory, POOL_SIZE);
    assert(pool != NULL);
    
    /* Create worst-case fragmentation: allocate many blocks, 
       then free all but first and last */
    void* first_block = tlsf_malloc(pool, 64);
    void* last_block = NULL;
    
    void* blocks[100];
    for (int i = 0; i < 100; i++) {
        blocks[i] = tlsf_malloc(pool, 128);
        assert(blocks[i] != NULL);
        last_block = blocks[i];
    }
    
    /* Free middle blocks */
    for (int i = 1; i < 99; i++) {
        tlsf_free(pool, blocks[i]);
    }
    
    /* Now we have free memory fragmented in the middle */
    float frag = tlsf_fragmentation(pool);
    printf("  Fragmentation after creating holes: %.2f%%\n", frag);
    
    /* Try to allocate a block that should fit in total free space
       but not in any single hole */
    void* large = tlsf_malloc(pool, 128 * 98);  /* Total free space */
    if (large) {
        printf("  TLSF successfully coalesced blocks!\n");
        tlsf_free(pool, large);
    } else {
        printf("  Fragmentation prevented allocation\n");
    }
    
    tlsf_free(pool, first_block);
    tlsf_free(pool, last_block);
    tlsf_destroy(pool);
    free(memory);
    
    printf("  Passed\n");
}

static void print_fragmentation_history(void* ptr, size_t size, int used, void* data) {
    (void)ptr;
    (void)data;
    
    static int block_num = 0;
    if (used) {
        printf("[%3d: %6zu] ", block_num, size);
    } else {
        printf("(%3d: %6zu) ", block_num, size);
    }
    
    block_num++;
    if (block_num % 8 == 0) {
        printf("\n");
    }
}

static void visualization_test(void) {
    printf("Fragmentation Test 3: Memory layout visualization\n");
    
    void* memory = malloc(POOL_SIZE);
    tlsf_t pool = tlsf_create(memory, POOL_SIZE);
    assert(pool != NULL);
    
    printf("\n  Initial empty pool:\n");
    tlsf_walk_pool(pool, print_fragmentation_history, NULL);
    printf("\n");
    
    /* Allocate pattern */
    void* a = tlsf_malloc(pool, 100);
    void* b = tlsf_malloc(pool, 200);
    void* c = tlsf_malloc(pool, 150);
    void* d = tlsf_malloc(pool, 300);
    
    printf("\n  After 4 allocations:\n");
    tlsf_walk_pool(pool, print_fragmentation_history, NULL);
    printf("\n");
    
    /* Free B and D */
    tlsf_free(pool, b);
    tlsf_free(pool, d);
    
    printf("\n  After freeing blocks 2 and 4:\n");
    tlsf_walk_pool(pool, print_fragmentation_history, NULL);
    printf("\n");
    
    /* Allocate E that spans B and D's space */
    void* e = tlsf_malloc(pool, 450);
    
    printf("\n  After allocating 450 bytes (should merge free spaces):\n");
    tlsf_walk_pool(pool, print_fragmentation_history, NULL);
    printf("\n");
    
    /* Cleanup */
    tlsf_free(pool, a);
    tlsf_free(pool, c);
    if (e) tlsf_free(pool, e);
    
    tlsf_destroy(pool);
    free(memory);
    
    printf("  Passed\n");
}

static void efficiency_test(void) {
    printf("Fragmentation Test 4: Memory efficiency\n");
    
    void* memory = malloc(POOL_SIZE);
    tlsf_t pool = tlsf_create(memory, POOL_SIZE);
    assert(pool != NULL);
    
    size_t total_allocated = 0;
    size_t overhead = 0;
    int allocation_count = 1000;
    void* allocations[allocation_count];
    
    /* Allocate various sizes */
    for (int i = 0; i < allocation_count; i++) {
        size_t size = (i % 50 + 1) * 16;  /* Sizes from 16 to 800 bytes */
        allocations[i] = tlsf_malloc(pool, size);
        assert(allocations[i] != NULL);
        total_allocated += size;
    }
    
    /* Calculate overhead */
    size_t total_used = total_allocated + allocation_count * sizeof(block_header_t);
    float efficiency = (float)total_allocated / total_used * 100.0f;
    
    printf("  Total allocated: %zu bytes\n", total_allocated);
    printf("  Block headers: %zu bytes\n", allocation_count * sizeof(block_header_t));
    printf("  Memory efficiency: %.2f%%\n", efficiency);
    
    /* Free half of allocations */
    for (int i = 0; i < allocation_count; i += 2) {
        tlsf_free(pool, allocations[i]);
        allocations[i] = NULL;
    }
    
    /* Calculate fragmentation */
    float frag = tlsf_fragmentation(pool);
    printf("  Fragmentation after freeing half: %.2f%%\n", frag);
    
    /* Try to allocate large block */
    void* large = tlsf_malloc(pool, total_allocated / 2);
    if (large) {
        printf("  Can allocate large block (good coalescing)\n");
        tlsf_free(pool, large);
    }
    
    /* Cleanup */
    for (int i = 0; i < allocation_count; i++) {
        if (allocations[i]) {
            tlsf_free(pool, allocations[i]);
        }
    }
    
    tlsf_destroy(pool);
    free(memory);
    
    printf("  Passed\n");
}

int main(void) {
    printf("=== TLSF Fragmentation Tests ===\n\n");
    
    fragmentation_measurement();
    worst_case_fragmentation();
    visualization_test();
    efficiency_test();
    
    printf("\n=== All fragmentation tests passed! ===\n");
    return 0;
}
