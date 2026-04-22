/*
 * ast_strings.h — UniLogic XPile string interning helpers
 *
 * The interning mechanism lives in ast_arena.c (arena_intern / arena_intern_cstr).
 * This header exposes the higher-level helpers that the parser and semantic
 * checker need when working with interned strings.
 *
 * All functions work through a UL_Arena pointer — there is no global state.
 */

#ifndef UL_AST_STRINGS_H
#define UL_AST_STRINGS_H

#include <stdint.h>
#include <stdbool.h>
#include "ast_arena.h"

/* =========================================================================
 * Core access
 * ========================================================================= */

/*
 * ul_str_get — return a C pointer into the string buffer for id.
 *
 * The string is null-terminated in the buffer, so this pointer is directly
 * usable with printf, strcmp, etc.
 *
 * IMPORTANT: the pointer is invalidated if arena_intern() triggers a
 * str_buf realloc. Never store this pointer past the next intern call.
 * Copy with ul_str_copy() if longevity is needed.
 */
static inline const char *ul_str_get(const UL_Arena *a, StrId id) {
    return str_get(a, id);   /* defined inline in ast_arena.h */
}

/*
 * ul_str_len — byte length of an interned string (not counting null term).
 */
uint32_t ul_str_len(const UL_Arena *a, StrId id);

/*
 * ul_str_eq — O(1) equality check for two interned strings.
 *
 * Since every unique string is stored exactly once, two StrIds are equal
 * if and only if their StrId values are equal — no strcmp needed.
 */
static inline bool ul_str_eq(StrId a, StrId b) {
    return a == b;
}

/*
 * ul_str_copy — copy an interned string into a caller-provided buffer.
 *
 * Writes at most buf_cap bytes including null terminator.
 * Returns the number of bytes written (not counting null terminator).
 * Safe to use after subsequent intern calls.
 */
uint32_t ul_str_copy(const UL_Arena *a, StrId id, char *buf, uint32_t buf_cap);

/* =========================================================================
 * Interning shortcuts
 * ========================================================================= */

/*
 * ul_intern — intern a counted string (does NOT need to be null-terminated).
 */
static inline StrId ul_intern(UL_Arena *a, const char *s, uint32_t len) {
    return arena_intern(a, s, len);
}

/*
 * ul_intern_cstr — intern a null-terminated C string.
 */
static inline StrId ul_intern_cstr(UL_Arena *a, const char *s) {
    return arena_intern_cstr(a, s);
}

/* =========================================================================
 * Type-name helpers
 *
 * Conveniences for the common pattern of comparing a StrId against the
 * well-known UniLogic type names pre-interned in arena.wk.
 * ========================================================================= */

static inline bool ul_str_is_int(const UL_Arena *a, StrId s) {
    return ul_str_eq(s, a->wk.kw_int);
}

static inline bool ul_str_is_float(const UL_Arena *a, StrId s) {
    return ul_str_eq(s, a->wk.kw_float);
}

static inline bool ul_str_is_bool(const UL_Arena *a, StrId s) {
    return ul_str_eq(s, a->wk.kw_bool);
}

static inline bool ul_str_is_string(const UL_Arena *a, StrId s) {
    return ul_str_eq(s, a->wk.kw_string);
}

static inline bool ul_str_is_none(const UL_Arena *a, StrId s) {
    return ul_str_eq(s, a->wk.kw_none);
}

static inline bool ul_str_is_array(const UL_Arena *a, StrId s) {
    return ul_str_eq(s, a->wk.kw_array);
}

static inline bool ul_str_is_list(const UL_Arena *a, StrId s) {
    return ul_str_eq(s, a->wk.kw_list);
}

/* =========================================================================
 * Operator helpers
 * ========================================================================= */

static inline bool ul_op_is_add(const UL_Arena *a, StrId op) {
    return ul_str_eq(op, a->wk.op_add);
}

static inline bool ul_op_is_sub(const UL_Arena *a, StrId op) {
    return ul_str_eq(op, a->wk.op_sub);
}

static inline bool ul_op_is_eq(const UL_Arena *a, StrId op) {
    return ul_str_eq(op, a->wk.op_eq);
}

static inline bool ul_op_is_ne(const UL_Arena *a, StrId op) {
    return ul_str_eq(op, a->wk.op_ne);
}

static inline bool ul_op_is_and(const UL_Arena *a, StrId op) {
    return ul_str_eq(op, a->wk.op_and);
}

static inline bool ul_op_is_or(const UL_Arena *a, StrId op) {
    return ul_str_eq(op, a->wk.op_or);
}

static inline bool ul_op_is_not(const UL_Arena *a, StrId op) {
    return ul_str_eq(op, a->wk.op_not);
}

/* =========================================================================
 * StrId validity
 * ========================================================================= */

static inline bool ul_str_valid(StrId id) {
    return id != UL_NULL_STR;
}

#endif /* UL_AST_STRINGS_H */
