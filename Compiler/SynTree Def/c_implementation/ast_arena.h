/*
 * ast_arena.h — UniLogic XPile arena allocator interface
 *
 * The arena owns four flat arrays:
 *   nodes[]      — all UL_Node structs, indexed by NodeId
 *   spans[]      — parallel source-location array, indexed by NodeId
 *   child_pool[] — packed child-list storage, indexed by ChildIdx
 *   str_buf[]    — interned string bytes, indexed by StrId
 *
 * Plus a hash-table (str_table) for the string interning lookup.
 *
 * All allocations are O(1) amortized (bump pointer with doubling growth).
 * The entire arena is freed in one call to arena_destroy().
 * No individual node free is possible or needed.
 *
 * NodeId 0 (UL_NULL_NODE) is reserved — node_count starts at 1.
 */

#ifndef UL_AST_ARENA_H
#define UL_AST_ARENA_H

#include <stdint.h>
#include <stddef.h>
#include "ast_nodes.h"

/* =========================================================================
 * String interning table entry
 * ========================================================================= */

typedef struct {
    uint32_t offset;  /* byte offset in str_buf — this is the StrId value */
    uint32_t length;  /* byte count (NOT including any null terminator) */
    uint32_t hash;    /* cached FNV-1a hash — avoids recomputing on lookup */
    uint32_t _pad;    /* keep 16-byte entries for alignment */
} UL_StrEntry; /* 16 bytes */

/* =========================================================================
 * Arena — the single allocator object for one compilation unit
 * ========================================================================= */

typedef struct {
    /* ── Node pool ──────────────────────────────────────────────────── */
    UL_Node  *nodes;        /* flat array; nodes[0] is the null sentinel */
    uint32_t  node_count;   /* next free index; starts at 1 */
    uint32_t  node_cap;     /* current allocated capacity */

    /* ── Source span pool (parallel to nodes[]) ─────────────────────── */
    UL_Span  *spans;        /* spans[i] = source location of nodes[i] */
    uint32_t  span_cap;     /* same capacity as node pool */

    /* ── Child pool ─────────────────────────────────────────────────── */
    NodeId   *child_pool;   /* packed NodeId arrays for all child lists */
    uint32_t  child_count;  /* next free slot */
    uint32_t  child_cap;    /* current allocated capacity */

    /* ── String buffer ──────────────────────────────────────────────── */
    char     *str_buf;      /* raw interned string bytes, null-terminated */
    uint32_t  str_len;      /* next free byte offset */
    uint32_t  str_cap;      /* current allocated capacity */

    /* ── String interning hash table (open-addressing, linear probe) ── */
    UL_StrEntry *str_table;
    uint32_t     str_table_cap;    /* must be a power of two */
    uint32_t     str_table_count;  /* number of occupied entries */

    /* ── Pre-interned well-known strings ────────────────────────────── */
    UL_WellKnown wk;

} UL_Arena;

/* =========================================================================
 * Initial capacities
 *
 * These cover most programs without triggering a realloc.
 * All can grow by doubling (arena_alloc_node / arena_intern handle this).
 * ========================================================================= */

#define UL_INIT_NODES    65536u    /* 65536 nodes × 32B = 2 MB             */
#define UL_INIT_CHILDREN 262144u   /* 256K child slots × 4B = 1 MB         */
#define UL_INIT_STRBUF   1048576u  /* 1 MB string buffer                   */
#define UL_INIT_STRTBL   65536u    /* hash table slots (power of two)       */

/* =========================================================================
 * API
 * ========================================================================= */

/*
 * arena_init — initialise a freshly-zeroed UL_Arena.
 *
 * Allocates all four backing arrays to their initial capacities.
 * Pre-interns UniLogic built-in keywords and operators into wk.
 * Reserves NodeId 0 as UL_NULL_NODE (a zero-initialised sentinel node).
 *
 * Returns 0 on success, -1 on OOM.
 */
int arena_init(UL_Arena *a);

/*
 * arena_alloc_node — reserve the next NodeId, zero-initialise the node.
 *
 * The caller sets node->kind and fills node->u after this call.
 * The parallel spans[id] entry is also zeroed; fill it with
 * arena_set_span() if source locations are available.
 *
 * Returns UL_NULL_NODE on OOM (should be treated as fatal).
 * O(1) amortized.
 */
NodeId arena_alloc_node(UL_Arena *a);

/*
 * arena_set_span — record source location for an existing node.
 */
void arena_set_span(UL_Arena *a, NodeId id,
                    uint32_t line, uint16_t col, uint16_t end_col);

/*
 * arena_intern — intern a string of `len` bytes.
 *
 * If an identical string is already interned, returns its existing StrId.
 * Otherwise copies the string into str_buf (null-terminated) and records
 * a new entry in str_table.
 *
 * The returned StrId is the byte offset of the string in str_buf.
 * O(1) average.
 */
StrId arena_intern(UL_Arena *a, const char *s, uint32_t len);

/*
 * arena_intern_cstr — convenience wrapper for null-terminated C strings.
 */
StrId arena_intern_cstr(UL_Arena *a, const char *s);

/*
 * arena_alloc_children — reserve `count` consecutive slots in child_pool.
 *
 * Returns the start index. The caller fills child_pool[start..start+count].
 * O(1) amortized.
 */
ChildIdx arena_alloc_children(UL_Arena *a, uint32_t count);

/*
 * arena_push_child — append one NodeId to a previously-allocated child block.
 *
 * NOTE: only safe immediately after arena_alloc_children while no other
 * child lists have been allocated. For general use, build into a local
 * NodeId[] buffer and commit with arena_commit_children().
 */
void arena_push_child(UL_Arena *a, ChildIdx start, uint32_t idx, NodeId child);

/*
 * arena_commit_children — copy a local NodeId buffer into the child pool.
 *
 * Typical parser pattern:
 *
 *   NodeId buf[256]; uint32_t n = 0;
 *   while (...) buf[n++] = parse_stmt(p, a);
 *   UL_Children ch = arena_commit_children(a, buf, n);
 *
 * Returns an initialised UL_Children struct.
 */
UL_Children arena_commit_children(UL_Arena *a, const NodeId *buf, uint32_t count);

/*
 * str_get — return a pointer into str_buf for the given StrId.
 *
 * The pointer is valid until the next call to arena_intern() that triggers
 * a str_buf realloc. Copy the string if you need it to survive interns.
 */
static inline const char *str_get(const UL_Arena *a, StrId id) {
    return (id == UL_NULL_STR) ? "(null)" : (a->str_buf + id);
}

/*
 * arena_str_len — return the byte length of an interned string.
 *
 * O(log n) binary search over str_table entries sorted by offset.
 * For hot paths, cache the length at intern time if needed.
 */
uint32_t arena_str_len(const UL_Arena *a, StrId id);

/*
 * arena_destroy — free all backing arrays.
 *
 * After this call, all NodeId / StrId / ChildIdx values from this arena
 * are invalid. The UL_Arena struct itself is not freed (caller owns it).
 */
void arena_destroy(UL_Arena *a);

/*
 * arena_stats — print allocation statistics to stdout (for debugging).
 */
void arena_stats(const UL_Arena *a);

/* =========================================================================
 * Child iteration macro
 *
 * Usage:
 *   UL_CHILDREN_FOREACH(arena, node->u.func_decl.body, child_id) {
 *       process(arena, child_id);
 *   }
 * ========================================================================= */

#define UL_CHILDREN_FOREACH(arena, children, var)                           \
    for (uint32_t _cfi = 0, var;                                            \
         _cfi < (uint32_t)(children).count &&                               \
             ((var = (arena)->child_pool[(children).start + _cfi]), 1);     \
         ++_cfi)

#endif /* UL_AST_ARENA_H */
