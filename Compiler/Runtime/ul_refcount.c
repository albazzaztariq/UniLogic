/*
 * ul_refcount.c — UniLogic Reference Counting Memory Mode Implementation
 *
 * Method:   Per-object header with two atomic int32 counts (strong, weak).
 *           Atomic operations use C11 _Atomic semantics.
 *           Strong count: relaxed increment, acq_rel decrement.
 *           Weak count:   relaxed increment, acq_rel decrement.
 *           Destructor called when strong count hits zero.
 *           Header freed when weak count hits zero.
 *
 * Inputs:   obj_ptr — pointer to user data region (after UL_RCHeader).
 * Outputs:  void (retain/release), void* (alloc, weak_lock).
 * Packages: mimalloc — allocation of (header + object) blocks.
 *           stdatomic.h — C11 atomic operations.
 *           string.h   — memset for zeroing freed headers (debug safety).
 */

#include "ul_refcount.h"
#include <mimalloc.h>
#include <stdatomic.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>

/* -------------------------------------------------------------------------
 * Allocation
 * ---------------------------------------------------------------------- */

void *ul_rc_alloc(const UL_TypeInfo *type) {
    assert(type != NULL);

    /* Allocate header + data in one contiguous block. */
    size_t total = sizeof(UL_RCHeader) + type->size;
    UL_RCHeader *hdr = (UL_RCHeader *)mi_malloc(total);
    if (!hdr) return NULL;

    /* Initialise counts.
     * strong_count = 1: caller holds the initial strong reference.
     * weak_count   = 1: sentinel — represents "strong side still alive". */
    atomic_init(&hdr->strong_count, 1);
    atomic_init(&hdr->weak_count,   1);
    hdr->type = type;

    /* Return pointer to user data (immediately after the header). */
    return (uint8_t *)hdr + sizeof(UL_RCHeader);
}

/* -------------------------------------------------------------------------
 * Strong reference operations
 * ---------------------------------------------------------------------- */

void ul_retain(void *obj_ptr) {
    if (!obj_ptr) return;

    UL_RCHeader *hdr = ul_rc_header(obj_ptr);
    /* Relaxed: safe because an existing reference must already be live,
     * which already provided the necessary ordering. */
    atomic_fetch_add_explicit(&hdr->strong_count, 1, memory_order_relaxed);
}

void ul_release(void *obj_ptr) {
    if (!obj_ptr) return;

    UL_RCHeader *hdr = ul_rc_header(obj_ptr);

    /* Acq_rel: the fetch_sub provides:
     *   - release semantics on this thread (all prior writes visible to destructor thread)
     *   - acquire semantics on the thread that observes count == 0
     * This mirrors Swift's ARC decrement ordering. */
    int32_t prev = atomic_fetch_sub_explicit(&hdr->strong_count, 1, memory_order_acq_rel);

    if (prev == 1) {
        /* Strong count just reached zero — run the destructor. */
        if (hdr->type && hdr->type->destructor) {
            hdr->type->destructor(obj_ptr);
        }

        /* Zero out user data to catch use-after-free bugs early. */
        if (hdr->type) {
            memset(obj_ptr, 0, hdr->type->size);
        }

        /* Drop the sentinel from weak_count.
         * If weak_count also reaches zero, free the entire block. */
        int32_t weak_prev = atomic_fetch_sub_explicit(
            &hdr->weak_count, 1, memory_order_acq_rel);

        if (weak_prev == 1) {
            /* No weak references remain — free the whole block. */
            mi_free(hdr);
        }
    }
}

/* -------------------------------------------------------------------------
 * Weak reference operations
 * ---------------------------------------------------------------------- */

void ul_weak_retain(void *obj_ptr) {
    if (!obj_ptr) return;

    UL_RCHeader *hdr = ul_rc_header(obj_ptr);
    atomic_fetch_add_explicit(&hdr->weak_count, 1, memory_order_relaxed);
}

void ul_weak_release(void *obj_ptr) {
    if (!obj_ptr) return;

    UL_RCHeader *hdr = ul_rc_header(obj_ptr);
    int32_t prev = atomic_fetch_sub_explicit(
        &hdr->weak_count, 1, memory_order_acq_rel);

    if (prev == 1) {
        /* weak_count reached zero — all strong AND weak refs are gone. */
        mi_free(hdr);
    }
}

void *ul_weak_lock(void *obj_ptr) {
    if (!obj_ptr) return NULL;

    UL_RCHeader *hdr = ul_rc_header(obj_ptr);

    /* Attempt to atomically increment strong_count only if it is > 0.
     * Uses a compare-exchange loop matching Swift's tryRetain pattern. */
    int32_t current = atomic_load_explicit(&hdr->strong_count, memory_order_relaxed);
    while (current > 0) {
        if (atomic_compare_exchange_weak_explicit(
                &hdr->strong_count,
                &current,
                current + 1,
                memory_order_acq_rel,
                memory_order_relaxed)) {
            /* Successfully promoted — return the strong reference. */
            return obj_ptr;
        }
        /* CAS failed (another thread changed count) — retry with updated current. */
    }

    /* strong_count was 0 — object already deallocated. */
    return NULL;
}
