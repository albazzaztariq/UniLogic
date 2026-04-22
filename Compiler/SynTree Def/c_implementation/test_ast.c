/*
 * test_ast.c — UniLogic XPile C AST self-test
 *
 * Builds a hand-crafted AST equivalent to the following UniLogic program:
 *
 *   fixed int MAX = 100
 *
 *   function add(int a, int b) returns int
 *       return a + b
 *
 *   function main() returns int
 *       int x = 0
 *       int y = 42
 *       if x == 0
 *           x = add(x, y)
 *       print x
 *       return x
 *
 * Then prints the AST with ul_debug_print_tree() and arena stats.
 * Verifies _Static_assert sizes at compile time (in ast_nodes.h).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "ast_nodes.h"
#include "ast_arena.h"
#include "ast_strings.h"

/* Forward declarations for debug functions defined in ast_debug.c */
void ul_debug_print_tree(const UL_Arena *a, NodeId root);
void ul_debug_print_node(const UL_Arena *a, NodeId id, int depth);
void ul_debug_dump_strings(const UL_Arena *a);

/* =========================================================================
 * Helpers for building nodes concisely
 * ========================================================================= */

static NodeId make_type(UL_Arena *a, const char *name) {
    NodeId id = arena_alloc_node(a);
    UL_Node *n = &a->nodes[id];
    n->kind = NK_TYPE_NAME;
    n->u.type_name.name       = arena_intern_cstr(a, name);
    n->u.type_name.array_size = 0;
    return id;
}

static NodeId make_identifier(UL_Arena *a, const char *name,
                               uint32_t line, uint16_t col)
{
    NodeId id = arena_alloc_node(a);
    UL_Node *n = &a->nodes[id];
    n->kind          = NK_IDENTIFIER;
    n->u.str_node.str = arena_intern_cstr(a, name);
    arena_set_span(a, id, line, col, 0);
    return id;
}

static NodeId make_int_lit(UL_Arena *a, int64_t value,
                            uint32_t line, uint16_t col)
{
    NodeId id = arena_alloc_node(a);
    UL_Node *n = &a->nodes[id];
    n->kind = NK_INT_LITERAL;
    ul_int_lit_set(n, value);
    arena_set_span(a, id, line, col, 0);
    return id;
}

static NodeId make_param(UL_Arena *a, const char *type_name,
                          const char *param_name)
{
    NodeId id = arena_alloc_node(a);
    UL_Node *n = &a->nodes[id];
    n->kind              = NK_PARAM;
    n->u.param.type_node = make_type(a, type_name);
    n->u.param.name      = arena_intern_cstr(a, param_name);
    n->u.param.default_  = UL_NULL_NODE;
    return id;
}

static NodeId make_binary_op(UL_Arena *a, const char *op,
                              NodeId left, NodeId right,
                              uint32_t line, uint16_t col)
{
    NodeId id = arena_alloc_node(a);
    UL_Node *n = &a->nodes[id];
    n->kind              = NK_BINARY_OP;
    n->u.binary_op.op    = arena_intern_cstr(a, op);
    n->u.binary_op.left  = left;
    n->u.binary_op.right = right;
    arena_set_span(a, id, line, col, 0);
    return id;
}

/* =========================================================================
 * Build the sample AST
 * ========================================================================= */

static NodeId build_program(UL_Arena *a) {

    /* ── fixed int MAX = 100 ────────────────────────────────────────────── */
    NodeId const_type  = make_type(a, "int");
    NodeId const_val   = make_int_lit(a, 100, 1, 1);
    NodeId const_decl  = arena_alloc_node(a);
    {
        UL_Node *n = &a->nodes[const_decl];
        n->kind                 = NK_CONST_DECL;
        n->u.const_decl.type_node = const_type;
        n->u.const_decl.name    = arena_intern_cstr(a, "MAX");
        n->u.const_decl.value   = const_val;
        arena_set_span(a, const_decl, 1, 1, 0);
    }

    /* ── function add(int a, int b) returns int ─────────────────────────── */

    /* params: a, b */
    NodeId param_a = make_param(a, "int", "a");
    NodeId param_b = make_param(a, "int", "b");

    /* body: return a + b */
    NodeId id_a      = make_identifier(a, "a", 3, 16);
    NodeId id_b      = make_identifier(a, "b", 3, 20);
    NodeId add_expr  = make_binary_op(a, "+", id_a, id_b, 3, 18);

    NodeId ret_node  = arena_alloc_node(a);
    {
        UL_Node *n = &a->nodes[ret_node];
        n->kind                   = NK_RETURN;
        n->u.single_child.value   = add_expr;
        arena_set_span(a, ret_node, 3, 9, 0);
    }

    NodeId add_params_buf[2] = { param_a, param_b };
    NodeId add_body_buf[1]   = { ret_node };

    NodeId add_func = arena_alloc_node(a);
    {
        UL_Node *n = &a->nodes[add_func];
        n->kind                       = NK_FUNCTION_DECL;
        n->u.func_decl.name           = arena_intern_cstr(a, "add");
        n->u.func_decl.return_type    = make_type(a, "int");
        n->u.func_decl.params         = arena_commit_children(a, add_params_buf, 2);
        n->u.func_decl.body           = arena_commit_children(a, add_body_buf, 1);
        n->u.func_decl.nested_start   = 0;
        n->aux                        = 0; /* nested_count */
        arena_set_span(a, add_func, 2, 1, 0);
    }

    /* ── function main() returns int ───────────────────────────────────── */

    /* int x = 0 */
    NodeId var_x = arena_alloc_node(a);
    {
        UL_Node *n = &a->nodes[var_x];
        n->kind                  = NK_VAR_DECL;
        n->u.var_decl.type_node  = make_type(a, "int");
        n->u.var_decl.name       = arena_intern_cstr(a, "x");
        n->u.var_decl.init       = make_int_lit(a, 0, 5, 17);
        arena_set_span(a, var_x, 5, 9, 0);
    }

    /* int y = 42 */
    NodeId var_y = arena_alloc_node(a);
    {
        UL_Node *n = &a->nodes[var_y];
        n->kind                  = NK_VAR_DECL;
        n->u.var_decl.type_node  = make_type(a, "int");
        n->u.var_decl.name       = arena_intern_cstr(a, "y");
        n->u.var_decl.init       = make_int_lit(a, 42, 6, 17);
        arena_set_span(a, var_y, 6, 9, 0);
    }

    /* if x == 0 */
    NodeId cond_x    = make_identifier(a, "x", 7, 12);
    NodeId cond_zero = make_int_lit(a, 0, 7, 17);
    NodeId cond      = make_binary_op(a, "==", cond_x, cond_zero, 7, 14);

    /* x = add(x, y) */
    NodeId call_x = make_identifier(a, "x", 8, 21);
    NodeId call_y = make_identifier(a, "y", 8, 24);
    NodeId call_args_buf[2] = { call_x, call_y };

    NodeId call_node = arena_alloc_node(a);
    {
        UL_Node *n = &a->nodes[call_node];
        n->kind          = NK_CALL;
        n->u.call.name   = arena_intern_cstr(a, "add");
        n->u.call.args   = arena_commit_children(a, call_args_buf, 2);
        arena_set_span(a, call_node, 8, 17, 0);
    }

    NodeId assign_tgt = make_identifier(a, "x", 8, 13);
    NodeId assign_node = arena_alloc_node(a);
    {
        UL_Node *n = &a->nodes[assign_node];
        n->kind            = NK_ASSIGN;
        n->u.assign.target = assign_tgt;
        n->u.assign.op     = arena_intern_cstr(a, "=");
        n->u.assign.value  = call_node;
        arena_set_span(a, assign_node, 8, 13, 0);
    }

    NodeId then_body_buf[1] = { assign_node };
    NodeId if_node = arena_alloc_node(a);
    {
        UL_Node *n = &a->nodes[if_node];
        n->kind                    = NK_IF;
        n->u.if_stmt.condition     = cond;
        n->u.if_stmt.then_body     = arena_commit_children(a, then_body_buf, 1);
        n->u.if_stmt.else_body.start = 0;
        n->u.if_stmt.else_body.count = 0;
        arena_set_span(a, if_node, 7, 9, 0);
    }

    /* print x */
    NodeId print_x    = make_identifier(a, "x", 9, 15);
    NodeId print_node = arena_alloc_node(a);
    {
        UL_Node *n = &a->nodes[print_node];
        n->kind                 = NK_PRINT;
        n->u.single_child.value = print_x;
        arena_set_span(a, print_node, 9, 9, 0);
    }

    /* return x */
    NodeId ret_x    = make_identifier(a, "x", 10, 16);
    NodeId ret_main = arena_alloc_node(a);
    {
        UL_Node *n = &a->nodes[ret_main];
        n->kind                 = NK_RETURN;
        n->u.single_child.value = ret_x;
        arena_set_span(a, ret_main, 10, 9, 0);
    }

    /* main body */
    NodeId main_body_buf[5] = {
        var_x, var_y, if_node, print_node, ret_main
    };

    NodeId main_func = arena_alloc_node(a);
    {
        UL_Node *n = &a->nodes[main_func];
        n->kind                       = NK_FUNCTION_DECL;
        n->u.func_decl.name           = arena_intern_cstr(a, "main");
        n->u.func_decl.return_type    = make_type(a, "int");
        n->u.func_decl.params.start   = 0;
        n->u.func_decl.params.count   = 0;
        n->u.func_decl.body           = arena_commit_children(a, main_body_buf, 5);
        n->u.func_decl.nested_start   = 0;
        n->aux                        = 0; /* nested_count */
        arena_set_span(a, main_func, 4, 1, 0);
    }

    /* ── Program ─────────────────────────────────────────────────────────── */
    NodeId prog_decls_buf[3] = { const_decl, add_func, main_func };
    NodeId program = arena_alloc_node(a);
    {
        UL_Node *n = &a->nodes[program];
        n->kind           = NK_PROGRAM;
        n->u.program.decls = arena_commit_children(a, prog_decls_buf, 3);
    }

    return program;
}

/* =========================================================================
 * Verification helpers
 * ========================================================================= */

static int test_string_interning(UL_Arena *a) {
    printf("\n--- Test: string interning ---\n");

    StrId s1 = arena_intern_cstr(a, "hello");
    StrId s2 = arena_intern_cstr(a, "hello");
    StrId s3 = arena_intern_cstr(a, "world");

    if (s1 != s2) {
        fprintf(stderr, "FAIL: same string interned twice gives different StrId\n");
        return 1;
    }
    if (s1 == s3) {
        fprintf(stderr, "FAIL: different strings have same StrId\n");
        return 1;
    }
    if (strcmp(ul_str_get(a, s1), "hello") != 0) {
        fprintf(stderr, "FAIL: ul_str_get returned wrong string\n");
        return 1;
    }
    if (!ul_str_eq(s1, s2)) {
        fprintf(stderr, "FAIL: ul_str_eq says equal strings are not equal\n");
        return 1;
    }
    if (ul_str_eq(s1, s3)) {
        fprintf(stderr, "FAIL: ul_str_eq says different strings are equal\n");
        return 1;
    }

    printf("  PASS: same string deduped correctly (StrId=%u)\n", s1);
    printf("  PASS: different strings have distinct StrIds (%u vs %u)\n", s1, s3);
    printf("  PASS: ul_str_get returns correct content\n");
    printf("  PASS: ul_str_eq correct for both equal and unequal\n");
    return 0;
}

static int test_well_known_strings(UL_Arena *a) {
    printf("\n--- Test: well-known strings ---\n");

    /* Intern the same names again and check they match wk */
    if (arena_intern_cstr(a, "int")   != a->wk.kw_int)    { fprintf(stderr, "FAIL: kw_int\n");   return 1; }
    if (arena_intern_cstr(a, "float") != a->wk.kw_float)  { fprintf(stderr, "FAIL: kw_float\n"); return 1; }
    if (arena_intern_cstr(a, "bool")  != a->wk.kw_bool)   { fprintf(stderr, "FAIL: kw_bool\n");  return 1; }
    if (arena_intern_cstr(a, "+")     != a->wk.op_add)    { fprintf(stderr, "FAIL: op_add\n");   return 1; }
    if (arena_intern_cstr(a, "==")    != a->wk.op_eq)     { fprintf(stderr, "FAIL: op_eq\n");    return 1; }

    printf("  PASS: all well-known StrIds match re-interned values\n");
    return 0;
}

static int test_node_sizes(void) {
    printf("\n--- Test: node sizes (compile-time assertions already enforced) ---\n");
    printf("  sizeof(UL_Node)     = %zu  (expected 32)\n", sizeof(UL_Node));
    printf("  sizeof(UL_Span)     = %zu  (expected 8)\n",  sizeof(UL_Span));
    printf("  sizeof(UL_Children) = %zu  (expected 8)\n",  sizeof(UL_Children));
    printf("  sizeof(UL_NodeFlags)= %zu  (expected 1)\n",  sizeof(UL_NodeFlags));

    int ok = 1;
    if (sizeof(UL_Node)     != 32) { fprintf(stderr, "FAIL: UL_Node != 32\n"); ok = 0; }
    if (sizeof(UL_Span)     !=  8) { fprintf(stderr, "FAIL: UL_Span != 8\n");  ok = 0; }
    if (sizeof(UL_Children) !=  8) { fprintf(stderr, "FAIL: UL_Children != 8\n"); ok = 0; }
    if (sizeof(UL_NodeFlags)!=  1) { fprintf(stderr, "FAIL: UL_NodeFlags != 1\n"); ok = 0; }
    if (ok) printf("  PASS: all sizes correct\n");
    return ok ? 0 : 1;
}

static int test_null_node(UL_Arena *a) {
    printf("\n--- Test: null node sentinel ---\n");
    /* NodeId 0 must exist and have kind=0 (not a valid NK_ value) */
    if (a->nodes[UL_NULL_NODE].kind != 0) {
        fprintf(stderr, "FAIL: null node kind != 0\n");
        return 1;
    }
    printf("  PASS: nodes[0].kind == 0 (null sentinel)\n");
    return 0;
}

static int test_child_pool(UL_Arena *a) {
    printf("\n--- Test: child pool ---\n");

    NodeId buf[5] = { 1, 2, 3, 4, 5 };
    UL_Children ch = arena_commit_children(a, buf, 5);

    if (ch.count != 5) {
        fprintf(stderr, "FAIL: child count mismatch (%u != 5)\n", ch.count);
        return 1;
    }

    int ok = 1;
    for (uint16_t i = 0; i < 5; i++) {
        NodeId got = a->child_pool[ch.start + i];
        if (got != (NodeId)(i + 1)) {
            fprintf(stderr, "FAIL: child[%u] = %u, expected %u\n", i, got, i + 1);
            ok = 0;
        }
    }
    if (ok) printf("  PASS: 5-element child list committed and read back correctly\n");

    /* Iteration macro */
    uint32_t sum = 0;
    UL_CHILDREN_FOREACH(a, ch, cid) {
        sum += cid;
    }
    if (sum != 15) {
        fprintf(stderr, "FAIL: UL_CHILDREN_FOREACH sum=%u expected 15\n", sum);
        return 1;
    }
    printf("  PASS: UL_CHILDREN_FOREACH sums to %u\n", sum);
    return ok ? 0 : 1;
}

/* =========================================================================
 * main
 * ========================================================================= */

int main(void) {
    printf("=================================================\n");
    printf("  UniLogic XPile C AST — test_ast\n");
    printf("=================================================\n");

    UL_Arena arena;
    if (arena_init(&arena) != 0) {
        fprintf(stderr, "FATAL: arena_init failed\n");
        return 1;
    }

    int failures = 0;

    failures += test_node_sizes();
    failures += test_null_node(&arena);
    failures += test_string_interning(&arena);
    failures += test_well_known_strings(&arena);
    failures += test_child_pool(&arena);

    /* ── Build and print the sample program AST ─────────────────────── */
    printf("\n--- Building sample program AST ---\n");
    NodeId root = build_program(&arena);
    printf("  Root NodeId = %u\n", root);

    printf("\n");
    ul_debug_print_tree(&arena, root);

    printf("\n--- Interned strings ---\n");
    ul_debug_dump_strings(&arena);

    printf("\n--- Arena stats ---\n");
    arena_stats(&arena);

    /* ── Summary ──────────────────────────────────────────────────────── */
    printf("\n=================================================\n");
    if (failures == 0) {
        printf("  ALL TESTS PASSED\n");
    } else {
        printf("  %d TEST(S) FAILED\n", failures);
    }
    printf("=================================================\n");

    arena_destroy(&arena);
    return failures != 0 ? 1 : 0;
}
