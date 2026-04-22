/*
 * ul_manual.c — UniLogic Manual Memory Mode Implementation
 *
 * Thin wrapper over mimalloc.
 * All allocation in manual mode routes through these three functions.
 * The compiler emits ul_alloc / ul_free / ul_realloc at memtake/memgive sites.
 *
 * Method:   Direct delegation to mi_malloc / mi_free / mi_realloc.
 * Inputs:   See ul_manual.h
 * Outputs:  See ul_manual.h
 * Packages: mimalloc (Microsoft, MIT licence) — https://github.com/microsoft/mimalloc
 */

#include "ul_manual.h"
#include <mimalloc.h>

void *ul_alloc(size_t size) {
    return mi_malloc(size);
}

void ul_free(void *ptr) {
    mi_free(ptr);
}

void *ul_realloc(void *ptr, size_t new_size) {
    return mi_realloc(ptr, new_size);
}
