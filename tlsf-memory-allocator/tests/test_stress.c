#include "tlsf.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#define POOL_SIZE (10 * 1024 * 1024)  /* 10MB pool */
#define MAX_ALLOCATIONS 10000
#define STRESS_ITERATIONS 100000

typedef struct {
    void* ptr;
    size_t size;
} allocation_t;

static void stress_test_random(void) {
    printf("Stress Test 1: Random allocations\n");
    
    void* memory = malloc(POOL_SIZE);
    tlsf_t pool = tlsf_create(memory, POOL_SIZE);
    assert(pool != NULL);
    
    allocation_t allocations[MAX_ALLOCATIONS] = {0};
    int alloc_count = 0;
    size_t total_allocated = 0;
    
    srand(time(NULL));
    
    for (int i = 0; i < STRESS_ITERATIONS; i++) {
        if (alloc_count < MAX_ALLOCATIONS && (rand() % 3 != 0)) {
            /* Allocate */
            size_t size = (rand() % 4096) + 1;
            void* ptr = tlsf_malloc(pool, size);
            
            if (ptr) {
                allocations[alloc_count].ptr = ptr;
                allocations[alloc_count].size = size;
                alloc_count++;
                total_allocated += size;
                
                /* Fill with pattern */
                memset(ptr, (i & 0xFF), size);
            }
        } else if (alloc_count > 0) {
            /* Free random allocation */
            int idx = rand() % alloc_count;
            tlsf_free(pool, allocations[idx].ptr);
            
            /* Shift array */
            total_allocated -= allocations[idx].size;
            for (int j = idx; j < alloc_count - 1; j++) {
                allocations[j] = allocations[j + 1];
            }
            alloc_count--;
        }
        
        /* Periodically check pool integrity */
        if (i % 10000 == 0) {
            assert(tlsf_check(pool) == 1);
        }
    }
    
    /* Free all remaining allocations */
    for (int i = 0; i < alloc_count; i++) {
        tlsf_free(pool, allocations[i].ptr);
    }
    
    assert(tlsf_check(pool) == 1);
    
    tlsf_destroy(pool);
    free(memory);
    
    printf("  Passed (%d iterations)\n", STRESS_ITERATIONS);
}

static void stress_test_pattern(void) {
    printf("Stress Test 2: Pattern allocations\n");
    
    void* memory = malloc(POOL_SIZE);
    tlsf_t pool = tlsf_create(memory, POOL_SIZE);
    assert(pool != NULL);
    
    /* Allocate in increasing size pattern */
    for (int pattern = 0; pattern < 10; pattern++) {
        void* blocks[100];
        
        /* Allocate increasing sizes */
        for (int i = 0; i < 100; i++) {
            size_t size = 16 * (i + 1);
            blocks[i] = tlsf_malloc(pool, size);
            assert(blocks[i] != NULL);
            memset(blocks[i], pattern, size);
        }
        
        /* Free in random order */
        int freed[100] = {0};
        int freed_count = 0;
        
        while (freed_count < 100) {
            int idx;
            do {
                idx = rand() % 100;
            } while (freed[idx]);
            
            tlsf_free(pool, blocks[idx]);
            freed[idx] = 1;
            freed_count++;
        }
        
        assert(tlsf_check(pool) == 1);
    }
    
    tlsf_destroy(pool);
    free(memory);
    
    printf("  Passed\n");
}

static void stress_test_realloc(void) {
    printf("Stress Test 3: Reallocation stress\n");
    
    void* memory = malloc(POOL_SIZE);
    tlsf_t pool = tlsf_create(memory, POOL_SIZE);
    assert(pool != NULL);
    
    void* ptr = NULL;
    
    for (int i = 0; i < 10000; i++) {
        size_t old_size = ptr ? tlsf_block_size(pool, ptr) : 0;
        size_t new_size = (rand() % 8192) + 1;
        
        ptr = tlsf_realloc(pool, ptr, new_size);
        assert(ptr != NULL);
        
        /* Verify old data if size increased */
        if (new_size > old_size && old_size > 0) {
            for (size_t j = 0; j < old_size; j++) {
                assert(((unsigned char*)ptr)[j] == (i - 1) & 0xFF);
            }
        }
        
        /* Fill with pattern */
        memset(ptr, i & 0xFF, new_size);
        
        if (i % 1000 == 0) {
            assert(tlsf_check(pool) == 1);
        }
    }
    
    tlsf_free(pool, ptr);
    tlsf_destroy(pool);
    free(memory);
    
    printf("  Passed\n");
}

static void benchmark_test(void) {
    printf("Benchmark Test: Performance measurement\n");
    
    void* memory = malloc(POOL_SIZE);
    tlsf_t pool = tlsf_create(memory, POOL_SIZE);
    assert(pool != NULL);
    
    clock_t start, end;
    double alloc_time = 0, free_time = 0;
    int iterations = 100000;
    
    void* allocations[iterations];
    size_t sizes[iterations];
    
    /* Allocation benchmark */
    start = clock();
    for (int i = 0; i < iterations; i++) {
        sizes[i] = (rand() % 1024) + 1;
        allocations[i] = tlsf_malloc(pool, sizes[i]);
        assert(allocations[i] != NULL);
    }
    end = clock();
    alloc_time = (double)(end - start) / CLOCKS_PER_SEC;
    
    /* Free benchmark */
    start = clock();
    for (int i = 0; i < iterations; i++) {
        tlsf_free(pool, allocations[i]);
    }
    end = clock();
    free_time = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("  Allocations: %d ops in %.3f seconds (%.0f ops/sec)\n",
           iterations, alloc_time, iterations / alloc_time);
    printf("  Frees: %d ops in %.3f seconds (%.0f ops/sec)\n",
           iterations, free_time, iterations / free_time);
    
    tlsf_destroy(pool);
    free(memory);
}

int main(void) {
    printf("=== TLSF Stress Tests ===\n\n");
    
    stress_test_random();
    stress_test_pattern();
    stress_test_realloc();
    benchmark_test();
    
    printf("\n=== All stress tests passed! ===\n");
    return 0;
}
