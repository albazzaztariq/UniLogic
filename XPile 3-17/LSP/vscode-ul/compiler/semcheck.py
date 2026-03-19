# ulc/semcheck.py — UniLogic semantic checker
# Walks the AST and verifies:
#   - All variables are declared before use
#   - No duplicate declarations in the same scope
#   - Function calls reference declared functions
#   - Return types match function signatures (basic)
# Annotates each node with a 'resolved_type' where possible.
# Raises SemanticError on any violation.

from ast_nodes import *


class SemanticError(Exception):
    pass


# Map UL type names to C equivalents (used later by codegen too)
UL_TO_C = {
    "int":    "int",
    "int8":   "int8_t",
    "int16":  "int16_t",
    "int32":  "int32_t",
    "int64":  "int64_t",
    "uint8":  "uint8_t",
    "uint16": "uint16_t",
    "uint32": "uint32_t",
    "uint64": "uint64_t",
    "float":  "float",
    "double": "double",
    "string": "char*",
    "bool":   "int",       # C has no bool without stdbool — use int by default
    "none":   "void",
    "complex": "double complex",
    "auto":   "__auto_type",
}

NUMERIC_TYPES = {"int","int8","int16","int32","int64",
                 "uint8","uint16","uint32","uint64","float","double"}


class Scope:
    def __init__(self, parent=None):
        self.vars   = {}   # name -> TypeName
        self.parent = parent

    def declare(self, name, type_, line, col):
        if name in self.vars:
            raise SemanticError(f"line {line}:{col} — '{name}' already declared in this scope")
        self.vars[name] = type_

    def lookup(self, name):
        if name in self.vars:
            return self.vars[name]
        if self.parent:
            return self.parent.lookup(name)
        return None


def _suggest_similar(name, candidates, max_dist=2):
    """Return up to 3 candidates with edit distance <= max_dist."""
    def _edit_dist(a, b):
        if abs(len(a) - len(b)) > max_dist:
            return max_dist + 1
        m, n = len(a), len(b)
        prev = list(range(n + 1))
        for i in range(1, m + 1):
            curr = [i] + [0] * n
            for j in range(1, n + 1):
                curr[j] = min(prev[j] + 1, curr[j-1] + 1,
                              prev[j-1] + (0 if a[i-1] == b[j-1] else 1))
            prev = curr
        return prev[n]
    hits = [(c, _edit_dist(name.lower(), c.lower())) for c in candidates]
    return [c for c, d in sorted(hits, key=lambda x: x[1]) if d <= max_dist][:3]


class Checker:
    def __init__(self, filename="<source>"):
        self.filename  = filename
        self.functions = {}   # name -> FunctionDecl
        self.types     = {}   # name -> TypeDecl
        self.current_fn = None

    def error(self, msg, line=0, col=0):
        raise SemanticError(f"{self.filename}:{line}:{col} — {msg}")

    def all_fields(self, type_name):
        """Return all fields for a type, including inherited fields (parent first)."""
        decl = self.types.get(type_name)
        if not decl:
            return []
        if decl.parent:
            return self.all_fields(decl.parent) + decl.fields
        return list(decl.fields)

    def check(self, program):
        # Register type and object declarations
        for decl in program.decls:
            if isinstance(decl, (TypeDecl, ObjectDecl)):
                if decl.name in self.types:
                    self.error(f"duplicate type '{decl.name}'", decl.line, decl.col)
                self.types[decl.name] = decl

        # Validate inheritance parents exist
        for decl in program.decls:
            if isinstance(decl, (TypeDecl, ObjectDecl)) and decl.parent:
                if decl.parent not in self.types:
                    self.error(f"type '{decl.name}' inherits from undefined type '{decl.parent}'",
                               decl.line, decl.col)

        # Register constants
        for decl in program.decls:
            if isinstance(decl, ConstDecl):
                self.constants = getattr(self, 'constants', {})
                self.constants[decl.name] = decl

        # First pass: register all function signatures so forward calls work
        for decl in program.decls:
            if isinstance(decl, ForeignImport):
                self.functions[decl.name] = decl
                continue
            if isinstance(decl, (TypeDecl, ObjectDecl, ConstDecl, DrDirective, NormDirective, AsmBlock)):
                continue
            if decl.name in self.functions:
                self.error(f"duplicate function '{decl.name}'", decl.line, decl.col)
            self.functions[decl.name] = decl
            # Also register nested functions so forward calls work
            for nf in getattr(decl, 'nested_functions', []):
                self.functions[nf.name] = nf

        # Register object methods as functions with object prefix
        for decl in program.decls:
            if isinstance(decl, ObjectDecl):
                for method in decl.methods:
                    method_key = f"{decl.name}.{method.name}"
                    self.functions[method_key] = method

        # Second pass: check each function body (skip non-functions)
        for decl in program.decls:
            if isinstance(decl, (ForeignImport, TypeDecl, ObjectDecl, ConstDecl,
                                 DrDirective, NormDirective, AsmBlock)):
                continue
            self.check_function(decl)

        # Check object method bodies — inject object fields into scope
        for decl in program.decls:
            if isinstance(decl, ObjectDecl):
                for method in decl.methods:
                    self.check_object_method(method, decl)

    def check_object_method(self, decl, obj_decl):
        """Check a method body with object fields injected into scope."""
        self.current_fn = decl
        scope = Scope()
        # Inject object fields (including inherited) so methods can reference them
        all_fields = list(obj_decl.fields)
        if obj_decl.parent and obj_decl.parent in self.types:
            parent = self.types[obj_decl.parent]
            if hasattr(parent, 'fields'):
                all_fields = list(parent.fields) + all_fields
        for f in all_fields:
            scope.declare(f.name, f.type_, f.line, f.col)
        for param in decl.params:
            scope.declare(param.name, param.type_, param.line, param.col)
        self.check_body(decl.body, scope)
        self.current_fn = None

    def check_function(self, decl):
        self.current_fn = decl
        scope = Scope()
        for param in decl.params:
            scope.declare(param.name, param.type_, param.line, param.col)
        # Register nested functions in scope so they can be called
        for nf in getattr(decl, 'nested_functions', []):
            scope.declare(nf.name, TypeName("none"), nf.line, nf.col)
        self.check_body(decl.body, scope)
        # Check nested function bodies
        for nf in getattr(decl, 'nested_functions', []):
            self.check_function(nf)

        # Enforce return on all paths for main() declared returns int
        if (decl.name == "main" and decl.return_type is not None
                and decl.return_type.name != "none"
                and not decl.returns_result and not decl.is_generator):
            if not self._definitely_returns(decl.body):
                self.error(
                    f"function '{decl.name}' declared to return '{decl.return_type.name}' "
                    f"but not all code paths return a value. "
                    f"Add 'return <value>' at the end of the function body.",
                    decl.line, decl.col)

        self.current_fn = None

    def _definitely_returns(self, stmts):
        """Check if a list of statements definitely returns on all paths."""
        for stmt in stmts:
            if isinstance(stmt, Return):
                return True
            if isinstance(stmt, If):
                # Both branches must return for the if to definitely return
                if (stmt.else_body
                        and self._definitely_returns(stmt.then_body)
                        and self._definitely_returns(stmt.else_body)):
                    return True
            if isinstance(stmt, Match):
                # All cases must return, and there must be a default
                has_default = any(c.value is None for c in stmt.cases)
                if has_default and all(
                        self._definitely_returns(c.body) for c in stmt.cases):
                    return True
        return False

    def check_body(self, stmts, scope):
        for stmt in stmts:
            self.check_stmt(stmt, scope)

    def check_stmt(self, node, scope):
        t = type(node)

        if t == VarDecl:
            if node.init is not None:
                self.check_expr(node.init, scope)
            scope.declare(node.name, node.type_, node.line, node.col)

        elif t == Assign:
            self.check_expr(node.target, scope)
            self.check_expr(node.value, scope)

        elif t == If:
            self.check_expr(node.condition, scope)
            self.check_body(node.then_body, Scope(scope))
            if node.else_body:
                self.check_body(node.else_body, Scope(scope))

        elif t == While:
            self.check_expr(node.condition, scope)
            self.check_body(node.body, Scope(scope))

        elif t == For:
            self.check_expr(node.iterable, scope)
            inner = Scope(scope)
            # loop var is implicitly int for now
            inner.declare(node.var, TypeName("int"), node.line, node.col)
            self.check_body(node.body, inner)

        elif t == Return:
            if node.value is not None:
                self.check_expr(node.value, scope)

        elif t == Print:
            self.check_expr(node.value, scope)

        elif t == ExprStmt:
            self.check_expr(node.expr, scope)

        elif t == Match:
            self.check_expr(node.subject, scope)
            for case in node.cases:
                if case.value is not None:
                    self.check_expr(case.value, scope)
                self.check_body(case.body, Scope(scope))

        elif t == Yield:
            if not self.current_fn or not self.current_fn.is_generator:
                self.error("'yield' used outside a generator function (use 'yields' in signature)",
                           node.line, node.col)
            self.check_expr(node.value, scope)

        elif t in (Escape, Continue):
            pass  # nothing to check

        elif t == TupleDestructure:
            self.check_expr(node.value, scope)
            for t_, name in node.targets:
                scope.declare(name, t_, node.line, node.col)

        elif t == NormDirective:
            pass  # inline @norm level change — handled by normalizer

        elif t == PostIncrement or t == PostDecrement:
            self.check_expr(node.operand, scope)

        else:
            self.error(f"unknown statement type {t.__name__}", 0, 0)

    def check_expr(self, node, scope):
        t = type(node)

        if t in (IntLiteral, FloatLiteral, StringLiteral, BoolLiteral, EmptyLiteral):
            return

        if t == ArrayLiteral:
            for elem in node.elements:
                self.check_expr(elem, scope)
            return

        if t == ArrayComprehension:
            self.check_expr(node.iterable, scope)
            inner = Scope(scope)
            inner.declare(node.var, TypeName("int"), node.line, node.col)
            self.check_expr(node.expr, inner)
            return

        if t == Identifier:
            if scope.lookup(node.name) is None:
                constants = getattr(self, 'constants', {})
                if node.name not in constants:
                    msg = f"undefined variable '{node.name}'"
                    # Collect all visible variable names for suggestions
                    visible = list(constants.keys())
                    s = scope
                    while s:
                        visible.extend(s.vars.keys())
                        s = s.parent
                    similar = _suggest_similar(node.name, visible)
                    if similar:
                        msg += f". Did you mean: {', '.join(similar)}?"
                    msg += f"\n  Hint: declare it first, e.g.: {node.name} int = 0"
                    self.error(msg, node.line, node.col)

        elif t == BinaryOp:
            self.check_expr(node.left, scope)
            self.check_expr(node.right, scope)

        elif t == UnaryOp:
            self.check_expr(node.operand, scope)

        elif t == Cast:
            self.check_expr(node.expr, scope)

        elif t == Call:
            if node.name not in self.functions and node.name not in BUILTINS:
                msg = f"call to undefined function '{node.name}'"
                all_fns = list(self.functions.keys()) + list(BUILTINS)
                similar = _suggest_similar(node.name, all_fns)
                if similar:
                    msg += f". Did you mean: {', '.join(similar)}?"
                self.error(msg, node.line, node.col)
            else:
                # Check argument count
                fn = self.functions.get(node.name)
                if fn and hasattr(fn, 'params'):
                    expected = len(fn.params)
                    got = len(node.args)
                    # Count required params (those without defaults)
                    required = sum(1 for p in fn.params if p.default is None)
                    # Allow variadic functions
                    is_variadic = getattr(fn, 'variadic', False)
                    if is_variadic:
                        if got < required:
                            self.error(
                                f"'{node.name}' requires at least {required} argument{'s' if required != 1 else ''}"
                                f" but was called with {got}",
                                node.line, node.col)
                    elif got < required or got > expected:
                        self.error(
                            f"'{node.name}' expects {required}{'-' + str(expected) if required != expected else ''}"
                            f" argument{'s' if expected != 1 else ''}"
                            f" but was called with {got}",
                            node.line, node.col)
            for arg in node.args:
                self.check_expr(arg, scope)

        elif t == Index:
            self.check_expr(node.target, scope)
            self.check_expr(node.index, scope)

        elif t == FieldAccess:
            self.check_expr(node.target, scope)
            parent_type = self._resolve_expr_type(node.target, scope)
            if parent_type and parent_type.name in self.types:
                field_names = [f.name for f in self.all_fields(parent_type.name)]
                if node.field not in field_names:
                    self.error(f"type '{parent_type.name}' has no field '{node.field}'",
                               node.line, node.col)

        elif t == PostIncrement or t == PostDecrement:
            self.check_expr(node.operand, scope)

        elif t == OkResult:
            self.check_expr(node.value, scope)

        elif t == ErrorResult:
            self.check_expr(node.value, scope)

        elif t == ResultPropagation:
            self.check_expr(node.expr, scope)

        elif t == MethodCall:
            self.check_expr(node.target, scope)
            for arg in node.args:
                self.check_expr(arg, scope)

        elif t == StructLiteral:
            if node.type_name not in self.types:
                self.error(f"unknown type '{node.type_name}'", node.line, node.col)
            for fname, fval in node.fields:
                self.check_expr(fval, scope)

        elif t == TupleLiteral:
            for elem in node.elements:
                self.check_expr(elem, scope)

        else:
            self.error(f"unknown expression type {t.__name__}", 0, 0)

    def _resolve_expr_type(self, node, scope):
        """Resolve the type of an expression. Returns TypeName or None."""
        if isinstance(node, Identifier):
            return scope.lookup(node.name)
        if isinstance(node, FieldAccess):
            parent_type = self._resolve_expr_type(node.target, scope)
            if parent_type and parent_type.name in self.types:
                for f in self.all_fields(parent_type.name):
                    if f.name == node.field:
                        return f.type_
        return None


# Built-in functions that don't need a UL declaration
BUILTINS = {
    "absval", "size", "memmove", "memcopy", "memset", "memtake", "memgive",
    "change", "exit", "range", "spawn", "wait", "lock", "unlock",
}


def check(program, filename="<source>"):
    Checker(filename).check(program)
