# ulc/unparse.py — Reconstruct readable UniLogic source from AST nodes
# Used by --dry-run to show the effect of normalization passes.

from ast_nodes import *


def unparse(program):
    """Convert a Program AST back into readable UniLogic source."""
    lines = []
    for decl in program.decls:
        if isinstance(decl, DrDirective):
            lines.append(f"@dr {decl.key} = {decl.value}")
        elif isinstance(decl, NormDirective):
            lines.append(f"@norm {decl.level}")
        elif isinstance(decl, TypeDecl):
            header = f"type {decl.name}"
            if decl.parent:
                header += f" inherits {decl.parent}"
            lines.append(header)
            for f in decl.fields:
                lines.append(f"  {f.type_.name} {f.name}")
            lines.append("end type")
            lines.append("")
        elif isinstance(decl, ForeignImport):
            params = ", ".join(f"{p.type_.name} {p.name}" for p in decl.params)
            ret = f" returns {decl.return_type.name}" if decl.return_type else ""
            variadic = ", ..." if decl.variadic else ""
            lines.append(f'import "{decl.lib}" function {decl.name}({params}{variadic}){ret}')
        elif isinstance(decl, FunctionDecl):
            params = ", ".join(f"{p.type_.name} {p.name}" for p in decl.params)
            if getattr(decl, 'is_generator', False):
                ret = f" yields {decl.return_type.name}" if decl.return_type else ""
            elif getattr(decl, 'returns_result', False):
                ret = f" returns {decl.return_type.name}|error" if decl.return_type else " returns ok|error"
            else:
                ret = f" returns {decl.return_type.name}" if decl.return_type else ""
            lines.append(f"function {decl.name}({params}){ret}")
            _emit_body(decl.body, lines, 1)
            lines.append("end function")
            lines.append("")
    # Remove trailing blank lines
    while lines and lines[-1] == "":
        lines.pop()
    return "\n".join(lines) + "\n"


def _emit_body(stmts, lines, depth):
    ind = "  " * depth
    for s in stmts:
        t = type(s)
        if t == VarDecl:
            fixed = "fixed " if s.fixed else ""
            arr = "array " if s.type_.is_array else ""
            init = f" = {_expr(s.init)}" if s.init else ""
            lines.append(f"{ind}{fixed}{arr}{s.type_.name} {s.name}{init}")
        elif t == Assign:
            lines.append(f"{ind}{_expr(s.target)} {s.op} {_expr(s.value)}")
        elif t == If:
            lines.append(f"{ind}if {_expr(s.condition)}")
            _emit_body(s.then_body, lines, depth + 1)
            if s.else_body:
                lines.append(f"{ind}else")
                _emit_body(s.else_body, lines, depth + 1)
            lines.append(f"{ind}end if")
        elif t == While:
            lines.append(f"{ind}while {_expr(s.condition)}")
            _emit_body(s.body, lines, depth + 1)
            lines.append(f"{ind}end while")
        elif t == For:
            lines.append(f"{ind}for each {s.var} in {_expr(s.iterable)}")
            _emit_body(s.body, lines, depth + 1)
            lines.append(f"{ind}end for")
        elif t == Match:
            lines.append(f"{ind}match {_expr(s.subject)}")
            for case in s.cases:
                if case.value is None:
                    lines.append(f"{ind}  default")
                else:
                    lines.append(f"{ind}  {_expr(case.value)}")
                _emit_body(case.body, lines, depth + 2)
            lines.append(f"{ind}end match")
        elif t == Return:
            if s.value is not None:
                if isinstance(s.value, OkResult):
                    lines.append(f"{ind}return ok {_expr(s.value.value)}")
                elif isinstance(s.value, ErrorResult):
                    lines.append(f"{ind}return error {_expr(s.value.value)}")
                else:
                    lines.append(f"{ind}return {_expr(s.value)}")
            else:
                lines.append(f"{ind}return")
        elif t == Print:
            lines.append(f"{ind}print {_expr(s.value)}")
        elif t == ExprStmt:
            lines.append(f"{ind}{_expr(s.expr)}")
        elif t == Escape:
            lines.append(f"{ind}escape")
        elif t == Continue:
            lines.append(f"{ind}continue")
        elif t == Yield:
            lines.append(f"{ind}yield {_expr(s.value)}")


def _expr(node):
    if node is None:
        return ""
    t = type(node)
    if t == Identifier:
        return node.name
    if t == IntLiteral:
        return str(node.value)
    if t == FloatLiteral:
        return str(node.value)
    if t == StringLiteral:
        return f'"{node.value}"'
    if t == BoolLiteral:
        return "true" if node.value else "false"
    if t == EmptyLiteral:
        return "empty"
    if t == BinaryOp:
        return f"{_expr(node.left)} {node.op} {_expr(node.right)}"
    if t == UnaryOp:
        if node.op == "-":
            return f"-{_expr(node.operand)}"
        return f"{node.op} {_expr(node.operand)}"
    if t == Call:
        args = ", ".join(_expr(a) for a in node.args)
        return f"{node.name}({args})"
    if t == Cast:
        return f"cast({_expr(node.expr)}, {node.target_type.name})"
    if t == Index:
        return f"{_expr(node.target)}[{_expr(node.index)}]"
    if t == ArrayLiteral:
        return f"[{', '.join(_expr(e) for e in node.elements)}]"
    if t == FieldAccess:
        return f"{_expr(node.target)}.{node.field}"
    if t == PostIncrement:
        return f"{_expr(node.operand)}++"
    if t == PostDecrement:
        return f"{_expr(node.operand)}--"
    if t == OkResult:
        return f"ok {_expr(node.value)}"
    if t == ErrorResult:
        return f"error {_expr(node.value)}"
    if t == ResultPropagation:
        return f"{_expr(node.expr)}?"
    return "???"
