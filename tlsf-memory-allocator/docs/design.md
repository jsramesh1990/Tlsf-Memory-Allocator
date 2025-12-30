markdown
# TLSF Memory Allocator Design

## Overview

The Two-Level Segregated Fit (TLSF) allocator is a real-time memory allocator designed for predictable performance and low fragmentation. It provides O(1) allocation and deallocation times with bounded response times.

## Architecture

### Core Components

1. **Memory Pool** - Contiguous memory area managed by TLSF
2. **Control Structure** - Metadata including bitmaps and free lists
3. **Block Headers** - Per-block metadata for management
4. **Free Lists** - Segregated lists of free blocks

### Data Structures
Memory Pool Layout:
+----------------------+
| Control Structure |
+----------------------+
| Block Header |
+----------------------+
| User Data Space |
+----------------------+
| Block Header |
+----------------------+
| User Data Space |
+----------------------+
| ... |
+----------------------+

text

### Block Header Structure

```c
typedef struct block_header_t {
    bsize_t prev_phys_size;    // Size of previous physical block
    bsize_t size;              // Size of this block (with status flag)
    block_header_t* next_free; // Next free block in list
    block_header_t* prev_free; // Previous free block in list
} block_header_t;
Algorithm
Two-Level Segregation
TLSF uses two levels of size classes:

First Level (FL) - Logarithmic size classes

Second Level (SL) - Linear sub-classes within each FL

Size Mapping:

text
Size = 2^(FL + shift) + SL * 2^(FL + shift - SL_count_log2)
Bitmaps
FL Bitmap - One bit per first-level list (indicates non-empty lists)

SL Bitmap - Per-FL bitmap indicating non-empty second-level lists

Allocation Algorithm
Size Classification - Map requested size to (FL, SL) indices

Bitmap Search - Find suitable size class using bit operations

Block Selection - Take first block from appropriate free list

Block Splitting - Split block if larger than needed

Update Metadata - Update bitmaps and free lists

Deallocation Algorithm
Block Marking - Mark block as free

Coalescing - Merge with adjacent free blocks

List Insertion - Insert into appropriate free list

Bitmap Update - Update bitmaps if list becomes non-empty

Performance Characteristics
Time Complexity
Allocation: O(1)

Deallocation: O(1)

Reallocation: O(n) for growing blocks (due to copy)

Space Overhead
Per pool: ~1KB for control structure

Per block: 16-24 bytes for header

Alignment padding: 0-15 bytes

Fragmentation
External fragmentation: Reduced by coalescing

Internal fragmentation: Limited by second-level granularity

Configuration Options
Key Parameters
Alignment - Memory alignment (default: 8 bytes)

Min Block Size - Minimum allocatable block (default: 32 bytes)

FL Index Count - Number of first-level classes (default: 14)

SL Index Count - Number of second-level classes (default: 8)

Compile-Time Options
TLSF_USE_LOCKS - Enable thread safety

TLSF_DEBUG - Enable debugging features

TLSF_STATISTICS - Enable statistics collection

Implementation Details
Memory Alignment
All allocations are aligned to TLSF_ALIGN_SIZE. The allocator ensures:

Pool start is properly aligned

All block headers are aligned

User data is aligned

Thread Safety
When TLSF_USE_LOCKS is enabled:

Single mutex protects entire pool

Platform-specific mutex implementation

Optional lock-free operation

Debug Features
When TLSF_DEBUG is enabled:

Memory filling patterns

Integrity checking

Boundary guards

Advantages
Deterministic - Bounded worst-case execution time

Efficient - Low fragmentation and overhead

Scalable - Works well with small and large pools

Portable - Platform-independent core

Limitations
Memory Overhead - Per-block header overhead

Fixed Pool - Cannot grow/shrink after creation

Fragmentation - Still possible with certain allocation patterns

Use Cases
Suitable For
Real-time systems

Embedded systems

Game development

High-performance computing

Not Suitable For
Systems requiring dynamic pool resizing

Very small memory constraints (< 1KB)

Single huge allocations (> 50% of pool)

References
Masmano, M., et al. "TLSF: A New Dynamic Memory Allocator for Real-Time Systems." Real-Time Systems, 2004.

OAR Corporation. "TLSF: Two-Level Segregated Fit Memory Allocator."

text

**docs/flow.md**:
```markdown
# TLSF Memory Allocator Flow

## Allocation Flowchart
┌─────────────────┐
│ tlsf_malloc() │
└────────┬────────┘
│
▼
┌─────────────────┐
│ Adjust size for │
│ alignment/min │
└────────┬────────┘
│
▼
┌─────────────────┐
│ Map size to │
│ (FL, SL) │
└────────┬────────┘
│
▼
┌─────────────────┐
│ Search bitmaps │
│ for suitable │
│ size class │
└────────┬────────┘
│
▼
┌─────────────────┐
│ Remove block │
│ from free list │
└────────┬────────┘
│
▼
┌─────────────────┐
│ Split block if │
│ too large │
└────────┬────────┘
│
▼
┌─────────────────┐
│ Mark block as │
│ used │
└────────┬────────┘
│
▼
┌─────────────────┐
│ Return pointer │
│ to user data │
└─────────────────┘

text

## Deallocation Flowchart
┌─────────────────┐
│ tlsf_free() │
└────────┬────────┘
│
▼
┌─────────────────┐
│ Get block from │
│ pointer │
└────────┬────────┘
│
▼
┌─────────────────┐
│ Mark block as │
│ free │
└────────┬────────┘
│
▼
┌─────────────────┐
│ Check previous │
│ block: free? │
└───────┬─────────┘
│
Yes │ No
┌────┴────┐
▼ ▼
┌─────────┐ ┌─────────┐
│ Remove │ │ Keep │
│ prev │ │ as is │
│ from │ └─────────┘
│ free │ │
│ list │ │
└────┬────┘ │
│ │
└────┬────┘
│
▼
┌─────────────────┐
│ Merge with │
│ previous block │
└───────┬─────────┘
│
▼
┌─────────────────┐
│ Check next │
│ block: free? │
└───────┬─────────┘
│
Yes │ No
┌────┴────┐
▼ ▼
┌───────┐ ┌───────┐
│Remove │ │Keep │
│next │ │as is │
│from │ └───────┘
│free │ │
│list │ │
└──┬────┘ │
│ │
└───┬────┘
│
▼
┌─────────────────┐
│ Merge with │
│ next block │
└───────┬─────────┘
│
▼
┌─────────────────┐
│ Insert merged │
│ block into │
│ free list │
└───────┬─────────┘
│
▼
┌─────────────────┐
│ Update bitmaps │
└─────────────────┘

text

## Reallocation Flowchart
┌─────────────────┐
│ tlsf_realloc() │
└────────┬────────┘
│
▼
┌─────────────────┐
│ Size == 0? │
└───────┬─────────┘
│
Yes │ No
┌────┴────┐
▼ ▼
┌───────┐ ┌─────────────────┐
│ Free │ │ ptr == NULL? │
│block │ └───────┬─────────┘
└───────┘ │
Yes │ No
┌────┴────┐
▼ ▼
┌───────┐ ┌──────────────┐
│Malloc │ │Same size? │
│new │ └──────┬───────┘
│size │ │
└───────┘ Yes │ No
│
▼
┌──────────────┐
│ Shrinking? │
└──────┬───────┘
│
Yes │ No
┌────┴────┐
▼ ▼
┌─────────┐ ┌──────────────┐
│ Try to │ │ Next block │
│ split │ │ free and │
│ block │ │ large enough?│
└────┬────┘ └──────┬───────┘
│ │
┌────┴─────┐ Yes │ No
│Return │ ┌────┴────┐
│split │ │ ▼
│block │ │ ┌──────────────┐
└──────────┘ │ │ Allocate new │
│ │ block and │
▼ │ copy data │
┌───────┴─┐ │ │
│ Merge │ │ │
│ blocks │ │ │
└────┬────┘ │ │
│ │ │
▼ ▼ │
┌──────────────┐ │
│ Return merged│ │
│ block │ │
└──────────────┘ │
▼
┌──────────────┐
│ Return new │
│ block │
└──────────────┘

text

## Memory Layout Transitions

### Initial State
Pool: [FREE: entire pool]

text

### After First Allocation
Pool: [USED: size1][FREE: remaining]

text

### After Multiple Allocations
Pool: [USED1][USED2][USED3][FREE]

text

### After Freeing Middle Block
Pool: [USED1][FREE][USED3][FREE]

text

### After Coalescing
Pool: [USED1][FREE: merged][USED3]

text

### After Split Allocation
Pool: [USED1][USED: part][FREE: part][USED3]

text

## Bitmap Operations Flow

### Finding Free Block
Map size → (FL, SL)

Check SL bitmap for FL

If non-empty: find first set bit ≥ SL

If empty: find next FL with non-empty bitmap

Get block from corresponding free list

text

### Updating Bitmaps on Free
Insert block into free list

If list was empty:

Set SL bit for (FL, SL)

If SL bitmap was empty: set FL bit

text

### Updating Bitmaps on Allocation
Remove block from free list

If list becomes empty:

Clear SL bit for (FL, SL)

If SL bitmap becomes empty: clear FL bit

text

## Error Handling Flow

### Allocation Failure
Size validation

Pool existence check

Bitmap search failure

Return NULL

text

### Invalid Free
Pointer validation

Block header validation

Double-free detection

Ignore or assert (debug mode)

text

### Pool Corruption
Magic number check

Block size validation

Pointer alignment check

Boundary checks

text

## Performance Optimization Paths

### Fast Path (Common Case)
- Size fits in small class
- Free list non-empty
- No splitting needed

### Slow Path (Complex Case)
- Large allocation
- Need to search higher classes
- Block splitting required
- Coalescing on free

### Critical Path Analysis
- Bitmap operations: O(1)
- Free list operations: O(1)
- Block splitting: O(1)
- Coalescing: O(1)

