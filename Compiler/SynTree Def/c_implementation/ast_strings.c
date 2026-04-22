/*
 * ast_strings.c — UniLogic XPile string interning helper implementations
 *
 * The heavy lifting (hash map, buffer management) is in ast_arena.c.
 * This file implements the non-inline string utilities declared in ast_strings.h.
 */

#include <string.h>
#include <stdio.h>
#include "ast_strings.h"

/* =========================================================================
 * ul_str_len
 * ========================================================================= */

uint32_t ul_str_len(const UL_Arena *a, StrId id) {
    return arena_str_len(a, id);
}

/* =========================================================================
 * ul_str_copy
 * ========================================================================= */

uint32_t ul_str_copy(const UL_Arena *a, StrId id, char *buf, uint32_t buf_cap) {
    if (buf_cap == 0) return 0;
    if (id == UL_NULL_STR) {
        buf[0] = '\0';
        return 0;
    }
    const char *src = str_get(a, id);
    uint32_t    len = (uint32_t)strlen(src);
    uint32_t    n   = (len < buf_cap - 1) ? len : buf_cap - 1;
    memcpy(buf, src, n);
    buf[n] = '\0';
    return n;
}
