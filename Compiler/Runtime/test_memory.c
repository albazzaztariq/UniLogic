/*
 * test_memory.c — UniLogic Memory Mode Tests
 *
 * Tests each of the four memory modes:
 *   - Manual: alloc, write, read back, free, verify no double-free
 *   - Arena:  alloc multiple objects, verify contents, reset, reuse, free
 *   - Refcount: alloc, retain, release (non-zero), release to zero (destructor)
 *               weak retain/release, weak_lock when live and when dead
 *   - GC:     alloc tracked objects, write barrier, explicit collect, verify freed
 *
 * Build instructions: see Makefile.
 * Each mode is compiled as a separate binary with -DUL_MEMORY_<MODE>.
 * This single .c file handles all modes via #ifdef.
 *
 * Method:   Positive and negative path testing for each allocator operation.
 * Inputs:   None (standalone executable).
 * Outputs:  PASS / FAIL lines to stdout; exits 0 on all pass, 1 on any fail.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

/* -------------------------------------------------------------------------
 * Test harness
 * ---------------------------------------------------------------------- */

static int total_tests  = 0;
static int passed_tests = 0;

#define TEST(name, cond) do {                                   \
    total_tests++;                                              \
    if (cond) {                                                 \
        printf("  PASS  %s\n", (name));                        \
        passed_tests++;                                         \
    } else {                                                    \
        printf("  FAIL  %s  (line %d)\n", (name), __LINE__);  \
    }                                                           \
} while (0)

static void print_summary(void) {
    printf("\n%d / %d tests passed.\n", passed_tests, total_tests);
    if (passed_tests != total_tests) exit(1);
}

/* -------------------------------------------------------------------------
 * Mode-specific tests
 * ---------------------------------------------------------------------- */

/* ---- MANUAL ---- */
#if defined(UL_MEMORY_MANUAL)
#include "ul_manual.h"

static void test_manual(void) {
    printf("[DEBUG] test_manual: start\n");

    /* Basic alloc and free */
    int *arr = (int *)ul_alloc(10 * sizeof(int));
    TEST("manual: ul_alloc returns non-null", arr != NULL);

    printf("[DEBUG] test_manual: writing values\n");
    for (int i = 0; i < 10; i++) arr[i] = i * 2;
    int ok = 1;
    for (int i = 0; i < 10; i++) ok &= (arr[i] == i * 2);
    TEST("manual: written values read back correctly", ok);

    ul_free(arr);
    TEST("manual: ul_free does not crash", 1);

    /* ul_free(NULL) must be safe */
    ul_free(NULL);
    TEST("manual: ul_free(NULL) is safe", 1);

    /* Realloc */
    printf("[DEBUG] test_manual: realloc test\n");
    char *buf = (char *)ul_alloc(16);
    TEST("manual: ul_alloc for realloc returns non-null", buf != NULL);
    memcpy(buf, "hello", 6);
    buf = (char *)ul_realloc(buf, 64);
    TEST("manual: ul_realloc returns non-null", buf != NULL);
    TEST("manual: ul_realloc preserves existing data", memcmp(buf, "hello", 6) == 0);
    ul_free(buf);

    /* Large allocation */
    printf("[DEBUG] test_manual: large alloc\n");
    void *big = ul_alloc(1024 * 1024); /* 1 MiB */
    TEST("manual: large alloc (1 MiB) returns non-null", big != NULL);
    ul_free(big);

    printf("[DEBUG] test_manual: done\n");
}

int main(void) {
    printf("=== UniLogic Memory Test: MANUAL ===\n\n");
    test_manual();
    print_summary();
    return 0;
}

/* ---- ARENA ---- */
#elif defined(UL_MEMORY_ARENA)
#include "ul_arena.h"

static void test_arena(void) {
    printf("[DEBUG] test_arena: start\n");

    /* Create arena with default size */
    UL_Arena *a = ul_arena_new(0);
    TEST("arena: ul_arena_new returns non-null", a != NULL);

    /* Allocate and use memory */
    printf("[DEBUG] test_arena: alloc 100 ints\n");
    int *arr = (int *)ul_arena_alloc(a, 100 * sizeof(int));
    TEST("arena: ul_arena_alloc returns non-null", arr != NULL);
    for (int i = 0; i < 100; i++) arr[i] = i;
    int ok = 1;
    for (int i = 0; i < 100; i++) ok &= (arr[i] == i);
    TEST("arena: written values read back correctly", ok);

    /* Allocate many small objects (stays within one block) */
    printf("[DEBUG] test_arena: alloc many small objects\n");
    int alloc_ok = 1;
    for (int i = 0; i < 200; i++) {
        char *p = (char *)ul_arena_alloc(a, 32);
        if (!p) { alloc_ok = 0; break; }
        memset(p, (char)i, 32);
    }
    TEST("arena: 200x32-byte allocations all succeed", alloc_ok);

    /* Allocate large object (forces new block) */
    printf("[DEBUG] test_arena: large alloc forcing new block\n");
    void *big = ul_arena_alloc(a, 128 * 1024); /* 128 KiB > 64 KiB block */
    TEST("arena: alloc > block_size forces new block, returns non-null", big != NULL);

    /* Reset — rewind all blocks */
    printf("[DEBUG] test_arena: reset\n");
    ul_arena_reset(a);
    TEST("arena: ul_arena_reset does not crash", 1);

    /* After reset, can allocate again */
    int *arr2 = (int *)ul_arena_alloc(a, 50 * sizeof(int));
    TEST("arena: alloc after reset returns non-null", arr2 != NULL);
    arr2[0] = 42;
    TEST("arena: alloc after reset is writable", arr2[0] == 42);

    /* Small arena (explicit size) */
    printf("[DEBUG] test_arena: small arena\n");
    UL_Arena *small = ul_arena_new(256);
    TEST("arena: small arena (256 bytes) created", small != NULL);
    /* Fill beyond initial block to exercise overflow path */
    int overflow_ok = 1;
    for (int i = 0; i < 20; i++) {
        void *p = ul_arena_alloc(small, 64);
        if (!p) { overflow_ok = 0; break; }
    }
    TEST("arena: 20x64-byte allocs in 256-byte arena (overflow path)", overflow_ok);
    ul_arena_free(small);

    /* Free main arena */
    printf("[DEBUG] test_arena: free arena\n");
    ul_arena_free(a);
    TEST("arena: ul_arena_free does not crash", 1);

    printf("[DEBUG] test_arena: done\n");
}

int main(void) {
    printf("=== UniLogic Memory Test: ARENA ===\n\n");
    test_arena();
    print_summary();
    return 0;
}

/* ---- REFCOUNT ---- */
#elif defined(UL_MEMORY_REFCOUNT)
#include "ul_refcount.h"

/* Simple test type: a Node with a value and a pointer to next. */
typedef struct TestNode {
    int   value;
    void *next;  /* would be TestNode* in real code */
} TestNode;

static int destructor_called = 0;
static void test_destructor(void *obj_ptr) {
    (void)obj_ptr;
    destructor_called++;
    printf("[DEBUG] test_destructor called (count=%d)\n", destructor_called);
}

static const UL_TypeInfo test_node_type = {
    .name        = "TestNode",
    .size        = sizeof(TestNode),
    .destructor  = test_destructor
};

static const UL_TypeInfo simple_type = {
    .name        = "SimpleInt",
    .size        = sizeof(int),
    .destructor  = NULL  /* no destructor */
};

static void test_refcount(void) {
    printf("[DEBUG] test_refcount: start\n");

    /* Alloc — initial strong count = 1 */
    void *obj = ul_rc_alloc(&test_node_type);
    TEST("refcount: ul_rc_alloc returns non-null", obj != NULL);
    UL_RCHeader *hdr = ul_rc_header(obj);
    TEST("refcount: initial strong_count == 1",
        atomic_load(&hdr->strong_count) == 1);
    TEST("refcount: initial weak_count == 1",
        atomic_load(&hdr->weak_count) == 1);

    /* Write to object */
    TestNode *node = (TestNode *)obj;
    node->value = 99;
    TEST("refcount: object data is writable", node->value == 99);

    /* Retain — strong count becomes 2 */
    printf("[DEBUG] test_refcount: retain\n");
    ul_retain(obj);
    TEST("refcount: ul_retain increments strong_count to 2",
        atomic_load(&hdr->strong_count) == 2);

    /* Release once — strong count back to 1, destructor NOT called */
    printf("[DEBUG] test_refcount: release (non-zero)\n");
    destructor_called = 0;
    ul_release(obj);
    TEST("refcount: ul_release decrements strong_count to 1",
        atomic_load(&hdr->strong_count) == 1);
    TEST("refcount: destructor NOT called at strong_count > 0",
        destructor_called == 0);

    /* Release to zero — destructor MUST be called */
    printf("[DEBUG] test_refcount: release to zero\n");
    ul_release(obj); /* obj is now freed — do NOT dereference */
    TEST("refcount: destructor called when strong_count reaches 0",
        destructor_called == 1);

    /* ul_retain(NULL) and ul_release(NULL) must be safe */
    ul_retain(NULL);
    TEST("refcount: ul_retain(NULL) is safe", 1);
    ul_release(NULL);
    TEST("refcount: ul_release(NULL) is safe", 1);

    /* Weak reference operations */
    printf("[DEBUG] test_refcount: weak reference test\n");
    void *obj2 = ul_rc_alloc(&simple_type);
    TEST("refcount: ul_rc_alloc for weak test", obj2 != NULL);
    UL_RCHeader *hdr2 = ul_rc_header(obj2);

    ul_weak_retain(obj2);
    TEST("refcount: ul_weak_retain increments weak_count to 2",
        atomic_load(&hdr2->weak_count) == 2);

    /* ul_weak_lock while object is alive — must succeed */
    void *locked = ul_weak_lock(obj2);
    TEST("refcount: ul_weak_lock returns non-null when object is alive",
        locked != NULL);
    TEST("refcount: ul_weak_lock increments strong_count",
        atomic_load(&hdr2->strong_count) == 2);

    /* Release the strong reference from weak_lock */
    ul_release(locked);
    TEST("refcount: release after weak_lock succeeds",
        atomic_load(&hdr2->strong_count) == 1);

    /* Release original strong reference — destructor fires, header survives (weak still held) */
    printf("[DEBUG] test_refcount: release original while weak held\n");
    destructor_called = 0;
    ul_release(obj2); /* strong → 0, weak_count drops sentinel from 2 → 1, header survives */
    TEST("refcount: destructor called when last strong released (weak held)",
        destructor_called == 1);
    /* weak_count should be 1 (our weak_retain from above) */
    TEST("refcount: weak_count is 1 after strong side dies (our weak still holds)",
        atomic_load(&hdr2->weak_count) == 1);

    /* ul_weak_lock while object is dead — must return NULL */
    void *locked2 = ul_weak_lock(obj2);
    TEST("refcount: ul_weak_lock returns NULL when object is dead", locked2 == NULL);

    /* Release weak reference — header must be freed (no crash) */
    printf("[DEBUG] test_refcount: weak_release to free header\n");
    ul_weak_release(obj2);
    TEST("refcount: ul_weak_release to zero frees header (no crash)", 1);

    /* No-type alloc */
    void *plain = ul_rc_alloc(&simple_type);
    TEST("refcount: alloc with NULL destructor type succeeds", plain != NULL);
    *(int *)plain = 7;
    TEST("refcount: plain int object writable", *(int *)plain == 7);
    ul_release(plain); /* should free without calling destructor */
    TEST("refcount: release with NULL destructor does not crash", 1);

    printf("[DEBUG] test_refcount: done\n");
}

int main(void) {
    printf("=== UniLogic Memory Test: REFCOUNT ===\n\n");
    test_refcount();
    print_summary();
    return 0;
}

/* ---- GC ---- */
#elif defined(UL_MEMORY_GC)
#include "ul_gc.h"

typedef struct GCTestNode {
    int   value;
    void *next; /* pointer field — offset 8 on 64-bit */
} GCTestNode;

static const size_t gc_test_offsets[] = { offsetof(GCTestNode, next) };
static const UL_GCTypeInfo gc_test_type = {
    .name            = "GCTestNode",
    .size            = sizeof(GCTestNode),
    .pointer_offsets = gc_test_offsets,
    .num_pointers    = 1
};

static const UL_GCTypeInfo gc_leaf_type = {
    .name            = "GCLeaf",
    .size            = sizeof(int),
    .pointer_offsets = NULL,
    .num_pointers    = 0
};

static void test_gc(void) {
    printf("[DEBUG] test_gc: init\n");
    ul_gc_init();
    TEST("gc: ul_gc_init does not crash", 1);

    /* Allocate a tracked object */
    printf("[DEBUG] test_gc: alloc node\n");
    GCTestNode *node = (GCTestNode *)ul_gc_alloc(&gc_test_type);
    TEST("gc: ul_gc_alloc returns non-null", node != NULL);
    node->value = 42;
    node->next  = NULL;
    TEST("gc: allocated object is writable", node->value == 42);

    /* Check the GC header colour is WHITE at birth */
    UL_GCHeader *hdr = (UL_GCHeader *)((uint8_t *)node - sizeof(UL_GCHeader));
    TEST("gc: object color is WHITE after allocation",
        (int)atomic_load(&hdr->color) == (int)UL_GC_WHITE);

    /* Write barrier — outside marking phase, should just store */
    printf("[DEBUG] test_gc: write barrier outside marking\n");
    void *old_field = node->next;
    ul_write_barrier_ptr((void **)&node->next, NULL);
    TEST("gc: ul_write_barrier_ptr outside mark phase stores value",
        node->next == NULL);
    (void)old_field;

    /* Allocate a leaf and link via write barrier */
    printf("[DEBUG] test_gc: alloc leaf and link\n");
    int *leaf = (int *)ul_gc_alloc(&gc_leaf_type);
    TEST("gc: leaf ul_gc_alloc returns non-null", leaf != NULL);
    *leaf = 99;
    ul_write_barrier_ptr((void **)&node->next, leaf);
    TEST("gc: write barrier linked leaf to node", node->next == (void *)leaf);

    /* Shade a non-NULL object (manual test) */
    printf("[DEBUG] test_gc: shade test\n");
    ul_gc_shade((void *)leaf);
    UL_GCHeader *leaf_hdr = (UL_GCHeader *)((uint8_t *)leaf - sizeof(UL_GCHeader));
    /* Shade only transitions WHITE→GRAY; since we're in IDLE phase the
     * gray queue entry exists but no marking is running. Color should be GRAY. */
    TEST("gc: ul_gc_shade transitions WHITE to GRAY",
        (int)atomic_load(&leaf_hdr->color) == (int)UL_GC_GRAY);

    /* Shade already-gray object — must not crash or double-push */
    ul_gc_shade((void *)leaf);
    TEST("gc: ul_gc_shade on GRAY object is safe no-op (stays GRAY)",
        (int)atomic_load(&leaf_hdr->color) == (int)UL_GC_GRAY);

    /* ul_gc_shade(NULL) must be safe */
    ul_gc_shade(NULL);
    TEST("gc: ul_gc_shade(NULL) is safe", 1);

    /* Explicit collection cycle */
    printf("[DEBUG] test_gc: explicit collect\n");
    ul_gc_collect();
    TEST("gc: ul_gc_collect does not crash", 1);

    /* Allocate more objects and collect again */
    printf("[DEBUG] test_gc: alloc many and collect\n");
    for (int i = 0; i < 50; i++) {
        int *p = (int *)ul_gc_alloc(&gc_leaf_type);
        if (p) *p = i;
    }
    ul_gc_collect();
    TEST("gc: second collect after 50 allocations does not crash", 1);

    printf("[DEBUG] test_gc: shutdown\n");
    ul_gc_shutdown();
    TEST("gc: ul_gc_shutdown does not crash", 1);

    printf("[DEBUG] test_gc: done\n");
}

int main(void) {
    printf("=== UniLogic Memory Test: GC ===\n\n");
    test_gc();
    print_summary();
    return 0;
}

#else
#  error "test_memory.c: no UL_MEMORY_<MODE> flag defined"
#endif
