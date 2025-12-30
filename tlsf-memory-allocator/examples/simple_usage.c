#include "tlsf.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    printf("=== TLSF Simple Usage Example ===\n\n");
    
    /* Create a memory pool of 1MB */
    size_t pool_size = 1024 * 1024;
    void* memory = malloc(pool_size);
    
    if (!memory) {
        printf("Failed to allocate memory for pool\n");
        return 1;
    }
    
    printf("1. Creating TLSF pool of %zu bytes...\n", pool_size);
    tlsf_t pool = tlsf_create(memory, pool_size);
    
    if (!pool) {
        printf("Failed to create TLSF pool\n");
        free(memory);
        return 1;
    }
    
    printf("   Pool created successfully at %p\n", (void*)pool);
    
    /* Basic allocation */
    printf("\n2. Basic allocation...\n");
    void* ptr1 = tlsf_malloc(pool, 256);
    if (ptr1) {
        printf("   Allocated 256 bytes at %p\n", ptr1);
        memset(ptr1, 'A', 256);
    }
    
    /* Multiple allocations */
    printf("\n3. Multiple allocations...\n");
    void* pointers[10];
    for (int i = 0; i < 10; i++) {
        pointers[i] = tlsf_malloc(pool, (i + 1) * 64);
        if (pointers[i]) {
            printf("   Allocated %d bytes at %p\n", (i + 1) * 64, pointers[i]);
        }
    }
    
    /* Aligned allocation */
    printf("\n4. Aligned allocation...\n");
    void* aligned_ptr = tlsf_memalign(pool, 64, 128);
    if (aligned_ptr) {
        printf("   Allocated 128 bytes aligned to 64 bytes at %p\n", aligned_ptr);
        printf("   Address modulo 64 = %lu\n", (uintptr_t)aligned_ptr % 64);
    }
    
    /* Reallocation */
    printf("\n5. Reallocation...\n");
    void* small_ptr = tlsf_malloc(pool, 100);
    if (small_ptr) {
        printf("   Allocated 100 bytes at %p\n", small_ptr);
        memset(small_ptr, 'B', 100);
        
        void* larger_ptr = tlsf_realloc(pool, small_ptr, 200);
        if (larger_ptr) {
            printf("   Reallocated to 200 bytes at %p\n", larger_ptr);
            /* Verify old data was preserved */
            printf("   First byte after realloc: '%c'\n", ((char*)larger_ptr)[0]);
            tlsf_free(pool, larger_ptr);
        }
    }
    
    /* Check fragmentation */
    printf("\n6. Checking fragmentation...\n");
    float frag = tlsf_fragmentation(pool);
    printf("   Current fragmentation: %.2f%%\n", frag);
    
    /* Free some memory */
    printf("\n7. Freeing memory...\n");
    for (int i = 0; i < 10; i += 2) {
        if (pointers[i]) {
            tlsf_free(pool, pointers[i]);
            printf("   Freed pointer %p\n", pointers[i]);
        }
    }
    
    if (ptr1) {
        tlsf_free(pool, ptr1);
        printf("   Freed pointer %p\n", ptr1);
    }
    
    if (aligned_ptr) {
        tlsf_free(pool, aligned_ptr);
        printf("   Freed aligned pointer %p\n", aligned_ptr);
    }
    
    /* Check fragmentation again */
    frag = tlsf_fragmentation(pool);
    printf("   Fragmentation after freeing: %.2f%%\n", frag);
    
    /* Check pool integrity */
    printf("\n8. Checking pool integrity...\n");
    if (tlsf_check(pool)) {
        printf("   Pool integrity check passed\n");
    } else {
        printf("   Pool integrity check failed\n");
    }
    
    /* Clean up remaining allocations */
    printf("\n9. Cleaning up...\n");
    for (int i = 1; i < 10; i += 2) {
        if (pointers[i]) {
            tlsf_free(pool, pointers[i]);
        }
    }
    
    printf("\n10. Destroying pool...\n");
    tlsf_destroy(pool);
    free(memory);
    
    printf("\n=== Example completed successfully ===\n");
    return 0;
}
