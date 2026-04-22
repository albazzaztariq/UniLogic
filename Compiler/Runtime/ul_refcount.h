/*
 * ul_refcount.h — UniLogic Reference Counting Memory Mode
 *
 * ARC-style reference counting for UL's "memory: refcount" mode.
 * Mirrors Swift's ARC design: every heap object carries a 16-byte header
 * prepended before its data. The header holds two atomic int32 counts and
 * a pointer to compiler-generated type metadata.
 *
 * Object layout in memory:
 *   [ UL_RCHeader (16 bytes) ][ user data ... ]
 *
 * Pointers returned to user code point PAST the header (to user data).
 * ul_retain / ul_release receive the user-data pointer and offset back
 * to find the header.
 *
 * Strong count rules:
 *   initial = 1 (set by ul_rc_alloc)
 *   retain:  fetch_add 1, memory_order_relaxed
 *   release: fetch_sub 1, memory_order_acq_rel
 *           → if result was 1 (now 0): run destructor, release header memory
 *             when weak_count also reaches 0.
 *
 * Weak count rules:
 *   weak_count starts at 1 (the "+1 for strong > 0" sentinel)
 *   weak_retain:  fetch_add 1, memory_order_relaxed
 *   weak_release: fetch_sub 1, memory_order_acq_rel
 *               → if result was 1 (now 0): mi_free the header+data block
 *   When strong count hits 0: weak_count is decremented by 1 (sentinel drop)
 *     → if that also reaches 0: header freed immediately.
 *
 * Requires: C11 (for _Atomic), mimalloc v3.
 */

#ifndef UL_REFCOUNT_H
#define UL_REFCOUNT_H

#include <stddef.h>
#include <stdint.h>
#include <stdatomic.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declaration — compiler emits one UL_TypeInfo per heap type. */
typedef struct UL_TypeInfo UL_TypeInfo;

/*
 * UL_RCHeader
 * Prepended before every heap object in refcount mode.
 * Size: 16 bytes on 64-bit (4 + 4 + 8).
 *
 * Fields:
 *   strong_count — number of owning (strong) references.
 *                  Object data is live while strong_count > 0.
 *   weak_count   — number of weak references + 1 sentinel (for strong > 0).
 *                  Header memory is live while weak_count > 0.
 *   type         — pointer to compiler-emitted type descriptor.
 *                  Used to call the destructor and recursively release fields.
 */
typedef struct UL_RCHeader {
    _Atomic(int32_t)      strong_count;
    _Atomic(int32_t)      weak_count;
    const UL_TypeInfo    *type;
} UL_RCHeader;

/*
 * UL_TypeInfo
 * Compiler-generated descriptor for each heap-allocated type.
 *
 * Fields:
 *   name        — type name string (for diagnostics)
 *   size        — sizeof(T) in bytes (not including header)
 *   destructor  — called when strong_count reaches 0; receives user-data ptr.
 *                 Must release all child pointer fields before returning.
 *                 May be NULL for types with no pointer fields.
 */
struct UL_TypeInfo {
    const char  *name;
    size_t       size;
    void       (*destructor)(void *obj_ptr);
};

/* -------------------------------------------------------------------------
 * Allocation
 * ---------------------------------------------------------------------- */

/*
 * ul_rc_alloc
 * Allocates sizeof(UL_RCHeader) + type->size bytes from mimalloc.
 * Initialises the header: strong_count = 1, weak_count = 1, type = type.
 * Returns a pointer to the user-data region (immediately after the header).
 * Returns NULL on OOM.
 *
 * The returned pointer has an implicit strong reference count of 1.
 * Caller must eventually call ul_release on it.
 */
void *ul_rc_alloc(const UL_TypeInfo *type);

/* -------------------------------------------------------------------------
 * Strong reference operations
 * ---------------------------------------------------------------------- */

/*
 * ul_retain
 * Increments the strong reference count of the object at `obj_ptr`.
 * obj_ptr must be a user-data pointer returned by ul_rc_alloc.
 * Uses memory_order_relaxed (safe because an existing reference holds the object alive).
 * Cost: ~2-4 ns (atomic fetch_add, relaxed).
 */
void ul_retain(void *obj_ptr);

/*
 * ul_release
 * Decrements the strong reference count.
 * If the count reaches zero:
 *   1. Calls type->destructor(obj_ptr) if non-NULL.
 *   2. Decrements weak_count by 1 (drops the sentinel).
 *   3. If weak_count also reaches zero: calls mi_free on the header block.
 * Uses memory_order_acq_rel on the decrement to synchronise destructor.
 * Cost: ~3-5 ns (non-zero path); ~30-80 ns (zero path, destructor + free).
 * Passing NULL is safe and is a no-op.
 */
void ul_release(void *obj_ptr);

/* -------------------------------------------------------------------------
 * Weak reference operations
 * ---------------------------------------------------------------------- */

/*
 * ul_weak_retain
 * Increments weak_count of the object at `obj_ptr`.
 * Does NOT prevent deallocation of the object data (only the header survives).
 * Used by the compiler for `weak` reference variables.
 * Cost: ~2-4 ns.
 */
void ul_weak_retain(void *obj_ptr);

/*
 * ul_weak_release
 * Decrements weak_count.
 * If weak_count reaches zero AND strong_count is already zero:
 *   the header + data block is freed via mi_free.
 * Cost: ~3-5 ns (non-zero path); ~20-40 ns (zero path, mi_free).
 * Passing NULL is safe and is a no-op.
 */
void ul_weak_release(void *obj_ptr);

/*
 * ul_weak_lock
 * Attempts to promote a weak reference to a strong reference.
 * Returns obj_ptr if strong_count > 0 (increments strong_count atomically).
 * Returns NULL if the object has already been deallocated (strong_count == 0).
 * Callers must call ul_release on the returned pointer when done.
 * This is analogous to Swift's optional binding of a weak reference.
 */
void *ul_weak_lock(void *obj_ptr);

/* -------------------------------------------------------------------------
 * Internal utility: header access
 * ---------------------------------------------------------------------- */

/*
 * ul_rc_header
 * Returns a pointer to the UL_RCHeader for the object at `obj_ptr`.
 * obj_ptr must be a user-data pointer returned by ul_rc_alloc.
 * This is an inline function used by retain/release internals.
 */
static inline UL_RCHeader *ul_rc_header(void *obj_ptr) {
    return (UL_RCHeader *)((uint8_t *)obj_ptr - sizeof(UL_RCHeader));
}

#ifdef __cplusplus
}
#endif

#endif /* UL_REFCOUNT_H */
