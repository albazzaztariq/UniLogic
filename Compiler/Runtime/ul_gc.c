/*
 * ul_gc.c — UniLogic Concurrent GC Memory Mode (Stub Implementation)
 *
 * Method:   Concurrent tricolor mark-and-sweep, Go-style.
 *           This file provides the scaffolding: thread creation, phase
 *           transitions, mark queue, write barrier dispatch, and sweep
 *           skeleton. The full concurrent traversal is marked TODO
 *           and left for the Phase 4 implementation sprint.
 *
 * Inputs:   See ul_gc.h for function signatures.
 * Outputs:  See ul_gc.h for return values.
 * Packages: mimalloc — base allocator for all GC-managed objects.
 *           pthreads (POSIX) / CreateThread (Windows) — GC background thread.
 *           stdatomic.h — C11 atomic operations for phase flag, colors.
 *           string.h    — memset for zeroing freed objects.
 *
 * STUB STATUS: The following are fully implemented:
 *   - ul_gc_alloc: allocation, GC list registration, header init
 *   - ul_write_barrier_ptr: phase-gated barrier with shade dispatch
 *   - ul_gc_shade: color CAS + gray queue push
 *   - ul_gc_init / ul_gc_shutdown: thread lifecycle
 *   - ul_gc_collect: synchronous collection trigger (full cycle)
 *
 * The following are STUBS (correct structure, not yet fully implemented):
 *   - gc_mark_phase: root scanning (stack/global scan is TODO)
 *   - gc_sweep_phase: object reclamation (iterates list, frees white objects)
 */

#include "ul_gc.h"
#include <mimalloc.h>
#include <stdatomic.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <stdio.h>

#if defined(_WIN32)
#  include <windows.h>
#  define UL_THREAD_T             HANDLE
#  define UL_THREAD_CREATE(t, fn) ((t) = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)(fn), NULL, 0, NULL))
#  define UL_THREAD_JOIN(t)       WaitForSingleObject((t), INFINITE)
#  define UL_MUTEX_T              CRITICAL_SECTION
#  define UL_MUTEX_INIT(m)        InitializeCriticalSection(&(m))
#  define UL_MUTEX_LOCK(m)        EnterCriticalSection(&(m))
#  define UL_MUTEX_UNLOCK(m)      LeaveCriticalSection(&(m))
#  define UL_MUTEX_DESTROY(m)     DeleteCriticalSection(&(m))
#else
#  include <pthread.h>
#  define UL_THREAD_T             pthread_t
#  define UL_THREAD_CREATE(t, fn) pthread_create(&(t), NULL, (fn), NULL)
#  define UL_THREAD_JOIN(t)       pthread_join((t), NULL)
#  define UL_MUTEX_T              pthread_mutex_t
#  define UL_MUTEX_INIT(m)        pthread_mutex_init(&(m), NULL)
#  define UL_MUTEX_LOCK(m)        pthread_mutex_lock(&(m))
#  define UL_MUTEX_UNLOCK(m)      pthread_mutex_unlock(&(m))
#  define UL_MUTEX_DESTROY(m)     pthread_mutex_destroy(&(m))
#endif

/* -------------------------------------------------------------------------
 * Global state
 * ---------------------------------------------------------------------- */

/* GC phase — read by all mutator threads' write barriers. */
_Atomic(UL_GCPhase) ul_gc_phase = UL_GC_PHASE_IDLE;

/* Intrusive singly-linked list of all GC-managed objects.
 * Protected by gc_list_mutex for concurrent access during allocation. */
static UL_GCHeader  *gc_alloc_list      = NULL;
static UL_MUTEX_T    gc_list_mutex;

/* Gray work queue — simple growable array of user-data pointers.
 * In full implementation this becomes a concurrent work-stealing queue. */
#define GC_GRAY_QUEUE_INITIAL_CAP  1024
static void        **gc_gray_queue      = NULL;
static size_t        gc_gray_head       = 0;   /* next dequeue index */
static size_t        gc_gray_tail       = 0;   /* next enqueue index */
static size_t        gc_gray_cap        = 0;
static UL_MUTEX_T    gc_gray_mutex;

/* GC background thread. */
static UL_THREAD_T   gc_thread;
static _Atomic(int)  gc_running         = 0;   /* 1 while thread alive */
static _Atomic(int)  gc_collect_request = 0;   /* 1 = trigger collection */
static _Atomic(int)  gc_collect_done    = 0;   /* 1 = cycle complete */

/* Statistics (informational). */
static _Atomic(size_t) gc_total_allocated = 0;
static _Atomic(size_t) gc_total_freed     = 0;

/* -------------------------------------------------------------------------
 * Internal: header access
 * ---------------------------------------------------------------------- */

static inline UL_GCHeader *gc_header_of(void *obj_ptr) {
    return (UL_GCHeader *)((uint8_t *)obj_ptr - sizeof(UL_GCHeader));
}

/* -------------------------------------------------------------------------
 * Gray queue operations
 * ---------------------------------------------------------------------- */

static void gc_gray_push(void *obj_ptr) {
    UL_MUTEX_LOCK(gc_gray_mutex);

    if (gc_gray_tail == gc_gray_cap) {
        /* Grow the queue. */
        size_t new_cap = gc_gray_cap == 0 ? GC_GRAY_QUEUE_INITIAL_CAP : gc_gray_cap * 2;
        void **new_buf = (void **)mi_realloc(gc_gray_queue, new_cap * sizeof(void *));
        if (!new_buf) {
            /* OOM: drop the gray entry; may cause false live objects, never unsafety. */
            UL_MUTEX_UNLOCK(gc_gray_mutex);
            return;
        }
        gc_gray_queue = new_buf;
        gc_gray_cap   = new_cap;
    }

    gc_gray_queue[gc_gray_tail++] = obj_ptr;
    UL_MUTEX_UNLOCK(gc_gray_mutex);
}

static void *gc_gray_pop(void) {
    UL_MUTEX_LOCK(gc_gray_mutex);
    void *obj = NULL;
    if (gc_gray_head < gc_gray_tail) {
        obj = gc_gray_queue[gc_gray_head++];
        /* Compact queue when head passes halfway. */
        if (gc_gray_head > gc_gray_cap / 2) {
            size_t remaining = gc_gray_tail - gc_gray_head;
            memmove(gc_gray_queue, gc_gray_queue + gc_gray_head, remaining * sizeof(void *));
            gc_gray_head = 0;
            gc_gray_tail = remaining;
        }
    }
    UL_MUTEX_UNLOCK(gc_gray_mutex);
    return obj;
}

/* -------------------------------------------------------------------------
 * ul_gc_shade — mark an object gray
 * ---------------------------------------------------------------------- */

void ul_gc_shade(void *obj_ptr) {
    if (!obj_ptr) return;

    UL_GCHeader *hdr = gc_header_of(obj_ptr);
    uint8_t expected = UL_GC_WHITE;
    /* Atomically transition WHITE → GRAY. */
    if (atomic_compare_exchange_strong_explicit(
            &hdr->color,
            &expected,
            (uint8_t)UL_GC_GRAY,
            memory_order_acq_rel,
            memory_order_relaxed)) {
        gc_gray_push(obj_ptr);
    }
    /* If already GRAY or BLACK, nothing to do. */
}

/* -------------------------------------------------------------------------
 * Write barrier
 * ---------------------------------------------------------------------- */

void ul_write_barrier_ptr(void **field_ptr, void *new_val) {
    /* Fast path: barrier disabled outside marking phase. */
    if (atomic_load_explicit(&ul_gc_phase, memory_order_relaxed) != UL_GC_PHASE_MARK) {
        *field_ptr = new_val;
        return;
    }

    /* Slow path: shade old value and new value, then write. */
    ul_gc_shade(*field_ptr); /* old value */
    ul_gc_shade(new_val);    /* new value */
    *field_ptr = new_val;
}

/* -------------------------------------------------------------------------
 * Mark phase
 * ---------------------------------------------------------------------- */

/*
 * gc_mark_roots — STUB
 * Scans program roots and shades them gray.
 * Full implementation: walk all thread stacks, global variables, registers.
 * This stub shades nothing — the full root scanner is a Phase 4 deliverable.
 */
static void gc_mark_roots(void) {
    /* TODO: Stop-the-world. Snapshot stack pointers for all threads.
     * Walk each frame and shade any pointer-looking value that falls
     * within the GC heap address range.
     * Walk all global UL_GCHeader* roots registered at startup.
     * Resume mutators. */
    fprintf(stderr, "[UL_GC] gc_mark_roots: STUB — no roots scanned\n");
}

/*
 * gc_drain_gray — drain the gray work queue.
 * For each gray object: mark it BLACK, then shade all its pointer fields.
 */
static void gc_drain_gray(void) {
    void *obj_ptr;
    while ((obj_ptr = gc_gray_pop()) != NULL) {
        UL_GCHeader *hdr = gc_header_of(obj_ptr);

        /* Mark black: fully scanned. */
        atomic_store_explicit(&hdr->color, (uint8_t)UL_GC_BLACK, memory_order_relaxed);

        /* Shade all pointer fields using the type's pointer offset table. */
        const UL_GCTypeInfo *type = hdr->type;
        if (type && type->pointer_offsets) {
            for (size_t i = 0; i < type->num_pointers; i++) {
                void **field = (void **)((uint8_t *)obj_ptr + type->pointer_offsets[i]);
                ul_gc_shade(*field);
            }
        }
    }
}

/*
 * gc_mark_phase — full marking cycle.
 * STW mark setup → concurrent mark → STW mark termination.
 */
static void gc_mark_phase(void) {
    /* Transition to marking phase — enables write barriers in mutators. */
    atomic_store_explicit(&ul_gc_phase, UL_GC_PHASE_MARK, memory_order_seq_cst);

    /* STW: Mark setup — shade all roots. */
    gc_mark_roots();

    /* Concurrent mark — drain gray queue.
     * Mutators are running concurrently; write barriers keep the invariant. */
    gc_drain_gray();

    /* STW: Mark termination — drain any residual grays produced by barriers. */
    gc_drain_gray();
}

/* -------------------------------------------------------------------------
 * Sweep phase
 * ---------------------------------------------------------------------- */

/*
 * gc_sweep_phase — reclaim all white objects.
 * Walks the global alloc list. Any object still WHITE at this point is
 * unreachable and can be freed. BLACK objects are reset to WHITE for the
 * next cycle.
 */
static void gc_sweep_phase(void) {
    atomic_store_explicit(&ul_gc_phase, UL_GC_PHASE_SWEEP, memory_order_seq_cst);

    UL_MUTEX_LOCK(gc_list_mutex);

    UL_GCHeader  *prev    = NULL;
    UL_GCHeader  *current = gc_alloc_list;

    while (current != NULL) {
        UL_GCHeader *next = current->next_alloc;
        uint8_t color = atomic_load_explicit(&current->color, memory_order_relaxed);

        if (color == UL_GC_WHITE) {
            /* Unreachable — reclaim. */
            if (prev) {
                prev->next_alloc = next;
            } else {
                gc_alloc_list = next;
            }
            void *obj_ptr = (uint8_t *)current + sizeof(UL_GCHeader);
            const UL_GCTypeInfo *type = current->type;
            if (type) {
                memset(obj_ptr, 0, type->size);
            }
            mi_free(current);
            atomic_fetch_add_explicit(&gc_total_freed, 1, memory_order_relaxed);
        } else {
            /* Reachable — reset to WHITE for next cycle, keep in list. */
            atomic_store_explicit(&current->color, (uint8_t)UL_GC_WHITE, memory_order_relaxed);
            prev = current;
        }

        current = next;
    }

    UL_MUTEX_UNLOCK(gc_list_mutex);

    /* Return to idle. */
    atomic_store_explicit(&ul_gc_phase, UL_GC_PHASE_IDLE, memory_order_seq_cst);
}

/* -------------------------------------------------------------------------
 * GC background thread
 * ---------------------------------------------------------------------- */

static void gc_run_cycle(void) {
    gc_mark_phase();
    gc_sweep_phase();
}

#if defined(_WIN32)
static DWORD WINAPI gc_thread_func(LPVOID arg) {
    (void)arg;
    while (atomic_load_explicit(&gc_running, memory_order_relaxed)) {
        if (atomic_exchange_explicit(&gc_collect_request, 0, memory_order_acq_rel)) {
            gc_run_cycle();
            atomic_store_explicit(&gc_collect_done, 1, memory_order_release);
        }
        Sleep(10); /* 10 ms idle poll — replace with condvar in full impl */
    }
    return 0;
}
#else
static void *gc_thread_func(void *arg) {
    (void)arg;
    while (atomic_load_explicit(&gc_running, memory_order_relaxed)) {
        if (atomic_exchange_explicit(&gc_collect_request, 0, memory_order_acq_rel)) {
            gc_run_cycle();
            atomic_store_explicit(&gc_collect_done, 1, memory_order_release);
        }
        /* TODO: Replace busy-poll with pthread_cond_wait for production. */
        struct timespec ts = {0, 10 * 1000 * 1000}; /* 10 ms */
        nanosleep(&ts, NULL);
    }
    return NULL;
}
#endif

/* -------------------------------------------------------------------------
 * Public API
 * ---------------------------------------------------------------------- */

void ul_gc_init(void) {
    UL_MUTEX_INIT(gc_list_mutex);
    UL_MUTEX_INIT(gc_gray_mutex);

    gc_gray_queue = (void **)mi_malloc(GC_GRAY_QUEUE_INITIAL_CAP * sizeof(void *));
    gc_gray_cap   = GC_GRAY_QUEUE_INITIAL_CAP;
    gc_gray_head  = 0;
    gc_gray_tail  = 0;

    atomic_store_explicit(&gc_running,         1, memory_order_relaxed);
    atomic_store_explicit(&gc_collect_request, 0, memory_order_relaxed);
    atomic_store_explicit(&gc_collect_done,    0, memory_order_relaxed);
    atomic_store_explicit(&ul_gc_phase, UL_GC_PHASE_IDLE, memory_order_relaxed);

    UL_THREAD_CREATE(gc_thread, gc_thread_func);
    fprintf(stderr, "[UL_GC] Initialised. GC thread started.\n");
}

void ul_gc_shutdown(void) {
    /* Signal GC thread to stop. */
    atomic_store_explicit(&gc_running, 0, memory_order_release);
    UL_THREAD_JOIN(gc_thread);

    /* Run a final sweep to report leaked objects. */
    gc_sweep_phase();

    size_t remaining = 0;
    UL_GCHeader *current = gc_alloc_list;
    while (current) { remaining++; current = current->next_alloc; }
    if (remaining > 0) {
        fprintf(stderr, "[UL_GC] Shutdown: %zu object(s) not collected.\n", remaining);
    }

    UL_MUTEX_DESTROY(gc_list_mutex);
    UL_MUTEX_DESTROY(gc_gray_mutex);
    if (gc_gray_queue) {
        mi_free(gc_gray_queue);
        gc_gray_queue = NULL;
    }

    fprintf(stderr, "[UL_GC] Shutdown complete. Allocated: %zu, Freed: %zu.\n",
        (size_t)atomic_load(&gc_total_allocated),
        (size_t)atomic_load(&gc_total_freed));
}

void *ul_gc_alloc(const UL_GCTypeInfo *type) {
    assert(type != NULL);

    size_t total = sizeof(UL_GCHeader) + type->size;
    UL_GCHeader *hdr = (UL_GCHeader *)mi_malloc(total);
    if (!hdr) return NULL;

    atomic_init(&hdr->color, (uint8_t)UL_GC_WHITE);
    hdr->type       = type;
    hdr->next_alloc = NULL;

    /* Register in the global alloc list (prepend — O(1)). */
    UL_MUTEX_LOCK(gc_list_mutex);
    hdr->next_alloc = gc_alloc_list;
    gc_alloc_list   = hdr;
    UL_MUTEX_UNLOCK(gc_list_mutex);

    atomic_fetch_add_explicit(&gc_total_allocated, 1, memory_order_relaxed);

    return (uint8_t *)hdr + sizeof(UL_GCHeader);
}

void ul_gc_collect(void) {
    /* Signal the GC thread and busy-wait for completion.
     * In production this would use a condition variable. */
    atomic_store_explicit(&gc_collect_done,    0, memory_order_release);
    atomic_store_explicit(&gc_collect_request, 1, memory_order_release);

    while (!atomic_load_explicit(&gc_collect_done, memory_order_acquire)) {
#if defined(_WIN32)
        Sleep(1);
#else
        struct timespec ts = {0, 1 * 1000 * 1000}; /* 1 ms */
        nanosleep(&ts, NULL);
#endif
    }
}
