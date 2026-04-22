/*
 * ast_nodes.h — UniLogic XPile C AST node definitions
 *
 * Flat arena-indexed AST. Every node is a fixed-size tagged union (32 bytes).
 * No heap pointers inside nodes. Cross-node references use NodeId (uint32_t).
 * String data lives in the arena string buffer, referenced by StrId (uint32_t).
 * Variable-length child lists live in the arena child pool, referenced by
 * UL_Children {start, count}.
 *
 * Source locations (line/col) are NOT stored in nodes — they live in the
 * parallel spans[] array (hot/cold split).
 *
 * See: AST_Optimization_Spec.md for full design rationale.
 */

#ifndef UL_AST_NODES_H
#define UL_AST_NODES_H

#include <stdint.h>
#include <stdbool.h>

/* ── Forward declaration ─────────────────────────────────────────────────── */
typedef struct UL_Node UL_Node;

/* =========================================================================
 * Identity types
 * ========================================================================= */

/* Index into UL_Arena.nodes[]. 0 is reserved as the null node. */
typedef uint32_t NodeId;

/* Byte offset into UL_Arena.str_buf. UINT32_MAX is the null/absent sentinel. */
typedef uint32_t StrId;

/* Index into UL_Arena.child_pool[]. */
typedef uint32_t ChildIdx;

#define UL_NULL_NODE  ((NodeId)0)
#define UL_NULL_STR   ((StrId)UINT32_MAX)

/* =========================================================================
 * Source span  (stored in the PARALLEL spans[] array, NOT inside UL_Node)
 * ========================================================================= */

typedef struct {
    uint32_t line;     /* 1-based source line */
    uint16_t col;      /* 1-based column */
    uint16_t end_col;  /* exclusive end column (0 if unknown) */
} UL_Span; /* 8 bytes */

/* =========================================================================
 * Node kind discriminant
 * ========================================================================= */

typedef uint8_t UL_NodeKind;

/* Literals */
#define NK_INT_LITERAL         ((UL_NodeKind)1)
#define NK_FLOAT_LITERAL       ((UL_NodeKind)2)
#define NK_STRING_LITERAL      ((UL_NodeKind)3)
#define NK_BOOL_LITERAL        ((UL_NodeKind)4)
#define NK_EMPTY_LITERAL       ((UL_NodeKind)5)

/* Expressions */
#define NK_IDENTIFIER          ((UL_NodeKind)6)
#define NK_BINARY_OP           ((UL_NodeKind)7)
#define NK_UNARY_OP            ((UL_NodeKind)8)
#define NK_CAST                ((UL_NodeKind)9)
#define NK_CALL                ((UL_NodeKind)10)
#define NK_METHOD_CALL         ((UL_NodeKind)11)
#define NK_INDEX               ((UL_NodeKind)12)
#define NK_SLICE_EXPR          ((UL_NodeKind)13)
#define NK_ARRAY_LITERAL       ((UL_NodeKind)14)
#define NK_ARRAY_COMPREHENSION ((UL_NodeKind)15)
#define NK_FIELD_ACCESS        ((UL_NodeKind)16)
#define NK_OK_RESULT           ((UL_NodeKind)17)
#define NK_ERROR_RESULT        ((UL_NodeKind)18)
#define NK_RESULT_PROPAGATION  ((UL_NodeKind)19)
#define NK_TUPLE_LITERAL       ((UL_NodeKind)20)
#define NK_FUNC_PTR_VALUE      ((UL_NodeKind)21)
#define NK_STRUCT_LITERAL      ((UL_NodeKind)22)

/* Statements */
#define NK_VAR_DECL            ((UL_NodeKind)23)
#define NK_ASSIGN              ((UL_NodeKind)24)
#define NK_IF                  ((UL_NodeKind)25)
#define NK_WHILE               ((UL_NodeKind)26)
#define NK_DO_WHILE            ((UL_NodeKind)27)
#define NK_FOR                 ((UL_NodeKind)28)
#define NK_C_FOR               ((UL_NodeKind)29)
#define NK_RETURN              ((UL_NodeKind)30)
#define NK_PRINT               ((UL_NodeKind)31)
#define NK_EXPR_STMT           ((UL_NodeKind)32)
#define NK_ESCAPE              ((UL_NodeKind)33)
#define NK_CONTINUE            ((UL_NodeKind)34)
#define NK_PORTAL_DECL         ((UL_NodeKind)35)
#define NK_GOTO_STMT           ((UL_NodeKind)36)
#define NK_KILLSWITCH          ((UL_NodeKind)37)
#define NK_TRY_STMT            ((UL_NodeKind)38)
#define NK_CATCH_CLAUSE        ((UL_NodeKind)39)
#define NK_THROW_STMT          ((UL_NodeKind)40)
#define NK_POST_INCREMENT      ((UL_NodeKind)41)
#define NK_POST_DECREMENT      ((UL_NodeKind)42)
#define NK_YIELD               ((UL_NodeKind)43)
#define NK_MATCH               ((UL_NodeKind)44)
#define NK_MATCH_CASE          ((UL_NodeKind)45)
#define NK_WITH_STMT           ((UL_NodeKind)46)

/* Top-level declarations */
#define NK_FUNCTION_DECL       ((UL_NodeKind)47)
#define NK_FOREIGN_IMPORT      ((UL_NodeKind)48)
#define NK_SYMBOL_DECL         ((UL_NodeKind)49)
#define NK_ALIAS_DECL          ((UL_NodeKind)50)
#define NK_TYPE_DECL           ((UL_NodeKind)51)
#define NK_OBJECT_DECL         ((UL_NodeKind)52)
#define NK_CONST_DECL          ((UL_NodeKind)53)
#define NK_MACRO_DECL          ((UL_NodeKind)54)
#define NK_ASM_BLOCK           ((UL_NodeKind)55)
#define NK_PARAM               ((UL_NodeKind)56)
#define NK_TUPLE_RETURN        ((UL_NodeKind)57)
#define NK_TUPLE_DESTRUCTURE   ((UL_NodeKind)58)
#define NK_TYPE_NAME           ((UL_NodeKind)59)
#define NK_ENUM_DECL           ((UL_NodeKind)60)
#define NK_DR_DIRECTIVE        ((UL_NodeKind)61)
#define NK_NORM_DIRECTIVE      ((UL_NodeKind)62)
#define NK_PROGRAM             ((UL_NodeKind)63)

#define NK_KIND_COUNT          ((UL_NodeKind)64) /* sentinel — update when adding kinds */

/* Human-readable name for a node kind (defined in ast_debug.c) */
const char *ul_kind_name(UL_NodeKind kind);

/* =========================================================================
 * Node flags byte
 *
 * One uint8_t of boolean flags shared across all node kinds.
 * Which flag means what depends on the node's kind field.
 * Friendly #defines below document intent at each use site.
 * ========================================================================= */

typedef struct {
    uint8_t flag_a : 1; /* VarDecl/ConstDecl: fixed (const)        */
                        /* FunctionDecl:      is_generator          */
                        /* For:               parallel              */
                        /* ForeignImport:     variadic              */
    uint8_t flag_b : 1; /* FunctionDecl:      returns_result        */
                        /* TypeName:          pointer               */
                        /* TypeName:          is_array (alt slot)   */
    uint8_t flag_c : 1; /* FunctionDecl:      is_imported           */
                        /* TypeName:          is_list               */
    uint8_t flag_d : 1; /* TypeDecl:          packed                */
                        /* FunctionDecl:      variadic              */
    uint8_t flag_e : 1; /* reserved */
    uint8_t flag_f : 1; /* reserved */
    uint8_t flag_g : 1; /* reserved */
    uint8_t flag_h : 1; /* reserved */
} UL_NodeFlags; /* 1 byte */

/* Friendly aliases — document intent at each use site */
#define NF_FIXED          flag_a   /* VarDecl, ConstDecl */
#define NF_IS_GENERATOR   flag_a   /* FunctionDecl */
#define NF_PARALLEL       flag_a   /* For */
#define NF_RETURNS_RESULT flag_b   /* FunctionDecl */
#define NF_POINTER        flag_b   /* TypeName */
#define NF_IS_ARRAY       flag_b   /* TypeName (when flag_c==0) */
#define NF_IS_IMPORTED    flag_c   /* FunctionDecl */
#define NF_IS_LIST        flag_c   /* TypeName */
#define NF_PACKED         flag_d   /* TypeDecl */
#define NF_VARIADIC       flag_d   /* FunctionDecl, ForeignImport */

/* =========================================================================
 * Child list reference
 *
 * An inline (start, count) pair into arena->child_pool[].
 * 8 bytes — fits in the payload union without eating extra space.
 * ========================================================================= */

typedef struct {
    ChildIdx start;  /* uint32_t: index into arena->child_pool */
    uint16_t count;  /* max 65535 children — sufficient for any list */
    uint16_t _pad;
} UL_Children; /* 8 bytes */

/* =========================================================================
 * The unified AST node — 32 bytes, cache-line friendly
 *
 * Layout:
 *   [0]      kind    (1 byte)
 *   [1]      flags   (1 byte)
 *   [2..3]   _pad    (2 bytes — future use / alignment)
 *   [4..31]  union u (28 bytes payload)
 *
 * Source location is NOT here — it is in arena->spans[node_id].
 * ========================================================================= */

struct UL_Node {
    UL_NodeKind  kind;         /* 1 byte — which union member is active */
    UL_NodeFlags flags;        /* 1 byte — boolean flags (kind-dependent) */
    uint16_t     aux;          /* 2 bytes — kind-specific auxiliary data:
                                  NK_FUNCTION_DECL: nested function count
                                  (all other kinds: 0 / reserved)         */

    union {

        /* ── NK_INT_LITERAL ─────────────────────────────────────────── */
        /* Python: IntLiteral.value
         *
         * int64_t is split into lo+hi uint32_t to avoid 8-byte alignment
         * forcing the union (and thus UL_Node) above 32 bytes.
         * Use the UL_INT_LIT_GET / UL_INT_LIT_SET macros for access.
         *
         * Byte order: lo = bits 0-31, hi = bits 32-63 (host endian).
         */
        struct {
            uint32_t lo;            /* bits  0–31 of the int64 value */
            uint32_t hi;            /* bits 32–63 of the int64 value */
            uint8_t  _p[20];
        } int_lit; /* 28 bytes */

        /* ── NK_FLOAT_LITERAL ───────────────────────────────────────── */
        /* Python: FloatLiteral.value
         *
         * double split into lo+hi uint32_t for the same alignment reason.
         * Use the UL_FLOAT_LIT_GET / UL_FLOAT_LIT_SET macros for access.
         */
        struct {
            uint32_t lo;            /* bits  0–31 of the double value */
            uint32_t hi;            /* bits 32–63 of the double value */
            uint8_t  _p[20];
        } float_lit; /* 28 bytes */

        /* ── NK_BOOL_LITERAL ────────────────────────────────────────── */
        /* Python: BoolLiteral.value */
        struct {
            uint8_t value;          /* 0 or 1 */
            uint8_t _p[27];
        } bool_lit; /* 28 bytes */

        /* ── NK_STRING_LITERAL, NK_IDENTIFIER ───────────────────────── */
        /* Python: StringLiteral.value, Identifier.name */
        struct {
            StrId   str;            /* 4 bytes — interned string */
            uint8_t _p[24];
        } str_node; /* 28 bytes */

        /* ── NK_EMPTY_LITERAL ───────────────────────────────────────── */
        /* Python: EmptyLiteral — no payload (null/None sentinel) */
        /* (use _raw, all zero) */

        /* ── NK_BINARY_OP ───────────────────────────────────────────── */
        /* Python: BinaryOp.op, .left, .right */
        struct {
            StrId    op;            /* 4 — interned operator string: "+", "==", "and" */
            NodeId   left;          /* 4 */
            NodeId   right;         /* 4 */
            uint8_t  _p[16];
        } binary_op; /* 28 bytes */

        /* ── NK_UNARY_OP ────────────────────────────────────────────── */
        /* Python: UnaryOp.op, .operand */
        struct {
            StrId    op;            /* 4 — "not", "-", "address", "deref" */
            NodeId   operand;       /* 4 */
            uint8_t  _p[20];
        } unary_op; /* 28 bytes */

        /* ── NK_CAST ────────────────────────────────────────────────── */
        /* Python: Cast.expr, .target_type */
        struct {
            NodeId   expr;          /* 4 */
            NodeId   target_type;   /* 4 — NodeId of NK_TYPE_NAME */
            uint8_t  _p[20];
        } cast; /* 28 bytes */

        /* ── NK_CALL ────────────────────────────────────────────────── */
        /* Python: Call.name, .args */
        struct {
            StrId        name;      /* 4 — interned function name */
            UL_Children  args;      /* 8 — children: argument expressions */
            uint8_t      _p[16];
        } call; /* 28 bytes */

        /* ── NK_METHOD_CALL ─────────────────────────────────────────── */
        /* Python: MethodCall.target, .method, .args */
        struct {
            NodeId       target;    /* 4 — object expression */
            StrId        method;    /* 4 — interned method name */
            UL_Children  args;      /* 8 — argument expressions */
            uint8_t      _p[12];
        } method_call; /* 28 bytes */

        /* ── NK_INDEX ───────────────────────────────────────────────── */
        /* Python: Index.target, .index */
        struct {
            NodeId   target;        /* 4 */
            NodeId   index;         /* 4 */
            uint8_t  _p[20];
        } index_expr; /* 28 bytes */

        /* ── NK_SLICE_EXPR ──────────────────────────────────────────── */
        /* Python: SliceExpr.target, .start, .end */
        struct {
            NodeId   target;        /* 4 */
            NodeId   start;         /* 4 */
            NodeId   end;           /* 4 */
            uint8_t  _p[16];
        } slice_expr; /* 28 bytes */

        /* ── NK_ARRAY_LITERAL ───────────────────────────────────────── */
        /* Python: ArrayLiteral.elements */
        struct {
            UL_Children elements;   /* 8 */
            uint8_t     _p[20];
        } array_lit; /* 28 bytes */

        /* ── NK_ARRAY_COMPREHENSION ─────────────────────────────────── */
        /* Python: ArrayComprehension.expr, .var, .iterable */
        struct {
            NodeId   expr;          /* 4 — expression to evaluate */
            StrId    var;           /* 4 — loop variable name (interned) */
            NodeId   iterable;      /* 4 — iterable expression */
            uint8_t  _p[16];
        } array_comp; /* 28 bytes */

        /* ── NK_FIELD_ACCESS ────────────────────────────────────────── */
        /* Python: FieldAccess.target, .field */
        struct {
            NodeId   target;        /* 4 */
            StrId    field;         /* 4 — interned field name */
            uint8_t  _p[20];
        } field_access; /* 28 bytes */

        /* ── NK_OK_RESULT, NK_ERROR_RESULT, NK_RESULT_PROPAGATION,
               NK_EXPR_STMT, NK_RETURN, NK_PRINT,
               NK_POST_INCREMENT, NK_POST_DECREMENT, NK_YIELD ──────── */
        /* Python: OkResult.value, ErrorResult.value, etc.
           UL_NULL_NODE when absent (e.g. bare return) */
        struct {
            NodeId   value;         /* 4 — UL_NULL_NODE if absent */
            uint8_t  _p[24];
        } single_child; /* 28 bytes */

        /* ── NK_TUPLE_LITERAL ───────────────────────────────────────── */
        /* Python: TupleLiteral.elements */
        struct {
            UL_Children elements;   /* 8 */
            uint8_t     _p[20];
        } tuple_lit; /* 28 bytes */

        /* ── NK_FUNC_PTR_VALUE ──────────────────────────────────────── */
        /* Python: FuncPtrValue.function, .params */
        struct {
            StrId        function;  /* 4 — interned function name */
            UL_Children  params;    /* 8 — NK_PARAM children */
            uint8_t      _p[16];
        } func_ptr; /* 28 bytes */

        /* ── NK_STRUCT_LITERAL ──────────────────────────────────────── */
        /* Python: StructLiteral.type_name, .fields
           fields child list: alternating [NK_IDENTIFIER(name), value_expr, ...] */
        struct {
            StrId        type_name; /* 4 — interned struct type name */
            UL_Children  fields;    /* 8 — alternating name/value pairs */
            uint8_t      _p[16];
        } struct_lit; /* 28 bytes */

        /* ── NK_VAR_DECL ────────────────────────────────────────────── */
        /* Python: VarDecl.type_, .name, .init, .fixed
           flags.NF_FIXED = VarDecl.fixed */
        struct {
            NodeId   type_node;     /* 4 — NodeId of NK_TYPE_NAME */
            StrId    name;          /* 4 — interned variable name */
            NodeId   init;          /* 4 — UL_NULL_NODE if no initializer */
            uint8_t  _p[16];
        } var_decl; /* 28 bytes */

        /* ── NK_ASSIGN ──────────────────────────────────────────────── */
        /* Python: Assign.target, .op, .value */
        struct {
            NodeId   target;        /* 4 */
            StrId    op;            /* 4 — "=", "+=", "-=", etc. */
            NodeId   value;         /* 4 */
            uint8_t  _p[16];
        } assign; /* 28 bytes */

        /* ── NK_IF ──────────────────────────────────────────────────── */
        /* Python: If.condition, .then_body, .else_body */
        struct {
            NodeId      condition;  /* 4 */
            UL_Children then_body;  /* 8 */
            UL_Children else_body;  /* 8 — count=0 if no else branch */
            uint8_t     _p[8];
        } if_stmt; /* 28 bytes */

        /* ── NK_WHILE, NK_DO_WHILE ──────────────────────────────────── */
        /* Python: While.condition/.body, DoWhile.body/.condition
           For NK_DO_WHILE the condition is still checked after body. */
        struct {
            NodeId      condition;  /* 4 */
            UL_Children body;       /* 8 */
            uint8_t     _p[16];
        } while_stmt; /* 28 bytes */

        /* ── NK_FOR ─────────────────────────────────────────────────── */
        /* Python: For.var, .iterable, .body, .parallel
           flags.NF_PARALLEL = For.parallel */
        struct {
            StrId        var;       /* 4 — interned loop variable name */
            NodeId       iterable;  /* 4 */
            UL_Children  body;      /* 8 */
            uint8_t      _p[12];
        } for_stmt; /* 28 bytes */

        /* ── NK_C_FOR ───────────────────────────────────────────────── */
        /* Python: CFor.init, .condition, .update, .body */
        struct {
            NodeId      init;       /* 4 — VarDecl or Assign */
            NodeId      condition;  /* 4 */
            NodeId      update;     /* 4 — PostIncrement, Assign, etc. */
            UL_Children body;       /* 8 */
            uint8_t     _p[8];
        } cfor_stmt; /* 28 bytes */

        /* ── NK_PORTAL_DECL, NK_GOTO_STMT ──────────────────────────── */
        /* Python: PortalDecl.name, GotoStmt.target */
        struct {
            StrId    name;          /* 4 — interned label name */
            uint8_t  _p[24];
        } label_node; /* 28 bytes */

        /* ── NK_KILLSWITCH ──────────────────────────────────────────── */
        /* Python: KillswitchStmt.condition */
        struct {
            NodeId   condition;     /* 4 — abort if false */
            uint8_t  _p[24];
        } killswitch; /* 28 bytes */

        /* ── NK_TRY_STMT ────────────────────────────────────────────── */
        /* Python: TryStmt.body, .handlers, .finally_body */
        struct {
            UL_Children body;           /* 8 */
            UL_Children handlers;       /* 8 — children: NK_CATCH_CLAUSE nodes */
            UL_Children finally_body;   /* 8 — count=0 if absent */
            uint8_t     _p[4];
        } try_stmt; /* 28 bytes */

        /* ── NK_CATCH_CLAUSE ────────────────────────────────────────── */
        /* Python: CatchClause.exception_type, .binding_name, .body */
        struct {
            StrId        exception_type; /* 4 — UL_NULL_STR for catch-all */
            StrId        binding_name;   /* 4 — UL_NULL_STR if no "as name" */
            UL_Children  body;           /* 8 */
            uint8_t      _p[12];
        } catch_clause; /* 28 bytes */

        /* ── NK_THROW_STMT ──────────────────────────────────────────── */
        /* Python: ThrowStmt.exception_type, .message */
        struct {
            StrId    exception_type; /* 4 — interned type name */
            NodeId   message;        /* 4 — message expression */
            uint8_t  _p[20];
        } throw_stmt; /* 28 bytes */

        /* ── NK_MATCH ───────────────────────────────────────────────── */
        /* Python: Match.subject, .cases */
        struct {
            NodeId       subject;   /* 4 */
            UL_Children  cases;     /* 8 — children: NK_MATCH_CASE nodes */
            uint8_t      _p[16];
        } match_stmt; /* 28 bytes */

        /* ── NK_MATCH_CASE ──────────────────────────────────────────── */
        /* Python: MatchCase.value, .body */
        struct {
            NodeId       value;     /* 4 — UL_NULL_NODE for default case */
            UL_Children  body;      /* 8 */
            uint8_t      _p[16];
        } match_case; /* 28 bytes */

        /* ── NK_WITH_STMT ───────────────────────────────────────────── */
        /* Python: WithStmt.expr, .binding, .body */
        struct {
            NodeId       expr;      /* 4 — resource expression */
            StrId        binding;   /* 4 — interned variable name */
            UL_Children  body;      /* 8 */
            uint8_t      _p[12];
        } with_stmt; /* 28 bytes */

        /* ── NK_TYPE_NAME ───────────────────────────────────────────── */
        /* Python: TypeName.name, .pointer, .is_array, .array_size, .is_list
           flags.NF_POINTER  = TypeName.pointer
           flags.NF_IS_ARRAY = TypeName.is_array  (flag_b when flag_c==0)
           flags.NF_IS_LIST  = TypeName.is_list   (flag_c) */
        struct {
            StrId    name;          /* 4 — e.g. "int", "float", "bool" */
            uint32_t array_size;    /* 4 — 0 = inferred from initializer */
            uint8_t  _p[20];
        } type_name; /* 28 bytes */

        /* ── NK_PARAM ───────────────────────────────────────────────── */
        /* Python: Param.type_, .name, .default */
        struct {
            NodeId   type_node;     /* 4 — NodeId of NK_TYPE_NAME */
            StrId    name;          /* 4 — interned parameter name */
            NodeId   default_;      /* 4 — UL_NULL_NODE if no default */
            uint8_t  _p[16];
        } param; /* 28 bytes */

        /* ── NK_FUNCTION_DECL ───────────────────────────────────────── */
        /* Python: FunctionDecl.name, .params, .return_type, .body,
                   .is_generator, .returns_result, .is_imported, .variadic,
                   .nested_functions
           flags.NF_IS_GENERATOR  = is_generator
           flags.NF_RETURNS_RESULT= returns_result
           flags.NF_IS_IMPORTED   = is_imported
           flags.NF_VARIADIC      = variadic
           node.aux               = count of nested functions
           nested_start           = ChildIdx into child_pool for nested fns */
        struct {
            StrId        name;          /* 4 */
            NodeId       return_type;   /* 4 — UL_NULL_NODE for void/none */
            UL_Children  params;        /* 8 */
            UL_Children  body;          /* 8 */
            ChildIdx     nested_start;  /* 4 — child_pool index; count in node.aux */
        } func_decl; /* 28 bytes */

        /* ── NK_FOREIGN_IMPORT ──────────────────────────────────────── */
        /* Python: ForeignImport.lib, .name, .params, .return_type, .variadic
           flags.NF_VARIADIC = variadic */
        struct {
            StrId        lib;           /* 4 */
            StrId        name;          /* 4 */
            NodeId       return_type;   /* 4 — UL_NULL_NODE for void */
            UL_Children  params;        /* 8 */
            uint8_t      _p[8];
        } foreign_import; /* 28 bytes */

        /* ── NK_SYMBOL_DECL ─────────────────────────────────────────── */
        /* Python: SymbolDecl.operator, .function */
        struct {
            StrId    operator_;     /* 4 — interned operator string, e.g. "+" */
            StrId    function;      /* 4 — interned function name */
            uint8_t  _p[20];
        } symbol_decl; /* 28 bytes */

        /* ── NK_ALIAS_DECL ──────────────────────────────────────────── */
        /* Python: AliasDecl.name, .target_type */
        struct {
            StrId    name;          /* 4 — alias name, e.g. "UserId" */
            NodeId   target_type;   /* 4 — NodeId of NK_TYPE_NAME */
            uint8_t  _p[20];
        } alias_decl; /* 28 bytes */

        /* ── NK_TYPE_DECL ───────────────────────────────────────────── */
        /* Python: TypeDecl.name, .fields, .parent, .packed
           flags.NF_PACKED = packed */
        struct {
            StrId        name;      /* 4 */
            StrId        parent;    /* 4 — UL_NULL_STR if no parent */
            UL_Children  fields;    /* 8 — children: NK_PARAM nodes */
            uint8_t      _p[12];
        } type_decl; /* 28 bytes */

        /* ── NK_OBJECT_DECL ─────────────────────────────────────────── */
        /* Python: ObjectDecl.name, .fields, .methods, .parent */
        struct {
            StrId        name;      /* 4 */
            StrId        parent;    /* 4 — UL_NULL_STR if no parent */
            UL_Children  fields;    /* 8 — children: NK_PARAM nodes */
            UL_Children  methods;   /* 8 — children: NK_FUNCTION_DECL nodes */
        } object_decl; /* 28 bytes */

        /* ── NK_CONST_DECL ──────────────────────────────────────────── */
        /* Python: ConstDecl.type_, .name, .value */
        struct {
            NodeId   type_node;     /* 4 — NodeId of NK_TYPE_NAME */
            StrId    name;          /* 4 */
            NodeId   value;         /* 4 — must be a literal */
            uint8_t  _p[16];
        } const_decl; /* 28 bytes */

        /* ── NK_MACRO_DECL ──────────────────────────────────────────── */
        /* Python: MacroDecl.name, .params, .expr
           params children: NK_IDENTIFIER nodes (macro parameter names) */
        struct {
            StrId        name;      /* 4 */
            NodeId       expr;      /* 4 — body expression */
            UL_Children  params;    /* 8 — children: NK_IDENTIFIER nodes */
            uint8_t      _p[12];
        } macro_decl; /* 28 bytes */

        /* ── NK_ASM_BLOCK ───────────────────────────────────────────── */
        /* Python: AsmBlock.arch, .body */
        struct {
            StrId    arch;          /* 4 — e.g. "x86_64" */
            StrId    body;          /* 4 — raw assembly text, interned */
            uint8_t  _p[20];
        } asm_block; /* 28 bytes */

        /* ── NK_ENUM_DECL ───────────────────────────────────────────── */
        /* Python: EnumDecl.name, .members
           members child list: alternating [NK_IDENTIFIER(name), value_expr, ...]
           value_expr may be UL_NULL_NODE if no explicit value */
        struct {
            StrId        name;      /* 4 */
            UL_Children  members;   /* 8 — alternating name/value pairs */
            uint8_t      _p[16];
        } enum_decl; /* 28 bytes */

        /* ── NK_TUPLE_RETURN ────────────────────────────────────────── */
        /* Python: TupleReturn.types */
        struct {
            UL_Children types;      /* 8 — children: NK_TYPE_NAME nodes */
            uint8_t     _p[20];
        } tuple_return; /* 28 bytes */

        /* ── NK_TUPLE_DESTRUCTURE ───────────────────────────────────── */
        /* Python: TupleDestructure.targets, .value */
        struct {
            NodeId       value;     /* 4 — RHS expression */
            UL_Children  targets;   /* 8 — children: NK_PARAM nodes */
            uint8_t      _p[16];
        } tuple_destr; /* 28 bytes */

        /* ── NK_DR_DIRECTIVE ────────────────────────────────────────── */
        /* Python: DrDirective.key, .value */
        struct {
            StrId    key;           /* 4 — e.g. "memory", "safety" */
            StrId    value;         /* 4 — e.g. "gc", "checked" */
            uint8_t  _p[20];
        } dr_directive; /* 28 bytes */

        /* ── NK_NORM_DIRECTIVE ──────────────────────────────────────── */
        /* Python: NormDirective.level */
        struct {
            uint32_t level;         /* 4 — 0–3 */
            uint8_t  _p[24];
        } norm_directive; /* 28 bytes */

        /* ── NK_PROGRAM ─────────────────────────────────────────────── */
        /* Python: Program.decls */
        struct {
            UL_Children decls;      /* 8 — top-level declarations */
            uint8_t     _p[20];
        } program; /* 28 bytes */

        /* Raw 28-byte fallback — always safe to zero-initialise */
        uint8_t _raw[28];

    } u; /* 28 bytes union payload */

}; /* UL_Node: 4 bytes header + 28 bytes payload = 32 bytes total */

/* =========================================================================
 * int64 / double accessor macros for split lo/hi storage
 *
 * int64_t and double are stored as two uint32_t fields (lo, hi) inside
 * int_lit and float_lit payloads. This avoids 8-byte alignment requirements
 * that would force the union — and therefore UL_Node — above 32 bytes.
 *
 * These macros reconstruct/store the 64-bit value via memcpy so the
 * compiler can optimise them freely without strict-aliasing concerns.
 * ========================================================================= */

#include <string.h> /* for memcpy in macros */

/* Read int64_t from a split int_lit node */
static inline int64_t ul_int_lit_get(const UL_Node *n) {
    uint64_t raw = ((uint64_t)n->u.int_lit.hi << 32) | (uint64_t)n->u.int_lit.lo;
    int64_t  v;
    memcpy(&v, &raw, sizeof(v));
    return v;
}

/* Write int64_t into a split int_lit node */
static inline void ul_int_lit_set(UL_Node *n, int64_t value) {
    uint64_t raw;
    memcpy(&raw, &value, sizeof(raw));
    n->u.int_lit.lo = (uint32_t)(raw & 0xFFFFFFFFu);
    n->u.int_lit.hi = (uint32_t)(raw >> 32);
}

/* Read double from a split float_lit node */
static inline double ul_float_lit_get(const UL_Node *n) {
    uint64_t raw = ((uint64_t)n->u.float_lit.hi << 32) | (uint64_t)n->u.float_lit.lo;
    double   v;
    memcpy(&v, &raw, sizeof(v));
    return v;
}

/* Write double into a split float_lit node */
static inline void ul_float_lit_set(UL_Node *n, double value) {
    uint64_t raw;
    memcpy(&raw, &value, sizeof(raw));
    n->u.float_lit.lo = (uint32_t)(raw & 0xFFFFFFFFu);
    n->u.float_lit.hi = (uint32_t)(raw >> 32);
}

/* =========================================================================
 * Compile-time size assertions
 * ========================================================================= */

_Static_assert(sizeof(UL_Node)     == 32, "UL_Node must be exactly 32 bytes");
_Static_assert(sizeof(UL_Span)     ==  8, "UL_Span must be exactly 8 bytes");
_Static_assert(sizeof(UL_Children) ==  8, "UL_Children must be exactly 8 bytes");
_Static_assert(sizeof(UL_NodeFlags)==  1, "UL_NodeFlags must be exactly 1 byte");

/* =========================================================================
 * Well-known interned StrIds
 *
 * Pre-interned at arena_init(). Use these for O(1) type name comparison
 * anywhere in the compiler — no strcmp needed.
 * ========================================================================= */

typedef struct {
    /* Primitive type names */
    StrId kw_int;
    StrId kw_float;
    StrId kw_bool;
    StrId kw_string;
    StrId kw_none;
    StrId kw_array;
    StrId kw_list;
    /* Result type keywords */
    StrId kw_ok;
    StrId kw_error;
    /* Variable modifier */
    StrId kw_fixed;
    /* Common operators (interned for O(1) comparison in semantic checker) */
    StrId op_add;   /* "+"  */
    StrId op_sub;   /* "-"  */
    StrId op_mul;   /* "*"  */
    StrId op_div;   /* "/"  */
    StrId op_mod;   /* "%"  */
    StrId op_eq;    /* "==" */
    StrId op_ne;    /* "!=" */
    StrId op_lt;    /* "<"  */
    StrId op_le;    /* "<=" */
    StrId op_gt;    /* ">"  */
    StrId op_ge;    /* ">=" */
    StrId op_and;   /* "and" */
    StrId op_or;    /* "or"  */
    StrId op_not;   /* "not" */
    StrId op_assign;    /* "="  */
    StrId op_add_assign;/* "+=" */
    StrId op_sub_assign;/* "-=" */
    StrId op_mul_assign;/* "*=" */
    StrId op_div_assign;/* "/=" */
} UL_WellKnown;

/* Convenience macro: O(1) StrId equality (no strcmp) */
#define str_eq(a, b)  ((a) == (b))

#endif /* UL_AST_NODES_H */
