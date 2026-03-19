/* __ul_profile.h — UniLogic C profiling instrumentation runtime
 * Included when compiling with --profile. Logs NDJSON to ul_profile.log.
 * All functions are static inline to avoid linker issues. */

#ifndef __UL_PROFILE_H
#define __UL_PROFILE_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
static inline long long __ul_time_ns(void) {
    LARGE_INTEGER freq, counter;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&counter);
    return (long long)((double)counter.QuadPart / freq.QuadPart * 1e9);
}
#else
static inline long long __ul_time_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (long long)ts.tv_sec * 1000000000LL + ts.tv_nsec;
}
#endif

/* Profile log file handle — lazy-opened on first event */
static FILE* __ul_log = NULL;

static inline FILE* __ul_get_log(void) {
    if (!__ul_log) {
        __ul_log = fopen("ul_profile.log", "w");
        if (!__ul_log) {
            fprintf(stderr, "[profile] cannot open ul_profile.log\n");
            __ul_log = stderr;
        }
    }
    return __ul_log;
}

/* Per-function entry time stack (simple fixed-size stack) */
#define __UL_MAX_DEPTH 256
static long long __ul_enter_times[__UL_MAX_DEPTH];
static int __ul_depth = 0;

/* Allocation tracking */
static long long __ul_total_allocs = 0;
static long long __ul_total_bytes = 0;
static long long __ul_live_bytes = 0;
static long long __ul_peak_bytes = 0;

static inline void __ul_enter(const char* func, const char* file, int line) {
    if (__ul_depth < __UL_MAX_DEPTH) {
        __ul_enter_times[__ul_depth] = __ul_time_ns();
    }
    __ul_depth++;
    FILE* f = __ul_get_log();
    fprintf(f, "{\"event\":\"enter\",\"func\":\"%s\",\"time_ns\":%lld,\"file\":\"%s\",\"line\":%d}\n",
            func, __ul_time_ns(), file, line);
}

static inline void __ul_exit(const char* func, const char* file, int line) {
    __ul_depth--;
    long long elapsed = 0;
    if (__ul_depth >= 0 && __ul_depth < __UL_MAX_DEPTH) {
        elapsed = __ul_time_ns() - __ul_enter_times[__ul_depth];
    }
    FILE* f = __ul_get_log();
    fprintf(f, "{\"event\":\"exit\",\"func\":\"%s\",\"time_ns\":%lld,\"elapsed_ns\":%lld,\"file\":\"%s\",\"line\":%d}\n",
            func, __ul_time_ns(), elapsed, file, line);
}

static inline void* __ul_alloc(size_t size) {
    void* ptr = malloc(size);
    __ul_total_allocs++;
    __ul_total_bytes += (long long)size;
    __ul_live_bytes += (long long)size;
    if (__ul_live_bytes > __ul_peak_bytes) __ul_peak_bytes = __ul_live_bytes;
    FILE* f = __ul_get_log();
    fprintf(f, "{\"event\":\"alloc\",\"size\":%lld,\"ptr\":\"%p\",\"time_ns\":%lld}\n",
            (long long)size, ptr, __ul_time_ns());
    return ptr;
}

static inline void __ul_free(void* ptr) {
    FILE* f = __ul_get_log();
    fprintf(f, "{\"event\":\"free\",\"ptr\":\"%p\",\"time_ns\":%lld}\n",
            ptr, __ul_time_ns());
    free(ptr);
}

/* DR behavior monitor hooks */
static inline void __ul_gc_collect(void) {
    FILE* f = __ul_get_log();
    fprintf(f, "{\"event\":\"gc_collect\",\"time_ns\":%lld}\n", __ul_time_ns());
}

static inline void __ul_refcount(void* ptr, int delta) {
    FILE* f = __ul_get_log();
    fprintf(f, "{\"event\":\"refcount\",\"ptr\":\"%p\",\"delta\":%d,\"time_ns\":%lld}\n",
            ptr, delta, __ul_time_ns());
}

static inline void __ul_arena_reset(void) {
    FILE* f = __ul_get_log();
    fprintf(f, "{\"event\":\"arena_reset\",\"time_ns\":%lld}\n", __ul_time_ns());
}

static inline void __ul_profile_dump(void) {
    FILE* f = __ul_get_log();
    fprintf(f, "{\"event\":\"summary\",\"total_allocs\":%lld,\"total_bytes\":%lld,\"peak_live_bytes\":%lld}\n",
            __ul_total_allocs, __ul_total_bytes, __ul_peak_bytes);
    if (__ul_log && __ul_log != stderr) {
        fclose(__ul_log);
        __ul_log = NULL;
    }
}

#endif /* __UL_PROFILE_H */
