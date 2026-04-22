/*
 * ul_arena.h — UniLogic Arena Memory Mode
 *
 * Bump-pointer arena allocator for UL's "memory: arena" mode.
 * Memory is organised as a linked list of fixed-size blocks (default 64 KiB)
 * allocated from mimalloc. Allocation is O(1) — a bounds check and pointer
 * bump. Deallocation is bulk-only: free the entire arena at once.
 *
 * API:
 *   ul_arena_new(initial_size)  — create arena with given initial block size
 *   ul_arena_alloc(arena, size) — bump-pointer allocation (16-byte aligned)
 *   ul_arena_reset(arena)       — rewind all blocks, retain backing memory
 *   ul_arena_free(arena)        — release all blocks back to mimalloc
 *
 * Overhead per object: 0 bytes (no per-object header).
 * Allocation fast path: ~2-5 ns (bounds check + pointer add).
 *
 * Requires mimalloc v3.
 */

#ifndef UL_ARENA_H
#define UL_ARENA_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Default block size: 64 KiB — matches mimalloc's internal page size. */
#define UL_ARENA_DEFAULT_BLOCK_SIZE  (64u * 1024u)

/* Alignment for all arena allocations (matches typical SIMD/cache line needs). */
#define UL_ARENA_ALIGN               16u

/*
 * UL_ArenaBlock
 * A single contiguous slab of memory managed by bump pointer.
 * Blocks form a singly-linked list; newest block is at `arena->current`.
 *
 * Fields:
 *   base  — first byte of the slab (returned by mi_malloc)
 *   bump  — next free byte (advances forward on each allocation)
 *   end   — one past the last byte (used for bounds check)
 *   next  — previous block in chain (NULL for the oldest block)
 */
typedef struct UL_ArenaBlock {
    uint8_t              *base;
    uint8_t              *bump;
    uint8_t              *end;
    struct UL_ArenaBlock *next;
} UL_ArenaBlock;

/*
 * UL_Arena
 * The arena handle passed to all arena API functions.
 *
 * Fields:
 *   current           — block currently being allocated from (newest)
 *   first             — oldest block in chain (traversed during free/reset)
 *   default_block_size — size in bytes for newly allocated blocks
 *   parent            — non-NULL for nested child arenas (informational)
 */
typedef struct UL_Arena {
    UL_ArenaBlock *current;
    UL_ArenaBlock *first;
    size_t         default_block_size;
    struct UL_Arena *parent;
} UL_Arena;

/*
 * ul_arena_new
 * Allocates and initialises a new arena.
 * `initial_size` — size in bytes of the first backing block.
 *   Pass 0 to use UL_ARENA_DEFAULT_BLOCK_SIZE (64 KiB).
 * Returns a pointer to the arena, or NULL on allocation failure.
 * The returned arena must be released with ul_arena_free.
 */
UL_Arena *ul_arena_new(size_t initial_size);

/*
 * ul_arena_alloc
 * Allocates `size` bytes from the arena, aligned to UL_ARENA_ALIGN (16).
 * If the current block has insufficient space, a new block is allocated
 * from mimalloc and prepended to the chain.
 * Returns a non-NULL pointer on success. Returns NULL only on OOM.
 * Individual frees are NOT supported — use ul_arena_free or ul_arena_reset.
 */
void *ul_arena_alloc(UL_Arena *arena, size_t size);

/*
 * ul_arena_reset
 * Rewinds all block bump pointers to their base addresses.
 * The backing memory from mimalloc is RETAINED — no mi_free calls.
 * On the next use, allocation is O(1) with zero allocator traffic.
 * All pointers previously returned by ul_arena_alloc become invalid.
 */
void ul_arena_reset(UL_Arena *arena);

/*
 * ul_arena_free
 * Releases all backing blocks to mimalloc and frees the arena header.
 * After this call, the UL_Arena pointer is invalid.
 * All pointers previously returned by ul_arena_alloc become invalid.
 */
void ul_arena_free(UL_Arena *arena);

#ifdef __cplusplus
}
#endif

#endif /* UL_ARENA_H */
