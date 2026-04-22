/*
 * ul_manual.h — UniLogic Manual Memory Mode
 *
 * Thin wrapper over mimalloc for UL's "memory: manual" mode.
 * ul_alloc(size)  → mi_malloc(size)
 * ul_free(ptr)    → mi_free(ptr)
 *
 * The compiler emits these calls directly when memory: manual is set.
 * No GC thread, no reference counting, no background activity.
 *
 * Inputs:  size  — bytes to allocate (must be > 0 for defined behaviour)
 *          ptr   — pointer previously returned by ul_alloc
 * Outputs: ul_alloc returns void* (NULL on OOM); ul_free returns void
 *
 * Requires mimalloc v3 headers. Build with -I<mimalloc-include-path>.
 */

#ifndef UL_MANUAL_H
#define UL_MANUAL_H

#include <stddef.h>
#include <mimalloc.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * ul_alloc
 * Allocates `size` bytes from mimalloc's thread-local heap.
 * Fast path: ~50-100 ns, no lock, no synchronisation.
 * Returns NULL on out-of-memory (mimalloc default behaviour).
 */
void *ul_alloc(size_t size);

/*
 * ul_free
 * Returns memory at `ptr` to mimalloc.
 * ptr must have been returned by ul_alloc (or mi_malloc family).
 * Passing NULL is safe and is a no-op.
 * Fast path: ~20-40 ns thread-local free.
 */
void ul_free(void *ptr);

/*
 * ul_realloc
 * Resizes an existing allocation. Semantics match realloc(3).
 */
void *ul_realloc(void *ptr, size_t new_size);

#ifdef __cplusplus
}
#endif

#endif /* UL_MANUAL_H */
