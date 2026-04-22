/*
 * ul_gc.h — UniLogic Concurrent GC Memory Mode (Stub)
 *
 * Stub for UL's "memory: gc" mode: concurrent tricolor mark-and-sweep
 * collector running on a dedicated background thread, following Go's GC
 * design (see Memory_Model_Spec.md §2).
 *
 * This file defines the full intended API. Implementations in ul_gc.c are
 * scaffolded (correct signatures, thread setup, basic mark queue) but the
 * full tricolor traversal and concurrent sweep are left for the follow-on
 * implementation phase (see spec §8, priority 4).
 *
 * Algorithm overview:
 *   Phase 1 — STW mark setup (<0.5 ms): snapshot roots, mark gray, resume.
 *   Phase 2 — Concurrent mark: GC thread drains gray queue, mutators run.
 *   Phase 3 — STW mark termination (<0.5 ms): drain residual, verify.
 *   Phase 4 — Concurrent sweep: return white objects to mimalloc free lists.
 *
 * Write barrier (hybrid, Go 1.8+):
 *   On every heap pointer store  ptr->field = newVal  during marking:
 *     shade(old_value)   // gray if white
 *     shade(newVal)      // gray if white
 *     ptr->field = newVal
 *
 * Requires: C11 (threads.h / pthreads), mimalloc v3.
 */

#ifndef UL_GC_H
#define UL_GC_H

#include <stddef.h>
#include <stdint.h>
#include <stdatomic.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------
 * Tricolor object state
 * ---------------------------------------------------------------------- */

/*
 * UL_GCColor
 * Tricolor marking state stored in each object's GC header.
 *   UL_GC_WHITE — not yet visited; candidate for collection.
 *   UL_GC_GRAY  — reachable, children not yet scanned.
 *   UL_GC_BLACK — fully scanned; all children are at least gray.
 */
typedef enum {
    UL_GC_WHITE = 0,
    UL_GC_GRAY  = 1,
    UL_GC_BLACK = 2
} UL_GCColor;

/* -------------------------------------------------------------------------
 * GC phase flag
 * Mutators check this before deciding whether the write barrier is active.
 * ---------------------------------------------------------------------- */

/*
 * UL_GCPhase
 * Global phase of the collector.
 *   UL_GC_PHASE_IDLE    — GC not running; barrier is disabled.
 *   UL_GC_PHASE_MARK    — Concurrent marking; barrier active.
 *   UL_GC_PHASE_SWEEP   — Concurrent sweeping; barrier disabled.
 */
typedef enum {
    UL_GC_PHASE_IDLE  = 0,
    UL_GC_PHASE_MARK  = 1,
    UL_GC_PHASE_SWEEP = 2
} UL_GCPhase;

/* Global phase variable — read by every mutator thread's write barrier check. */
extern _Atomic(UL_GCPhase) ul_gc_phase;

/* -------------------------------------------------------------------------
 * Object header
 * ---------------------------------------------------------------------- */

/*
 * UL_GCHeader
 * Prepended before every GC-managed heap object.
 * Size: 16 bytes on 64-bit.
 *
 * Fields:
 *   color      — tricolor state (atomic for concurrent GC access)
 *   type       — compiler-emitted type descriptor (pointer field offsets)
 *   next_alloc — intrusive linked list of all live GC objects (for sweep)
 */
typedef struct UL_GCHeader {
    _Atomic(uint8_t)          color;       /* UL_GCColor stored as uint8 */
    const struct UL_GCTypeInfo *type;
    struct UL_GCHeader        *next_alloc; /* GC's global alloc list */
} UL_GCHeader;

/*
 * UL_GCTypeInfo
 * Compiler-generated type descriptor for GC-managed types.
 *
 * Fields:
 *   name          — type name string (diagnostics)
 *   size          — sizeof(T) bytes (not including header)
 *   pointer_offsets — array of byte offsets of pointer fields within T
 *   num_pointers  — length of pointer_offsets array
 */
typedef struct UL_GCTypeInfo {
    const char    *name;
    size_t         size;
    const size_t  *pointer_offsets; /* offsets of pointer fields in T */
    size_t         num_pointers;
} UL_GCTypeInfo;

/* -------------------------------------------------------------------------
 * Initialisation and shutdown
 * ---------------------------------------------------------------------- */

/*
 * ul_gc_init
 * Initialises the GC subsystem. Must be called once at program startup
 * before any ul_gc_alloc calls. Spawns the background GC thread.
 * Sets ul_gc_phase to UL_GC_PHASE_IDLE.
 */
void ul_gc_init(void);

/*
 * ul_gc_shutdown
 * Signals the GC thread to stop and waits for it to exit.
 * Must be called before program exit to ensure clean teardown.
 * Runs a final collection cycle to log any remaining live objects.
 */
void ul_gc_shutdown(void);

/* -------------------------------------------------------------------------
 * Allocation
 * ---------------------------------------------------------------------- */

/*
 * ul_gc_alloc
 * Allocates sizeof(UL_GCHeader) + type->size bytes from mimalloc.
 * Registers the allocation with the GC's global object list.
 * Initialises the header: color = WHITE, type = type.
 * Returns a pointer to the user-data region (past the header).
 * Returns NULL on OOM.
 *
 * The caller MUST NOT free the returned pointer — the GC owns it.
 * Cost: ~60-120 ns (mimalloc + GC list registration).
 */
void *ul_gc_alloc(const UL_GCTypeInfo *type);

/* -------------------------------------------------------------------------
 * Write barrier
 * ---------------------------------------------------------------------- */

/*
 * ul_write_barrier_ptr
 * Hybrid write barrier (Go 1.8+ style). Called by compiler-generated code
 * at every heap pointer store during the marking phase.
 *
 * Semantics:
 *   shade(*field_ptr)  // mark old value gray if white
 *   shade(new_val)     // mark new value gray if white
 *   *field_ptr = new_val
 *
 * When ul_gc_phase != UL_GC_PHASE_MARK, this reduces to a plain store.
 * The phase check is an atomic load; cost outside marking: ~1 ns.
 * Cost during marking: ~5-8 ns.
 *
 * Parameters:
 *   field_ptr — address of the pointer field being written
 *   new_val   — new pointer value being stored
 */
void ul_write_barrier_ptr(void **field_ptr, void *new_val);

/* -------------------------------------------------------------------------
 * Manual GC trigger (for testing / explicit control)
 * ---------------------------------------------------------------------- */

/*
 * ul_gc_collect
 * Synchronously triggers a complete GC cycle (mark + sweep).
 * Blocks the calling thread until the cycle completes.
 * Intended for testing and for programs that want explicit control.
 */
void ul_gc_collect(void);

/* -------------------------------------------------------------------------
 * Internal: shade (mark an object gray)
 * ---------------------------------------------------------------------- */

/*
 * ul_gc_shade
 * If `obj_ptr` is a live GC object currently colored WHITE, atomically
 * sets it to GRAY and pushes it onto the mark work queue.
 * Called internally by ul_write_barrier_ptr and the root scanner.
 * obj_ptr is a user-data pointer (the GC header is at obj_ptr - sizeof(UL_GCHeader)).
 * Passing NULL is safe and is a no-op.
 */
void ul_gc_shade(void *obj_ptr);

#ifdef __cplusplus
}
#endif

#endif /* UL_GC_H */
