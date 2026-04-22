/*
 * ul_memory.h — UniLogic Unified Memory Interface
 *
 * Single include that selects the correct memory mode implementation
 * based on compile-time flags. Exactly one of the following must be
 * defined when compiling any UniLogic runtime object:
 *
 *   -DUL_MEMORY_MANUAL    → manual mode (mimalloc wrapper)
 *   -DUL_MEMORY_ARENA     → arena mode (bump-pointer allocator)
 *   -DUL_MEMORY_REFCOUNT  → refcount mode (ARC-style)
 *   -DUL_MEMORY_GC        → gc mode (concurrent mark-and-sweep stub)
 *
 * The compiler driver sets the appropriate flag based on the `memory:`
 * key in the source file's DR block. User code does not include this
 * header directly — it is included by the compiler-emitted preamble.
 *
 * If no mode is defined, a compile-time error is raised.
 * If multiple modes are defined simultaneously, a compile-time error is raised.
 */

#ifndef UL_MEMORY_H
#define UL_MEMORY_H

/* -------------------------------------------------------------------------
 * Mutual exclusion guard
 * ---------------------------------------------------------------------- */

#if defined(UL_MEMORY_MANUAL) && defined(UL_MEMORY_ARENA)
#  error "ul_memory.h: cannot define both UL_MEMORY_MANUAL and UL_MEMORY_ARENA"
#endif
#if defined(UL_MEMORY_MANUAL) && defined(UL_MEMORY_REFCOUNT)
#  error "ul_memory.h: cannot define both UL_MEMORY_MANUAL and UL_MEMORY_REFCOUNT"
#endif
#if defined(UL_MEMORY_MANUAL) && defined(UL_MEMORY_GC)
#  error "ul_memory.h: cannot define both UL_MEMORY_MANUAL and UL_MEMORY_GC"
#endif
#if defined(UL_MEMORY_ARENA) && defined(UL_MEMORY_REFCOUNT)
#  error "ul_memory.h: cannot define both UL_MEMORY_ARENA and UL_MEMORY_REFCOUNT"
#endif
#if defined(UL_MEMORY_ARENA) && defined(UL_MEMORY_GC)
#  error "ul_memory.h: cannot define both UL_MEMORY_ARENA and UL_MEMORY_GC"
#endif
#if defined(UL_MEMORY_REFCOUNT) && defined(UL_MEMORY_GC)
#  error "ul_memory.h: cannot define both UL_MEMORY_REFCOUNT and UL_MEMORY_GC"
#endif

/* -------------------------------------------------------------------------
 * Mode selection
 * ---------------------------------------------------------------------- */

#if defined(UL_MEMORY_MANUAL)
/* ---- Manual mode ---- */
#  include "ul_manual.h"

#elif defined(UL_MEMORY_ARENA)
/* ---- Arena mode ---- */
#  include "ul_arena.h"

#elif defined(UL_MEMORY_REFCOUNT)
/* ---- Reference counting mode ---- */
#  include "ul_refcount.h"

#elif defined(UL_MEMORY_GC)
/* ---- GC mode ---- */
#  include "ul_gc.h"

#else
#  error "ul_memory.h: no memory mode defined. Define one of: " \
         "UL_MEMORY_MANUAL, UL_MEMORY_ARENA, UL_MEMORY_REFCOUNT, UL_MEMORY_GC"
#endif

/* -------------------------------------------------------------------------
 * Mode identification macro
 * Provides a human-readable string naming the active mode.
 * Accessible from runtime diagnostics and test output.
 * ---------------------------------------------------------------------- */

#if defined(UL_MEMORY_MANUAL)
#  define UL_MEMORY_MODE_NAME  "manual"
#elif defined(UL_MEMORY_ARENA)
#  define UL_MEMORY_MODE_NAME  "arena"
#elif defined(UL_MEMORY_REFCOUNT)
#  define UL_MEMORY_MODE_NAME  "refcount"
#elif defined(UL_MEMORY_GC)
#  define UL_MEMORY_MODE_NAME  "gc"
#endif

#endif /* UL_MEMORY_H */
