# ulc/ast_nodes.py — UniLogic AST node definitions
# Every node is a plain dataclass. No methods — data only.
# The semantic checker and codegen walk these trees.

from dataclasses import dataclass, field
from typing import Optional, List, Any


# ── Types ─────────────────────────────────────────────────────────────────────

@dataclass
class TypeName:
    name: str                        # e.g. 'int', 'string', 'bool'
    pointer: bool = False            # true if wrapped in < >
    line: int = 0
    col: int = 0
    is_array: bool = False           # true if 'array int' declaration
    array_size: int = 0              # 0 = inferred from initializer
    is_list: bool = False            # true if 'list int' declaration


# ── Expressions ───────────────────────────────────────────────────────────────

@dataclass
class IntLiteral:
    value: int
    line: int = 0; col: int = 0

@dataclass
class FloatLiteral:
    value: float
    line: int = 0; col: int = 0

@dataclass
class StringLiteral:
    value: str
    line: int = 0; col: int = 0

@dataclass
class BoolLiteral:
    value: bool
    line: int = 0; col: int = 0

@dataclass
class EmptyLiteral:          # 'empty' — null/None
    line: int = 0; col: int = 0

@dataclass
class Identifier:
    name: str
    line: int = 0; col: int = 0

@dataclass
class BinaryOp:
    op: str                  # '+', '-', '==', 'and', etc.
    left: Any
    right: Any
    line: int = 0; col: int = 0

@dataclass
class UnaryOp:
    op: str                  # 'not', '-', 'address', 'deref'
    operand: Any
    line: int = 0; col: int = 0

@dataclass
class Cast:
    expr: Any                # change(x)->int
    target_type: TypeName
    line: int = 0; col: int = 0

@dataclass
class Call:
    name: str
    args: List[Any]
    line: int = 0; col: int = 0

@dataclass
class Index:
    target: Any              # arr[i]
    index: Any
    line: int = 0; col: int = 0

@dataclass
class SliceExpr:             # arr[i:j] or str[i:j]
    target: Any
    start: Any               # start index expression
    end: Any                 # end index expression (exclusive)
    line: int = 0; col: int = 0

@dataclass
class ArrayLiteral:          # [1, 2, 3]
    elements: List[Any]
    line: int = 0; col: int = 0

@dataclass
class ArrayComprehension:    # [expr for var in iterable]
    expr: Any                # the expression to evaluate
    var: str                 # loop variable name
    iterable: Any            # iterable expression (e.g. range(1, 6))
    line: int = 0; col: int = 0

@dataclass
class FieldAccess:            # p.x
    target: Any               # expression (e.g. Identifier)
    field: str                # field name
    line: int = 0; col: int = 0

@dataclass
class OkResult:               # ok expr
    value: Any
    line: int = 0; col: int = 0

@dataclass
class ErrorResult:            # error "msg"
    value: Any
    line: int = 0; col: int = 0

@dataclass
class ResultPropagation:      # expr? — unwrap ok or propagate error
    expr: Any
    line: int = 0; col: int = 0


# ── Statements ────────────────────────────────────────────────────────────────

@dataclass
class VarDecl:
    type_: TypeName
    name: str
    init: Optional[Any]      # None if no initializer
    fixed: bool = False      # 'fixed' keyword — const in C
    line: int = 0; col: int = 0

@dataclass
class Assign:
    target: Any              # Identifier or Index
    op: str                  # '=', '+=', '-=', etc.
    value: Any
    line: int = 0; col: int = 0

@dataclass
class If:
    condition: Any
    then_body: List[Any]
    else_body: List[Any]     # empty list if no else
    line: int = 0; col: int = 0

@dataclass
class While:
    condition: Any
    body: List[Any]
    line: int = 0; col: int = 0

@dataclass
class DoWhile:
    body: List[Any]
    condition: Any               # checked after each iteration
    line: int = 0; col: int = 0

@dataclass
class For:
    var: str                 # loop variable name
    iterable: Any            # expression after 'in'
    body: List[Any]
    line: int = 0; col: int = 0
    parallel: bool = False   # true for 'parallel for'

@dataclass
class CFor:
    init: Any                # VarDecl or Assign for initialization
    condition: Any           # loop condition expression
    update: Any              # update statement (PostIncrement, Assign, etc.)
    body: List[Any]
    line: int = 0; col: int = 0

@dataclass
class Return:
    value: Optional[Any]
    line: int = 0; col: int = 0

@dataclass
class Print:
    value: Any
    line: int = 0; col: int = 0

@dataclass
class ExprStmt:              # a bare expression used as a statement (e.g. a call)
    expr: Any
    line: int = 0; col: int = 0

@dataclass
class Escape:                # 'escape' = break
    line: int = 0; col: int = 0

@dataclass
class Continue:
    line: int = 0; col: int = 0

@dataclass
class PortalDecl:             # portal label_name
    name: str
    line: int = 0; col: int = 0

@dataclass
class GotoStmt:               # goto label_name
    target: str
    line: int = 0; col: int = 0


@dataclass
class KillswitchStmt:
    condition: Any             # boolean expression — abort if false
    line: int = 0; col: int = 0

@dataclass
class CatchClause:
    exception_type: Optional[str]  # None for catch-all
    binding_name: Optional[str]    # None if no "as name"
    body: List[Any] = field(default_factory=list)
    line: int = 0; col: int = 0

@dataclass
class TryStmt:
    body: List[Any]
    handlers: List[Any]            # list of CatchClause
    finally_body: List[Any]        # empty if no finally
    line: int = 0; col: int = 0

@dataclass
class ThrowStmt:
    exception_type: str
    message: Any                   # expression for message
    line: int = 0; col: int = 0


@dataclass
class PostIncrement:           # x++
    operand: Any
    line: int = 0; col: int = 0

@dataclass
class PostDecrement:           # x--
    operand: Any
    line: int = 0; col: int = 0

@dataclass
class Yield:
    value: Any
    line: int = 0; col: int = 0

@dataclass
class MatchCase:
    value: Optional[Any]       # expression, or None for 'default'
    body: List[Any]
    line: int = 0; col: int = 0

@dataclass
class Match:
    subject: Any               # expression being matched
    cases: List[MatchCase]
    line: int = 0; col: int = 0


# ── Top-level declarations ────────────────────────────────────────────────────

@dataclass
class TupleReturn:
    types: List[TypeName]            # (int, int) → [TypeName("int"), TypeName("int")]
    line: int = 0; col: int = 0

@dataclass
class TupleLiteral:
    elements: List[Any]              # (expr1, expr2, ...) for multi-value return
    line: int = 0; col: int = 0

@dataclass
class TupleDestructure:
    targets: List[Any]               # [(type, name), ...] for (int lo, int hi) = call()
    value: Any
    line: int = 0; col: int = 0

@dataclass
class Param:
    type_: TypeName
    name: str
    default: Optional[Any] = None    # default value expression (literal)
    line: int = 0; col: int = 0

@dataclass
class FunctionDecl:
    name: str
    params: List[Param]
    return_type: Optional[TypeName]   # None means 'none' / void
    body: List[Any]
    line: int = 0; col: int = 0
    is_generator: bool = False        # true if 'yields' instead of 'returns'
    returns_result: bool = False      # true if 'returns ok|error'
    is_imported: bool = False         # true if resolved from import "file.ul"
    variadic: bool = False            # true if last param is ...
    nested_functions: List[Any] = field(default_factory=list)

@dataclass
class ForeignImport:
    lib: str                         # library name, e.g. "math", "stdio"
    name: str                        # function name, e.g. "sqrt", "printf"
    params: List[Param]
    return_type: Optional[TypeName]  # None means 'none' / void
    variadic: bool = False           # true if last param is "..."
    line: int = 0; col: int = 0

@dataclass
class TypeDecl:
    name: str
    fields: List[Param]              # reuses Param (type_ + name)
    parent: Optional[str] = None     # parent type name for inheritance
    line: int = 0; col: int = 0

@dataclass
class ObjectDecl:
    name: str
    fields: List[Param]              # data fields
    methods: List[Any]               # list of FunctionDecl
    parent: Optional[str] = None     # parent object name for inheritance
    line: int = 0; col: int = 0

@dataclass
class StructLiteral:
    type_name: str                   # e.g. 'Point'
    fields: List[Any]                # list of (name, expr) tuples
    line: int = 0; col: int = 0

@dataclass
class MethodCall:
    target: Any                      # expression — the object instance
    method: str                      # method name
    args: List[Any]
    line: int = 0; col: int = 0

@dataclass
class EnumDecl:
    name: str
    members: List[Any]           # list of (name, value) tuples
    line: int = 0; col: int = 0

@dataclass
class ConstDecl:
    type_: TypeName
    name: str
    value: Any                       # must be a literal
    line: int = 0; col: int = 0

@dataclass
class AsmBlock:
    arch: str                        # e.g. 'x86_64'
    body: str                        # raw assembly text
    line: int = 0; col: int = 0

@dataclass
class WithStmt:
    expr: Any                        # resource expression (e.g. open("file.txt"))
    binding: str                     # variable name (e.g. 'f')
    body: List[Any]
    line: int = 0; col: int = 0

@dataclass
class DrDirective:
    key: str                         # e.g. 'memory', 'safety'
    value: str                       # e.g. 'gc', 'checked'
    line: int = 0; col: int = 0

@dataclass
class NormDirective:
    level: int                       # 0–3
    line: int = 0; col: int = 0

@dataclass
class Program:
    decls: List[Any]                 # FunctionDecl, ForeignImport, TypeDecl, ObjectDecl, ConstDecl, DrDirective, or NormDirective
