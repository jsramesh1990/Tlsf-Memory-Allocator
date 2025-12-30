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
