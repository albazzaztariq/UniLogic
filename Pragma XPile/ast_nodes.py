"""
Pragma AST Nodes
Each node represents one construct in a .run file.
"""

from dataclasses import dataclass, field
from typing import List, Optional, Any


# ─── Top level ────────────────────────────────────────────────────────────────

@dataclass
class Program:
    globals: List[Any] = field(default_factory=list)   # list of GlobalBlock
    mains:   List[Any] = field(default_factory=list)   # list of MainBlock


@dataclass
class GlobalBlock:
    body: List[Any] = field(default_factory=list)


@dataclass
class MainBlock:
    body: List[Any] = field(default_factory=list)


# ─── Declarations ─────────────────────────────────────────────────────────────

@dataclass
class ImportDecl:
    module: str


@dataclass
class VarDecl:
    """int x = 3  or  int x  (no initializer)"""
    type_name: str
    name: str
    init: Optional[Any] = None


@dataclass
class ConstantDecl:
    """constant int x = 3"""
    type_name: str
    name: str
    init: Any


@dataclass
class ArrayDecl:
    """array[String] ToDoList = [...]"""
    elem_type: str
    name: str
    init: List[Any] = field(default_factory=list)


@dataclass
class ListDecl:
    """List[String] Errands = [...]"""
    elem_type: str
    name: str
    init: List[Any] = field(default_factory=list)


@dataclass
class FunctionDecl:
    name: str
    return_type: str
    params: List[Any] = field(default_factory=list)   # list of Param
    body: List[Any] = field(default_factory=list)


@dataclass
class Param:
    type_name: str
    name: str


@dataclass
class ObjectDecl:
    name: str
    parent: Optional[str] = None              # inherits ParentName
    ifaces: List[str] = field(default_factory=list)   # interfaces A, B, C
    fields: List[Any] = field(default_factory=list)   # list of ObjectField
    methods: List[Any] = field(default_factory=list)  # list of FunctionDecl


@dataclass
class ObjectField:
    type_name: str
    name: str
    default: Optional[Any] = None


@dataclass
class InterfaceDecl:
    """interface Athlete ... end interface"""
    name: str
    methods: List[Any] = field(default_factory=list)   # FunctionDecl with empty body


@dataclass
class MandateMember:
    name: str
    value: Optional[int] = None   # explicit value, or None (auto-increment)


@dataclass
class MandateDecl:
    """mandate Priority low = 1, medium = 2, high = 3 end mandate"""
    name: str
    members: List[MandateMember] = field(default_factory=list)


# ─── Statements ───────────────────────────────────────────────────────────────

@dataclass
class AssignStmt:
    """name = expr  or  name op= expr"""
    target: Any       # Identifier or FieldAccess
    op: str           # '=' '+=' '-=' '*=' '/='
    value: Any


@dataclass
class ReturnStmt:
    value: Optional[Any] = None


@dataclass
class PrintStmt:
    value: Any


@dataclass
class IfStmt:
    condition: Any
    body: List[Any] = field(default_factory=list)
    elif_clauses: List[Any] = field(default_factory=list)   # list of ElifClause
    else_body: Optional[List[Any]] = None


@dataclass
class ElifClause:
    condition: Any
    body: List[Any] = field(default_factory=list)


@dataclass
class ForEachStmt:
    var: str
    iterable: Any
    body: List[Any] = field(default_factory=list)


@dataclass
class ForStmt:
    """for int i = 0 while i < 10 do i++"""
    init: Any          # VarDecl
    condition: Any
    step: Any
    body: List[Any] = field(default_factory=list)


@dataclass
class WhileStmt:
    """while x < 5 do ... end do"""
    condition: Any
    body: List[Any] = field(default_factory=list)


@dataclass
class TryStmt:
    body: List[Any] = field(default_factory=list)
    catches: List[Any] = field(default_factory=list)   # list of CatchClause


@dataclass
class CatchClause:
    exceptions: List[str]    # one or more exception type names
    var: str
    body: List[Any] = field(default_factory=list)


@dataclass
class NewStmt:
    """new Runner  or  new Cyclist["Jake", 17, 1.6]"""
    type_name: str
    args: List[Any] = field(default_factory=list)


@dataclass
class EscapeStmt:
    """escape — breaks out of the nearest loop or conditions block"""
    pass


@dataclass
class SkipStmt:
    """skip — continues to the next loop iteration"""
    pass


@dataclass
class RawBlock:
    body: List[Any] = field(default_factory=list)


@dataclass
class ConditionsCase:
    value: Optional[Any]     # None when is_default=True
    is_default: bool
    body: List[Any] = field(default_factory=list)


@dataclass
class ConditionsStmt:
    """conditions x ... end conditions"""
    expr: Any
    cases: List[ConditionsCase] = field(default_factory=list)


# ─── Expressions ──────────────────────────────────────────────────────────────

@dataclass
class Identifier:
    name: str


@dataclass
class SuperRef:
    """Represents 'super' — the parent object reference."""
    pass


@dataclass
class FieldAccess:
    """Runner.age  or  super.Method"""
    obj: Any
    field: str


@dataclass
class CallExpr:
    """AddNums(a, b)  or  obj.method(args)"""
    func: Any          # Identifier or FieldAccess
    args: List[Any] = field(default_factory=list)


@dataclass
class BinOp:
    op: str
    left: Any
    right: Any


@dataclass
class UnaryOp:
    op: str            # '!' or '-' or 'flip'
    operand: Any


@dataclass
class PostfixOp:
    op: str            # '++' or '--'
    operand: Any


@dataclass
class IndexExpr:
    """arr[i]"""
    array: Any
    index: Any


@dataclass
class CastExpr:
    """change(x)->int"""
    expr: Any
    target_type: str


@dataclass
class IntLit:
    value: int


@dataclass
class FloatLit:
    value: float


@dataclass
class StringLit:
    value: str


@dataclass
class BoolLit:
    value: bool


@dataclass
class NullLit:
    pass


@dataclass
class EmptyLit:
    pass


# ─── Memory (RAW blocks) ──────────────────────────────────────────────────────

@dataclass
class PointerDecl:
    """pointer int64 p = ADDRESS<num1>"""
    type_name: str
    name: str
    target: Any        # AddressOf node


@dataclass
class AddressOf:
    target: str        # variable name


@dataclass
class MemAccess:
    """MEM<0xDEADBEEF>"""
    address: Any


@dataclass
class DerefExpr:
    """DEREF<p>"""
    pointer: Any


@dataclass
class SizeOf:
    """SIZE<Type>"""
    type_name: str
