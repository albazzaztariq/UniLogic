/*
 * ul_arena.c — UniLogic Arena Memory Mode Implementation
 *
 * Method:   Linked list of fixed-size blocks allocated from mimalloc.
 *           Allocation uses a bump pointer within the current block.
 *           Deallocation is bulk: walk the block chain and call mi_free.
 *
 * Inputs:   See ul_arena.h for function signatures.
 * Outputs:  See ul_arena.h for return values.
 * Packages: mimalloc (Microsoft, MIT licence) — base block allocator.
 *           string.h — memset for arena zeroing.
 */

#include "ul_arena.h"
#include <mimalloc.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

/* Round `v` up to the nearest multiple of `align`. `align` must be a power of 2. */
#define ALIGN_UP(v, align)  (((v) + (align) - 1u) & ~((align) - 1u))

/* Return the larger of two size_t values. */
#define UL_MAX(a, b)  ((a) > (b) ? (a) : (b))

/* -------------------------------------------------------------------------
 * Internal helpers
 * ---------------------------------------------------------------------- */

/*
 * ul_arena_new_block
 * Allocates a new UL_ArenaBlock from mimalloc with at least `block_size`
 * bytes of usable space. The block struct and data are two separate
 * allocations so the block header survives independent of data lifetime.
 *
 * Returns NULL on OOM.
 */
static UL_ArenaBlock *ul_arena_new_block(size_t block_size) {
    UL_ArenaBlock *block = (UL_ArenaBlock *)mi_malloc(sizeof(UL_ArenaBlock));
    if (!block) return NULL;

    uint8_t *data = (uint8_t *)mi_malloc(block_size);
    if (!data) {
        mi_free(block);
        return NULL;
    }

    block->base = data;
    block->bump = data;
    block->end  = data + block_size;
    block->next = NULL;
    return block;
}

/* -------------------------------------------------------------------------
 * Public API
 * ---------------------------------------------------------------------- */

UL_Arena *ul_arena_new(size_t initial_size) {
    if (initial_size == 0) {
        initial_size = UL_ARENA_DEFAULT_BLOCK_SIZE;
    }

    UL_Arena *arena = (UL_Arena *)mi_malloc(sizeof(UL_Arena));
    if (!arena) return NULL;

    UL_ArenaBlock *first_block = ul_arena_new_block(initial_size);
    if (!first_block) {
        mi_free(arena);
        return NULL;
    }

    arena->current            = first_block;
    arena->first              = first_block;
    arena->default_block_size = initial_size;
    arena->parent             = NULL;
    return arena;
}

void *ul_arena_alloc(UL_Arena *arena, size_t size) {
    assert(arena != NULL);
    if (size == 0) size = 1;  /* zero-size alloc returns valid unique pointer */

    /* Align the requested size to UL_ARENA_ALIGN (16 bytes). */
    size_t aligned = ALIGN_UP(size, UL_ARENA_ALIGN);

    /* Fast path: enough space in current block. */
    if (arena->current->bump + aligned <= arena->current->end) {
        void *ptr = arena->current->bump;
        arena->current->bump += aligned;
        return ptr;
    }

    /* Slow path: current block is full — allocate a new block.
     * The new block must fit at least `aligned` bytes. */
    size_t block_size = UL_MAX(arena->default_block_size, aligned);
    UL_ArenaBlock *new_block = ul_arena_new_block(block_size);
    if (!new_block) return NULL;

    /* Prepend new block to the chain (current becomes new, old current
     * is reachable via new_block->next for reset/free traversal). */
    new_block->next  = arena->current;
    arena->current   = new_block;
    /* Update first only if the chain is being walked from `first`.
     * NOTE: first points to the oldest block; current is the newest.
     * We prepend to the head (current), so first stays the same.
     * During free/reset we must traverse from current toward first.
     * To do that we walk via `next` until next == NULL.
     * Since we prepend, the new block's `next` points to the old current,
     * and eventually the chain reaches `first` (which has next == NULL).
     * So `first` does NOT need to be updated here — it remains the tail. */

    /* Allocate from the new block (guaranteed to fit). */
    void *ptr = new_block->bump;
    new_block->bump += aligned;
    return ptr;
}

void ul_arena_reset(UL_Arena *arena) {
    assert(arena != NULL);

    /* Walk every block from current toward first (via next chain)
     * and rewind bump pointer to base. Backing memory is kept. */
    UL_ArenaBlock *block = arena->current;
    while (block != NULL) {
        block->bump = block->base;
        block = block->next;
    }
    /* After reset, start allocating from `current` again (which is now empty). */
}

void ul_arena_free(UL_Arena *arena) {
    assert(arena != NULL);

    /* Walk the block chain from current to first, freeing each block's
     * data buffer and then the block struct itself. */
    UL_ArenaBlock *block = arena->current;
    while (block != NULL) {
        UL_ArenaBlock *next = block->next;
        mi_free(block->base);
        mi_free(block);
        block = next;
    }

    /* Zero the arena header to catch use-after-free bugs early. */
    memset(arena, 0, sizeof(UL_Arena));
    mi_free(arena);
}
