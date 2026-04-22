/*
 * ast_debug.c — UniLogic XPile AST debug printer
 *
 * Walks the arena's node array recursively and prints a readable
 * indented tree. Output goes to stdout.
 *
 * Usage:
 *   ul_debug_print_node(&arena, program_node_id, 0);
 *
 * Also provides ul_kind_name() which is declared in ast_nodes.h and used
 * throughout the compiler for diagnostics.
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "ast_nodes.h"
#include "ast_arena.h"
#include "ast_strings.h"

/* =========================================================================
 * ul_kind_name — human-readable node kind string
 * ========================================================================= */

const char *ul_kind_name(UL_NodeKind kind) {
    switch (kind) {
        case NK_INT_LITERAL:         return "IntLiteral";
        case NK_FLOAT_LITERAL:       return "FloatLiteral";
        case NK_STRING_LITERAL:      return "StringLiteral";
        case NK_BOOL_LITERAL:        return "BoolLiteral";
        case NK_EMPTY_LITERAL:       return "EmptyLiteral";
        case NK_IDENTIFIER:          return "Identifier";
        case NK_BINARY_OP:           return "BinaryOp";
        case NK_UNARY_OP:            return "UnaryOp";
        case NK_CAST:                return "Cast";
        case NK_CALL:                return "Call";
        case NK_METHOD_CALL:         return "MethodCall";
        case NK_INDEX:               return "Index";
        case NK_SLICE_EXPR:          return "SliceExpr";
        case NK_ARRAY_LITERAL:       return "ArrayLiteral";
        case NK_ARRAY_COMPREHENSION: return "ArrayComprehension";
        case NK_FIELD_ACCESS:        return "FieldAccess";
        case NK_OK_RESULT:           return "OkResult";
        case NK_ERROR_RESULT:        return "ErrorResult";
        case NK_RESULT_PROPAGATION:  return "ResultPropagation";
        case NK_TUPLE_LITERAL:       return "TupleLiteral";
        case NK_FUNC_PTR_VALUE:      return "FuncPtrValue";
        case NK_STRUCT_LITERAL:      return "StructLiteral";
        case NK_VAR_DECL:            return "VarDecl";
        case NK_ASSIGN:              return "Assign";
        case NK_IF:                  return "If";
        case NK_WHILE:               return "While";
        case NK_DO_WHILE:            return "DoWhile";
        case NK_FOR:                 return "For";
        case NK_C_FOR:               return "CFor";
        case NK_RETURN:              return "Return";
        case NK_PRINT:               return "Print";
        case NK_EXPR_STMT:           return "ExprStmt";
        case NK_ESCAPE:              return "Escape";
        case NK_CONTINUE:            return "Continue";
        case NK_PORTAL_DECL:         return "PortalDecl";
        case NK_GOTO_STMT:           return "GotoStmt";
        case NK_KILLSWITCH:          return "KillswitchStmt";
        case NK_TRY_STMT:            return "TryStmt";
        case NK_CATCH_CLAUSE:        return "CatchClause";
        case NK_THROW_STMT:          return "ThrowStmt";
        case NK_POST_INCREMENT:      return "PostIncrement";
        case NK_POST_DECREMENT:      return "PostDecrement";
        case NK_YIELD:               return "Yield";
        case NK_MATCH:               return "Match";
        case NK_MATCH_CASE:          return "MatchCase";
        case NK_WITH_STMT:           return "WithStmt";
        case NK_FUNCTION_DECL:       return "FunctionDecl";
        case NK_FOREIGN_IMPORT:      return "ForeignImport";
        case NK_SYMBOL_DECL:         return "SymbolDecl";
        case NK_ALIAS_DECL:          return "AliasDecl";
        case NK_TYPE_DECL:           return "TypeDecl";
        case NK_OBJECT_DECL:         return "ObjectDecl";
        case NK_CONST_DECL:          return "ConstDecl";
        case NK_MACRO_DECL:          return "MacroDecl";
        case NK_ASM_BLOCK:           return "AsmBlock";
        case NK_PARAM:               return "Param";
        case NK_TUPLE_RETURN:        return "TupleReturn";
        case NK_TUPLE_DESTRUCTURE:   return "TupleDestructure";
        case NK_TYPE_NAME:           return "TypeName";
        case NK_ENUM_DECL:           return "EnumDecl";
        case NK_DR_DIRECTIVE:        return "DrDirective";
        case NK_NORM_DIRECTIVE:      return "NormDirective";
        case NK_PROGRAM:             return "Program";
        default:                     return "<unknown>";
    }
}

/* =========================================================================
 * Internal helpers
 * ========================================================================= */

#define INDENT_UNIT 2
#define MAX_DEPTH   256

static void indent(int depth) {
    for (int i = 0; i < depth * INDENT_UNIT; i++) putchar(' ');
}

static void print_str(const UL_Arena *a, StrId id, const char *label) {
    if (id == UL_NULL_STR) {
        printf("%s=(null)", label);
    } else {
        printf("%s=\"%s\"", label, ul_str_get(a, id));
    }
}

/* Forward declaration for recursive calls */
static void print_node(const UL_Arena *a, NodeId id, int depth);

static void print_children(const UL_Arena *a, UL_Children ch,
                            const char *label, int depth)
{
    indent(depth);
    printf("%s[%u]:\n", label, (unsigned)ch.count);
    for (uint16_t i = 0; i < ch.count; i++) {
        print_node(a, a->child_pool[ch.start + i], depth + 1);
    }
}

/* =========================================================================
 * print_node — recursive tree printer
 * ========================================================================= */

static void print_node(const UL_Arena *a, NodeId id, int depth) {
    if (depth > MAX_DEPTH) {
        indent(depth);
        printf("... (max depth)\n");
        return;
    }

    if (id == UL_NULL_NODE) {
        indent(depth);
        printf("(null)\n");
        return;
    }

    if (id >= a->node_count) {
        indent(depth);
        printf("(invalid NodeId %u)\n", id);
        return;
    }

    const UL_Node *n = &a->nodes[id];
    const UL_Span *sp = &a->spans[id];

    indent(depth);

    /* Print node header */
    printf("[#%u] %s", id, ul_kind_name(n->kind));

    /* Print source span if available */
    if (sp->line > 0) {
        printf("  @%u:%u", sp->line, (unsigned)sp->col);
    }

    /* Print flags byte if non-zero */
    if (*(uint8_t *)&n->flags) {
        printf("  flags=0x%02x", *(uint8_t *)&n->flags);
    }

    printf("\n");

    /* Print kind-specific payload */
    switch (n->kind) {

        case NK_INT_LITERAL:
            indent(depth + 1);
            printf("value=%lld\n", (long long)ul_int_lit_get(n));
            break;

        case NK_FLOAT_LITERAL:
            indent(depth + 1);
            printf("value=%g\n", ul_float_lit_get(n));
            break;

        case NK_BOOL_LITERAL:
            indent(depth + 1);
            printf("value=%s\n", n->u.bool_lit.value ? "true" : "false");
            break;

        case NK_STRING_LITERAL:
            indent(depth + 1);
            print_str(a, n->u.str_node.str, "value");
            printf("\n");
            break;

        case NK_EMPTY_LITERAL:
            /* no payload */
            break;

        case NK_IDENTIFIER:
            indent(depth + 1);
            print_str(a, n->u.str_node.str, "name");
            printf("\n");
            break;

        case NK_BINARY_OP:
            indent(depth + 1);
            print_str(a, n->u.binary_op.op, "op");
            printf("\n");
            print_node(a, n->u.binary_op.left,  depth + 1);
            print_node(a, n->u.binary_op.right, depth + 1);
            break;

        case NK_UNARY_OP:
            indent(depth + 1);
            print_str(a, n->u.unary_op.op, "op");
            printf("\n");
            print_node(a, n->u.unary_op.operand, depth + 1);
            break;

        case NK_CAST:
            print_node(a, n->u.cast.target_type, depth + 1);
            print_node(a, n->u.cast.expr,        depth + 1);
            break;

        case NK_CALL:
            indent(depth + 1);
            print_str(a, n->u.call.name, "name");
            printf("\n");
            print_children(a, n->u.call.args, "args", depth + 1);
            break;

        case NK_METHOD_CALL:
            indent(depth + 1);
            print_str(a, n->u.method_call.method, "method");
            printf("\n");
            indent(depth + 1);
            printf("target:\n");
            print_node(a, n->u.method_call.target, depth + 2);
            print_children(a, n->u.method_call.args, "args", depth + 1);
            break;

        case NK_INDEX:
            indent(depth + 1);
            printf("target:\n");
            print_node(a, n->u.index_expr.target, depth + 2);
            indent(depth + 1);
            printf("index:\n");
            print_node(a, n->u.index_expr.index, depth + 2);
            break;

        case NK_SLICE_EXPR:
            indent(depth + 1);
            printf("target:\n");
            print_node(a, n->u.slice_expr.target, depth + 2);
            indent(depth + 1);
            printf("start:\n");
            print_node(a, n->u.slice_expr.start, depth + 2);
            indent(depth + 1);
            printf("end:\n");
            print_node(a, n->u.slice_expr.end, depth + 2);
            break;

        case NK_ARRAY_LITERAL:
            print_children(a, n->u.array_lit.elements, "elements", depth + 1);
            break;

        case NK_ARRAY_COMPREHENSION:
            indent(depth + 1);
            print_str(a, n->u.array_comp.var, "var");
            printf("\n");
            indent(depth + 1);
            printf("expr:\n");
            print_node(a, n->u.array_comp.expr, depth + 2);
            indent(depth + 1);
            printf("iterable:\n");
            print_node(a, n->u.array_comp.iterable, depth + 2);
            break;

        case NK_FIELD_ACCESS:
            indent(depth + 1);
            print_str(a, n->u.field_access.field, "field");
            printf("\n");
            print_node(a, n->u.field_access.target, depth + 1);
            break;

        case NK_OK_RESULT:
        case NK_ERROR_RESULT:
        case NK_RESULT_PROPAGATION:
        case NK_EXPR_STMT:
        case NK_PRINT:
        case NK_POST_INCREMENT:
        case NK_POST_DECREMENT:
        case NK_YIELD:
            print_node(a, n->u.single_child.value, depth + 1);
            break;

        case NK_RETURN:
            if (n->u.single_child.value != UL_NULL_NODE) {
                print_node(a, n->u.single_child.value, depth + 1);
            } else {
                indent(depth + 1);
                printf("(void return)\n");
            }
            break;

        case NK_TUPLE_LITERAL:
            print_children(a, n->u.tuple_lit.elements, "elements", depth + 1);
            break;

        case NK_FUNC_PTR_VALUE:
            indent(depth + 1);
            print_str(a, n->u.func_ptr.function, "function");
            printf("\n");
            print_children(a, n->u.func_ptr.params, "params", depth + 1);
            break;

        case NK_STRUCT_LITERAL:
            indent(depth + 1);
            print_str(a, n->u.struct_lit.type_name, "type_name");
            printf("\n");
            print_children(a, n->u.struct_lit.fields, "fields", depth + 1);
            break;

        case NK_VAR_DECL:
            indent(depth + 1);
            print_str(a, n->u.var_decl.name, "name");
            printf("  fixed=%s\n", n->flags.NF_FIXED ? "true" : "false");
            indent(depth + 1);
            printf("type:\n");
            print_node(a, n->u.var_decl.type_node, depth + 2);
            if (n->u.var_decl.init != UL_NULL_NODE) {
                indent(depth + 1);
                printf("init:\n");
                print_node(a, n->u.var_decl.init, depth + 2);
            }
            break;

        case NK_ASSIGN:
            indent(depth + 1);
            print_str(a, n->u.assign.op, "op");
            printf("\n");
            indent(depth + 1);
            printf("target:\n");
            print_node(a, n->u.assign.target, depth + 2);
            indent(depth + 1);
            printf("value:\n");
            print_node(a, n->u.assign.value, depth + 2);
            break;

        case NK_IF:
            indent(depth + 1);
            printf("condition:\n");
            print_node(a, n->u.if_stmt.condition, depth + 2);
            print_children(a, n->u.if_stmt.then_body, "then", depth + 1);
            if (n->u.if_stmt.else_body.count > 0) {
                print_children(a, n->u.if_stmt.else_body, "else", depth + 1);
            }
            break;

        case NK_WHILE:
        case NK_DO_WHILE:
            indent(depth + 1);
            printf("condition:\n");
            print_node(a, n->u.while_stmt.condition, depth + 2);
            print_children(a, n->u.while_stmt.body, "body", depth + 1);
            break;

        case NK_FOR:
            indent(depth + 1);
            print_str(a, n->u.for_stmt.var, "var");
            printf("  parallel=%s\n",
                   n->flags.NF_PARALLEL ? "true" : "false");
            indent(depth + 1);
            printf("iterable:\n");
            print_node(a, n->u.for_stmt.iterable, depth + 2);
            print_children(a, n->u.for_stmt.body, "body", depth + 1);
            break;

        case NK_C_FOR:
            indent(depth + 1);
            printf("init:\n");
            print_node(a, n->u.cfor_stmt.init, depth + 2);
            indent(depth + 1);
            printf("condition:\n");
            print_node(a, n->u.cfor_stmt.condition, depth + 2);
            indent(depth + 1);
            printf("update:\n");
            print_node(a, n->u.cfor_stmt.update, depth + 2);
            print_children(a, n->u.cfor_stmt.body, "body", depth + 1);
            break;

        case NK_ESCAPE:
        case NK_CONTINUE:
            /* no payload */
            break;

        case NK_PORTAL_DECL:
        case NK_GOTO_STMT:
            indent(depth + 1);
            print_str(a, n->u.label_node.name, "name");
            printf("\n");
            break;

        case NK_KILLSWITCH:
            indent(depth + 1);
            printf("condition:\n");
            print_node(a, n->u.killswitch.condition, depth + 2);
            break;

        case NK_TRY_STMT:
            print_children(a, n->u.try_stmt.body,         "body",     depth + 1);
            print_children(a, n->u.try_stmt.handlers,     "handlers", depth + 1);
            if (n->u.try_stmt.finally_body.count > 0) {
                print_children(a, n->u.try_stmt.finally_body, "finally", depth + 1);
            }
            break;

        case NK_CATCH_CLAUSE:
            indent(depth + 1);
            print_str(a, n->u.catch_clause.exception_type, "exception_type");
            printf("  ");
            print_str(a, n->u.catch_clause.binding_name,   "binding_name");
            printf("\n");
            print_children(a, n->u.catch_clause.body, "body", depth + 1);
            break;

        case NK_THROW_STMT:
            indent(depth + 1);
            print_str(a, n->u.throw_stmt.exception_type, "exception_type");
            printf("\n");
            print_node(a, n->u.throw_stmt.message, depth + 1);
            break;

        case NK_MATCH:
            indent(depth + 1);
            printf("subject:\n");
            print_node(a, n->u.match_stmt.subject, depth + 2);
            print_children(a, n->u.match_stmt.cases, "cases", depth + 1);
            break;

        case NK_MATCH_CASE:
            indent(depth + 1);
            if (n->u.match_case.value == UL_NULL_NODE) {
                printf("value=default\n");
            } else {
                printf("value:\n");
                print_node(a, n->u.match_case.value, depth + 2);
            }
            print_children(a, n->u.match_case.body, "body", depth + 1);
            break;

        case NK_WITH_STMT:
            indent(depth + 1);
            print_str(a, n->u.with_stmt.binding, "binding");
            printf("\n");
            indent(depth + 1);
            printf("expr:\n");
            print_node(a, n->u.with_stmt.expr, depth + 2);
            print_children(a, n->u.with_stmt.body, "body", depth + 1);
            break;

        case NK_TYPE_NAME: {
            const char *qual = "";
            if (n->flags.NF_POINTER)  qual = "ptr";
            else if (n->flags.NF_IS_LIST) qual = "list";
            else if (n->flags.NF_IS_ARRAY) qual = "array";
            indent(depth + 1);
            print_str(a, n->u.type_name.name, "name");
            printf("  qual=%s  array_size=%u\n",
                   *qual ? qual : "none",
                   n->u.type_name.array_size);
            break;
        }

        case NK_PARAM:
            indent(depth + 1);
            print_str(a, n->u.param.name, "name");
            printf("\n");
            print_node(a, n->u.param.type_node, depth + 1);
            if (n->u.param.default_ != UL_NULL_NODE) {
                indent(depth + 1);
                printf("default:\n");
                print_node(a, n->u.param.default_, depth + 2);
            }
            break;

        case NK_FUNCTION_DECL: {
            indent(depth + 1);
            print_str(a, n->u.func_decl.name, "name");
            printf("  generator=%s  result=%s  imported=%s  variadic=%s\n",
                   n->flags.NF_IS_GENERATOR   ? "true" : "false",
                   n->flags.NF_RETURNS_RESULT  ? "true" : "false",
                   n->flags.NF_IS_IMPORTED     ? "true" : "false",
                   n->flags.NF_VARIADIC        ? "true" : "false");
            if (n->u.func_decl.return_type != UL_NULL_NODE) {
                indent(depth + 1);
                printf("return_type:\n");
                print_node(a, n->u.func_decl.return_type, depth + 2);
            }
            print_children(a, n->u.func_decl.params, "params", depth + 1);
            print_children(a, n->u.func_decl.body,   "body",   depth + 1);
            if (n->aux > 0) {
                /* nested_count stored in node.aux; nested_start in payload */
                UL_Children nested;
                nested.start = n->u.func_decl.nested_start;
                nested.count = n->aux;
                nested._pad  = 0;
                print_children(a, nested, "nested", depth + 1);
            }
            break;
        }

        case NK_FOREIGN_IMPORT:
            indent(depth + 1);
            print_str(a, n->u.foreign_import.lib,  "lib");
            printf("  ");
            print_str(a, n->u.foreign_import.name, "name");
            printf("  variadic=%s\n",
                   n->flags.NF_VARIADIC ? "true" : "false");
            if (n->u.foreign_import.return_type != UL_NULL_NODE) {
                indent(depth + 1);
                printf("return_type:\n");
                print_node(a, n->u.foreign_import.return_type, depth + 2);
            }
            print_children(a, n->u.foreign_import.params, "params", depth + 1);
            break;

        case NK_SYMBOL_DECL:
            indent(depth + 1);
            print_str(a, n->u.symbol_decl.operator_, "operator");
            printf("  ");
            print_str(a, n->u.symbol_decl.function,  "function");
            printf("\n");
            break;

        case NK_ALIAS_DECL:
            indent(depth + 1);
            print_str(a, n->u.alias_decl.name, "name");
            printf("\n");
            print_node(a, n->u.alias_decl.target_type, depth + 1);
            break;

        case NK_TYPE_DECL:
            indent(depth + 1);
            print_str(a, n->u.type_decl.name, "name");
            printf("  ");
            print_str(a, n->u.type_decl.parent, "parent");
            printf("  packed=%s\n", n->flags.NF_PACKED ? "true" : "false");
            print_children(a, n->u.type_decl.fields, "fields", depth + 1);
            break;

        case NK_OBJECT_DECL:
            indent(depth + 1);
            print_str(a, n->u.object_decl.name, "name");
            printf("  ");
            print_str(a, n->u.object_decl.parent, "parent");
            printf("\n");
            print_children(a, n->u.object_decl.fields,  "fields",  depth + 1);
            print_children(a, n->u.object_decl.methods, "methods", depth + 1);
            break;

        case NK_CONST_DECL:
            indent(depth + 1);
            print_str(a, n->u.const_decl.name, "name");
            printf("\n");
            print_node(a, n->u.const_decl.type_node, depth + 1);
            print_node(a, n->u.const_decl.value,     depth + 1);
            break;

        case NK_MACRO_DECL:
            indent(depth + 1);
            print_str(a, n->u.macro_decl.name, "name");
            printf("\n");
            print_children(a, n->u.macro_decl.params, "params", depth + 1);
            indent(depth + 1);
            printf("expr:\n");
            print_node(a, n->u.macro_decl.expr, depth + 2);
            break;

        case NK_ASM_BLOCK:
            indent(depth + 1);
            print_str(a, n->u.asm_block.arch, "arch");
            printf("\n");
            indent(depth + 1);
            print_str(a, n->u.asm_block.body, "body");
            printf("\n");
            break;

        case NK_ENUM_DECL:
            indent(depth + 1);
            print_str(a, n->u.enum_decl.name, "name");
            printf("\n");
            print_children(a, n->u.enum_decl.members, "members", depth + 1);
            break;

        case NK_TUPLE_RETURN:
            print_children(a, n->u.tuple_return.types, "types", depth + 1);
            break;

        case NK_TUPLE_DESTRUCTURE:
            indent(depth + 1);
            printf("value:\n");
            print_node(a, n->u.tuple_destr.value, depth + 2);
            print_children(a, n->u.tuple_destr.targets, "targets", depth + 1);
            break;

        case NK_DR_DIRECTIVE:
            indent(depth + 1);
            print_str(a, n->u.dr_directive.key,   "key");
            printf("  ");
            print_str(a, n->u.dr_directive.value, "value");
            printf("\n");
            break;

        case NK_NORM_DIRECTIVE:
            indent(depth + 1);
            printf("level=%u\n", n->u.norm_directive.level);
            break;

        case NK_PROGRAM:
            print_children(a, n->u.program.decls, "decls", depth + 1);
            break;

        default:
            indent(depth + 1);
            printf("(no printer for kind %d)\n", (int)n->kind);
            break;
    }
}

/* =========================================================================
 * Public API
 * ========================================================================= */

/*
 * ul_debug_print_node — print a node and its entire subtree.
 *
 * depth: initial indentation level (pass 0 for top-level call).
 */
void ul_debug_print_node(const UL_Arena *a, NodeId id, int depth) {
    print_node(a, id, depth);
}

/*
 * ul_debug_print_tree — print from the root (program node) downward.
 */
void ul_debug_print_tree(const UL_Arena *a, NodeId root) {
    printf("=== UniLogic AST (%u nodes) ===\n", a->node_count - 1);
    print_node(a, root, 0);
    printf("=== end ===\n");
}

/*
 * ul_debug_dump_strings — print all interned strings with their StrIds.
 */
void ul_debug_dump_strings(const UL_Arena *a) {
    printf("=== Interned strings (%u) ===\n", a->str_table_count);
    uint32_t printed = 0;
    for (uint32_t i = 0; i < a->str_table_cap; i++) {
        const UL_StrEntry *e = &a->str_table[i];
        if (e->length == 0 && e->hash == 0) continue;
        printf("  StrId=%-6u len=%-4u hash=0x%08x  \"%s\"\n",
               e->offset, e->length, e->hash,
               a->str_buf + e->offset);
        printed++;
        if (printed >= a->str_table_count) break;
    }
    printf("=== end strings ===\n");
}
