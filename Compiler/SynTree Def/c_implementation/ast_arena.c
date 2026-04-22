/*
 * ast_arena.c — UniLogic XPile arena allocator implementation
 *
 * Four backing arrays managed here:
 *   nodes[]      bump-pointer node pool
 *   spans[]      parallel source locations
 *   child_pool[] packed child lists
 *   str_buf[]    interned string bytes
 *   str_table[]  open-addressing hash map (FNV-1a, linear probing)
 *
 * Growth strategy: doubling (like std::vector). Resizing never invalidates
 * NodeId / StrId / ChildIdx values because they are indices, not pointers.
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "ast_arena.h"

/* =========================================================================
 * Internal helpers
 * ========================================================================= */

/* FNV-1a 32-bit hash — fast, decent distribution for short strings */
static uint32_t fnv1a(const char *s, uint32_t len) {
    uint32_t h = 2166136261u;
    for (uint32_t i = 0; i < len; i++) {
        h ^= (unsigned char)s[i];
        h *= 16777619u;
    }
    return h;
}

/* =========================================================================
 * String table — open addressing, linear probing, load factor <= 0.75
 * ========================================================================= */

/*
 * str_table_find — locate the slot for a string with given hash and content.
 *
 * Returns the slot index. If *found is set to 1, the slot holds an existing
 * match. If *found is 0, the slot is empty and the string should be inserted.
 */
static uint32_t str_table_find(const UL_Arena *a,
                                const char *s, uint32_t len, uint32_t hash,
                                int *found)
{
    uint32_t mask = a->str_table_cap - 1;
    uint32_t slot = hash & mask;

    for (;;) {
        const UL_StrEntry *e = &a->str_table[slot];

        if (e->length == 0 && e->offset == 0 && e->hash == 0) {
            /* Empty slot — string not present */
            *found = 0;
            return slot;
        }

        if (e->hash == hash && e->length == len &&
            memcmp(a->str_buf + e->offset, s, len) == 0)
        {
            /* Found a match */
            *found = 1;
            return slot;
        }

        slot = (slot + 1) & mask;
    }
}

/*
 * str_table_grow — double the hash table and rehash all entries.
 */
static int str_table_grow(UL_Arena *a) {
    uint32_t new_cap = a->str_table_cap * 2;
    UL_StrEntry *new_table = (UL_StrEntry *)calloc(new_cap, sizeof(UL_StrEntry));
    if (!new_table) return -1;

    uint32_t mask = new_cap - 1;
    for (uint32_t i = 0; i < a->str_table_cap; i++) {
        UL_StrEntry *old = &a->str_table[i];
        if (old->length == 0 && old->hash == 0) continue; /* empty */

        uint32_t slot = old->hash & mask;
        while (new_table[slot].length != 0 || new_table[slot].hash != 0) {
            slot = (slot + 1) & mask;
        }
        new_table[slot] = *old;
    }

    free(a->str_table);
    a->str_table     = new_table;
    a->str_table_cap = new_cap;
    return 0;
}

/* =========================================================================
 * arena_init
 * ========================================================================= */

int arena_init(UL_Arena *a) {
    memset(a, 0, sizeof(*a));

    /* ── Node pool ────────────────────────────────────────────────────── */
    a->node_cap  = UL_INIT_NODES;
    a->nodes     = (UL_Node *)calloc(a->node_cap, sizeof(UL_Node));
    a->spans     = (UL_Span *)calloc(a->node_cap, sizeof(UL_Span));
    if (!a->nodes || !a->spans) goto oom;

    /* Reserve NodeId 0 as the null/sentinel node */
    a->nodes[0].kind = 0; /* kind 0 is never a valid NK_ value */
    a->node_count = 1;
    a->span_cap   = a->node_cap;

    /* ── Child pool ───────────────────────────────────────────────────── */
    a->child_cap   = UL_INIT_CHILDREN;
    a->child_pool  = (NodeId *)calloc(a->child_cap, sizeof(NodeId));
    if (!a->child_pool) goto oom;
    a->child_count = 0;

    /* ── String buffer ────────────────────────────────────────────────── */
    a->str_cap = UL_INIT_STRBUF;
    a->str_buf = (char *)malloc(a->str_cap);
    if (!a->str_buf) goto oom;
    /* Reserve offset 0 as an empty string so StrId=0 is never UL_NULL_STR
       confusion. Place a null byte at offset 0. */
    a->str_buf[0] = '\0';
    a->str_len = 1; /* next write starts at offset 1 */

    /* ── String table ─────────────────────────────────────────────────── */
    a->str_table_cap   = UL_INIT_STRTBL;
    a->str_table       = (UL_StrEntry *)calloc(a->str_table_cap,
                                                sizeof(UL_StrEntry));
    if (!a->str_table) goto oom;
    a->str_table_count = 0;

    /* ── Pre-intern well-known strings ────────────────────────────────── */
    a->wk.kw_int    = arena_intern_cstr(a, "int");
    a->wk.kw_float  = arena_intern_cstr(a, "float");
    a->wk.kw_bool   = arena_intern_cstr(a, "bool");
    a->wk.kw_string = arena_intern_cstr(a, "string");
    a->wk.kw_none   = arena_intern_cstr(a, "none");
    a->wk.kw_array  = arena_intern_cstr(a, "array");
    a->wk.kw_list   = arena_intern_cstr(a, "list");
    a->wk.kw_ok     = arena_intern_cstr(a, "ok");
    a->wk.kw_error  = arena_intern_cstr(a, "error");
    a->wk.kw_fixed  = arena_intern_cstr(a, "fixed");

    a->wk.op_add        = arena_intern_cstr(a, "+");
    a->wk.op_sub        = arena_intern_cstr(a, "-");
    a->wk.op_mul        = arena_intern_cstr(a, "*");
    a->wk.op_div        = arena_intern_cstr(a, "/");
    a->wk.op_mod        = arena_intern_cstr(a, "%");
    a->wk.op_eq         = arena_intern_cstr(a, "==");
    a->wk.op_ne         = arena_intern_cstr(a, "!=");
    a->wk.op_lt         = arena_intern_cstr(a, "<");
    a->wk.op_le         = arena_intern_cstr(a, "<=");
    a->wk.op_gt         = arena_intern_cstr(a, ">");
    a->wk.op_ge         = arena_intern_cstr(a, ">=");
    a->wk.op_and        = arena_intern_cstr(a, "and");
    a->wk.op_or         = arena_intern_cstr(a, "or");
    a->wk.op_not        = arena_intern_cstr(a, "not");
    a->wk.op_assign     = arena_intern_cstr(a, "=");
    a->wk.op_add_assign = arena_intern_cstr(a, "+=");
    a->wk.op_sub_assign = arena_intern_cstr(a, "-=");
    a->wk.op_mul_assign = arena_intern_cstr(a, "*=");
    a->wk.op_div_assign = arena_intern_cstr(a, "/=");

    return 0;

oom:
    arena_destroy(a);
    fprintf(stderr, "[arena] OOM during arena_init\n");
    return -1;
}

/* =========================================================================
 * arena_alloc_node
 * ========================================================================= */

NodeId arena_alloc_node(UL_Arena *a) {
    if (a->node_count >= a->node_cap) {
        uint32_t new_cap = a->node_cap * 2;

        UL_Node *new_nodes = (UL_Node *)realloc(a->nodes,
                                                  new_cap * sizeof(UL_Node));
        UL_Span *new_spans = (UL_Span *)realloc(a->spans,
                                                  new_cap * sizeof(UL_Span));
        if (!new_nodes || !new_spans) {
            fprintf(stderr, "[arena] OOM: cannot grow node pool to %u\n",
                    new_cap);
            return UL_NULL_NODE;
        }

        /* Zero-initialise the newly allocated region */
        memset(new_nodes + a->node_cap, 0,
               (new_cap - a->node_cap) * sizeof(UL_Node));
        memset(new_spans + a->node_cap, 0,
               (new_cap - a->node_cap) * sizeof(UL_Span));

        a->nodes    = new_nodes;
        a->spans    = new_spans;
        a->node_cap = new_cap;
        a->span_cap = new_cap;
        /* No NodeId fixup needed: they are indices, not pointers */
    }

    NodeId id = a->node_count++;
    memset(&a->nodes[id], 0, sizeof(UL_Node));
    memset(&a->spans[id], 0, sizeof(UL_Span));
    return id;
}

/* =========================================================================
 * arena_set_span
 * ========================================================================= */

void arena_set_span(UL_Arena *a, NodeId id,
                    uint32_t line, uint16_t col, uint16_t end_col)
{
    if (id == UL_NULL_NODE || id >= a->node_count) return;
    a->spans[id].line    = line;
    a->spans[id].col     = col;
    a->spans[id].end_col = end_col;
}

/* =========================================================================
 * arena_intern
 * ========================================================================= */

StrId arena_intern(UL_Arena *a, const char *s, uint32_t len) {
    uint32_t hash = fnv1a(s, len);

    /* Grow the hash table if load > 75% */
    if (a->str_table_count * 4 >= a->str_table_cap * 3) {
        if (str_table_grow(a) != 0) {
            fprintf(stderr, "[arena] OOM: cannot grow string table\n");
            return UL_NULL_STR;
        }
    }

    int found;
    uint32_t slot = str_table_find(a, s, len, hash, &found);

    if (found) {
        return a->str_table[slot].offset;
    }

    /* Grow str_buf if needed (len + null terminator) */
    while (a->str_len + len + 1 > a->str_cap) {
        uint32_t new_cap = a->str_cap * 2;
        char *new_buf = (char *)realloc(a->str_buf, new_cap);
        if (!new_buf) {
            fprintf(stderr, "[arena] OOM: cannot grow string buffer\n");
            return UL_NULL_STR;
        }
        a->str_buf = new_buf;
        a->str_cap = new_cap;
        /* str_table entries store offsets — still valid after realloc */
    }

    /* Copy string into buffer */
    StrId offset = a->str_len;
    memcpy(a->str_buf + offset, s, len);
    a->str_buf[offset + len] = '\0'; /* null-terminate for C compat */
    a->str_len += len + 1;

    /* Insert into hash table */
    a->str_table[slot].offset = offset;
    a->str_table[slot].length = len;
    a->str_table[slot].hash   = hash;
    a->str_table_count++;

    return offset;
}

/* =========================================================================
 * arena_intern_cstr
 * ========================================================================= */

StrId arena_intern_cstr(UL_Arena *a, const char *s) {
    return arena_intern(a, s, (uint32_t)strlen(s));
}

/* =========================================================================
 * arena_str_len
 * ========================================================================= */

uint32_t arena_str_len(const UL_Arena *a, StrId id) {
    if (id == UL_NULL_STR) return 0;
    /* Linear scan of table — acceptable for debug / error paths. */
    for (uint32_t i = 0; i < a->str_table_cap; i++) {
        if (a->str_table[i].offset == id && a->str_table[i].length != 0) {
            return a->str_table[i].length;
        }
    }
    /* Fallback: measure with strlen (str_buf is null-terminated) */
    return (uint32_t)strlen(a->str_buf + id);
}

/* =========================================================================
 * arena_alloc_children
 * ========================================================================= */

ChildIdx arena_alloc_children(UL_Arena *a, uint32_t count) {
    if (count == 0) return 0;

    if (a->child_count + count > a->child_cap) {
        uint32_t need   = a->child_count + count;
        uint32_t new_cap = a->child_cap * 2;
        if (new_cap < need) new_cap = need * 2;

        NodeId *new_pool = (NodeId *)realloc(a->child_pool,
                                              new_cap * sizeof(NodeId));
        if (!new_pool) {
            fprintf(stderr, "[arena] OOM: cannot grow child pool to %u\n",
                    new_cap);
            return 0;
        }
        /* Zero the new region */
        memset(new_pool + a->child_cap, 0,
               (new_cap - a->child_cap) * sizeof(NodeId));
        a->child_pool = new_pool;
        a->child_cap  = new_cap;
    }

    ChildIdx idx      = a->child_count;
    a->child_count   += count;
    return idx;
}

/* =========================================================================
 * arena_push_child
 * ========================================================================= */

void arena_push_child(UL_Arena *a, ChildIdx start, uint32_t idx, NodeId child) {
    /* Caller must have allocated sufficient space with arena_alloc_children */
    assert(start + idx < a->child_count);
    a->child_pool[start + idx] = child;
}

/* =========================================================================
 * arena_commit_children
 * ========================================================================= */

UL_Children arena_commit_children(UL_Arena *a,
                                   const NodeId *buf, uint32_t count)
{
    UL_Children ch;
    ch.count = (uint16_t)(count > 0xFFFF ? 0xFFFF : count);
    ch._pad  = 0;

    if (count == 0) {
        ch.start = 0;
        return ch;
    }

    ch.start = arena_alloc_children(a, count);
    memcpy(a->child_pool + ch.start, buf, count * sizeof(NodeId));
    return ch;
}

/* =========================================================================
 * arena_destroy
 * ========================================================================= */

void arena_destroy(UL_Arena *a) {
    free(a->nodes);
    free(a->spans);
    free(a->child_pool);
    free(a->str_buf);
    free(a->str_table);
    memset(a, 0, sizeof(*a));
}

/* =========================================================================
 * arena_stats
 * ========================================================================= */

void arena_stats(const UL_Arena *a) {
    printf("=== UL_Arena stats ===\n");
    printf("  nodes:      %u / %u  (%zu KB used)\n",
           a->node_count, a->node_cap,
           (size_t)a->node_count * sizeof(UL_Node) / 1024);
    printf("  spans:      %u (parallel, %zu KB)\n",
           a->node_count,
           (size_t)a->node_count * sizeof(UL_Span) / 1024);
    printf("  child_pool: %u / %u  (%zu KB used)\n",
           a->child_count, a->child_cap,
           (size_t)a->child_count * sizeof(NodeId) / 1024);
    printf("  str_buf:    %u / %u bytes  (%u strings interned)\n",
           a->str_len, a->str_cap, a->str_table_count);
    printf("  str_table:  %u / %u slots (load %.1f%%)\n",
           a->str_table_count, a->str_table_cap,
           a->str_table_cap
               ? 100.0 * a->str_table_count / a->str_table_cap
               : 0.0);

    size_t total =
        (size_t)a->node_cap   * sizeof(UL_Node)     +
        (size_t)a->span_cap   * sizeof(UL_Span)     +
        (size_t)a->child_cap  * sizeof(NodeId)      +
        (size_t)a->str_cap                          +
        (size_t)a->str_table_cap * sizeof(UL_StrEntry);
    printf("  total alloc: %zu KB\n", total / 1024);
}
