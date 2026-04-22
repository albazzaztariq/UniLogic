#!/usr/bin/env python3
"""py2ul.py — Python to UniLogic transpiler.

Translates Python source with type annotations into UniLogic (.ul) source.
Usage: python py2ul.py input.py [-o output.ul]
"""

import ast
import sys
import os
import textwrap


# UL keywords that conflict with common Python identifiers — prefix with _py_
# This must include ALL keywords and keyword aliases from lexer.py's KEYWORDS + _KW_ALIASES.
_UL_KEYWORDS = {
    # Core keywords (from KEYWORDS set in lexer.py)
    "function", "end", "returns", "return",
    "if", "else", "while", "do", "for", "each", "in",
    "match", "iterate", "default", "escape", "continue",
    "type", "inherits", "forks", "new", "fixed", "constant",
    "import", "from", "export", "print", "prompt",
    "parallel", "killswitch", "teleport", "portal", "goto",
    "try", "catch", "finally", "throw", "with", "as",
    "nocache", "yield", "yields", "inline", "pack",
    "true", "false", "empty",
    "and", "or", "not", "equals",
    "both1", "both0", "either1", "diff", "bitflip", "negate", "left", "right",
    "char_code", "sort",
    "address", "deref", "memmove", "memcopy", "memset", "memtake", "memgive",
    "size", "change", "absval",
    "int", "integer", "float", "double", "string", "bool", "none", "complex",
    "int8", "int16", "int32", "int64",
    "uint8", "uint16", "uint32", "uint64",
    "array", "list", "map", "arena", "file", "auto",
    "ok", "error", "some",
    "object", "const", "cast", "exit", "typeof", "spawn", "wait",
    "lock", "unlock", "try_lock",
    "random", "random_int", "random_seed", "userinput",
    # Keyword aliases (from _KW_ALIASES in lexer.py) — also tokenized as KW
    "integer", "rand", "rand_int", "rand_seed", "args",
}


def _safe_ident(name):
    """Prefix Python identifiers that collide with UL keywords, and sanitize non-ASCII chars."""
    # Sanitize non-ASCII characters — UL lexer only handles ASCII identifiers.
    # Replace each non-ASCII char with its Unicode code point (e.g. Ø → _u00d8).
    sanitized = ''.join(
        c if (c.isascii() and (c.isalnum() or c == '_')) else f"_u{ord(c):04x}"
        for c in name
    )
    # Ensure it doesn't start with a digit after sanitization
    if sanitized and sanitized[0].isdigit():
        sanitized = '_' + sanitized
    if sanitized != name:
        name = sanitized
    if name in _UL_KEYWORDS:
        return f"_py_{name}"
    return name


def _collect_free_vars(func_node):
    """Return set of names used-but-not-locally-defined in func_node's body.

    Uses Python's ast to walk the body, collecting Load references and
    Store targets, then returns Load - (Store ∪ params ∪ builtins).
    Does NOT recurse into nested function definitions (their own scope).
    """
    param_names = set()
    for arg in func_node.args.args + func_node.args.posonlyargs + func_node.args.kwonlyargs:
        param_names.add(arg.arg)
    if func_node.args.vararg:
        param_names.add(func_node.args.vararg.arg)
    if func_node.args.kwarg:
        param_names.add(func_node.args.kwarg.arg)

    loaded = set()
    stored = set()

    def walk(nodes):
        for node in nodes:
            if isinstance(node, ast.FunctionDef) or isinstance(node, ast.AsyncFunctionDef):
                # Don't recurse into nested-nested functions (their own scope)
                continue
            if isinstance(node, ast.Name):
                if isinstance(node.ctx, ast.Load):
                    loaded.add(node.id)
                elif isinstance(node.ctx, ast.Store):
                    stored.add(node.id)
            for child in ast.iter_child_nodes(node):
                walk([child])

    walk(func_node.body)
    # Free vars: referenced but not locally assigned and not a parameter
    return loaded - stored - param_names


class Py2UL(ast.NodeVisitor):
    def __init__(self, source_lines):
        self.lines = []
        self.depth = 0
        self.warnings = []
        self.source_lines = source_lines
        self.in_class = None  # current class name or None
        self.translated_lines = 0
        self._current_func = None    # enclosing function name for nested func detection
        self._hoisted_funcs = []     # (hoisted_lines, original_name, mangled_name) to emit at file scope
        self._vararg_name = None     # name of *args param in current function (remapped to _va_args in UL)
        self._defined_objects = set()    # track object names defined in this file
        self._imported_names = set()     # names brought in via import/from-import
        self._needed_stubs = []          # (name,) stub objects to emit before first use
        self._py_imports = []            # ("module", ["name1", "name2"]) Python imports to re-emit
        self._closure_globals = set()    # names promoted to module-level globals to support hoisted closures
        self._module_init_stmts = []     # imperative stmts (e.g. map_set calls) that must live in __module_init__

    def emit(self, text=""):
        self.lines.append("  " * self.depth + text)
        self.translated_lines += 1

    def emit_init(self, text=""):
        """Emit a statement into the __module_init__ function body (for module-scope imperative calls)."""
        self._module_init_stmts.append("  " + text)

    def warn(self, node, reason):
        lineno = getattr(node, 'lineno', 0)
        self.warnings.append((lineno, reason))
        self.emit(f"// TRANSPILER WARNING line {lineno}: {reason} — manual conversion required")

    # ── Type mapping ─────────────────────────────────────────────────────

    def map_type(self, node):
        """Convert a Python type annotation AST node to a UL type string."""
        if node is None:
            return "none"
        if isinstance(node, ast.Constant):
            if node.value is None:
                return "none"
            if isinstance(node.value, str):
                # String annotation (forward reference like "Report") — resolve via map_type(str)
                return self.map_type(node.value)
            return str(node.value)
        if isinstance(node, ast.Name):
            mapped = {
                'int': 'int', 'float': 'float', 'str': 'string',
                'bool': 'bool', 'None': 'none', 'NoneType': 'none',
                'bytes': 'string', 'list': 'array int', 'List': 'array int',
                'dict': 'map', 'Dict': 'map', 'set': 'array int', 'Set': 'array int',
                'tuple': 'array int', 'Tuple': 'array int',
                # typing wrappers that have no UL equivalent — treat as 'auto'
                'Final': 'auto', 'ClassVar': 'auto', 'Any': 'auto',
                'Type': 'auto', 'Callable': 'auto', 'Iterator': 'auto',
                'Generator': 'auto', 'Iterable': 'auto', 'Sequence': 'auto',
                'Mapping': 'auto', 'MutableMapping': 'auto',
                'FrozenSet': 'array int', 'frozenset': 'array int',
            }.get(node.id)
            if mapped:
                return mapped
            # If this is a locally-defined type (object/enum in this file), keep it.
            # Otherwise fall back to 'auto' — cross-file user types aren't known to the UL parser.
            if node.id in self._defined_objects:
                return node.id
            return 'auto'
        if isinstance(node, ast.Attribute):
            return self.map_type(node.attr) if isinstance(node.attr, str) else "int"
        if isinstance(node, ast.Subscript):
            base = self._name(node.value)
            if base in ('list', 'List'):
                inner = self.map_type(node.slice)
                # UL 'array' only accepts built-in type keywords as element type;
                # user-defined types as element types are not supported — fall back to array int.
                _ul_builtin_types = {
                    'int', 'float', 'double', 'string', 'bool', 'none',
                    'int8', 'int16', 'int32', 'int64',
                    'uint8', 'uint16', 'uint32', 'uint64', 'auto',
                }
                if inner not in _ul_builtin_types:
                    inner = 'int'
                return f"array {inner}"
            if base in ('Optional', 'Final', 'ClassVar', 'Annotated'):
                # Unwrap to inner type
                return self.map_type(node.slice)
            if base in ('tuple', 'Tuple'):
                return "array int"  # flatten
            if base in ('dict', 'Dict'):
                return "map"
            if base in ('set', 'Set', 'FrozenSet', 'frozenset'):
                return "array int"
            if base in ('Type', 'Callable', 'Iterator', 'Generator',
                        'Iterable', 'Sequence', 'Mapping', 'MutableMapping'):
                return "auto"
            return self.map_type(node.slice)
        if isinstance(node, ast.BinOp) and isinstance(node.op, ast.BitOr):
            # X | None → X
            left = self.map_type(node.left)
            right = self.map_type(node.right)
            if right == 'none':
                return left
            return left
        if isinstance(node, str):
            mapped = {'int': 'int', 'float': 'float', 'str': 'string', 'bool': 'bool',
                      'None': 'none', 'NoneType': 'none', 'bytes': 'string'}.get(node)
            if mapped:
                return mapped
            # User-defined type name as string: keep if locally defined, else 'auto'
            if node in self._defined_objects:
                return node
            return 'auto'
        self._warnings.append(f"py2ul: cannot infer type for annotation {ast.dump(node) if hasattr(node, '_fields') else node}")
        return "auto"

    def _name(self, node):
        if isinstance(node, ast.Name):
            return node.id
        if isinstance(node, ast.Attribute):
            return node.attr
        self._warnings.append(f"py2ul: cannot extract name from {type(node).__name__}")
        return f"__unknown_name_{type(node).__name__}"

    # ── Expressions ──────────────────────────────────────────────────────

    def expr(self, node):
        """Convert a Python expression AST to UL source string."""
        if isinstance(node, ast.Constant):
            if isinstance(node.value, str):
                # Escape special chars so string fits on one line in UL
                v = node.value
                v = v.replace(chr(92), chr(92)+chr(92))   # \ → \\
                v = v.replace(chr(34), chr(92)+chr(34))   # " → \"
                v = v.replace(chr(10), chr(92)+chr(110))  # newline → \n
                v = v.replace(chr(13), chr(92)+chr(114))  # CR → \r
                v = v.replace(chr(9),  chr(92)+chr(116))  # tab → \t
                # Escape non-ASCII chars — UL lexer/codegen only handles ASCII safely
                escaped = []
                for c in v:
                    if ord(c) > 127:
                        escaped.append(f"\\u{ord(c):04x}")
                    else:
                        escaped.append(c)
                v = ''.join(escaped)
                return f'"{v}"'
            if isinstance(node.value, bool):
                return "true" if node.value else "false"
            if node.value is None:
                return "empty"
            if isinstance(node.value, bytes):
                # bytes literal b'...' — emit as hex string in python_compat mode
                # codegen_python will pass it through as a string; bytes ops won't work
                # but this avoids a parse error from the b'...' prefix syntax
                hex_str = node.value.hex()
                self.warn(node, f"bytes literal — emitted as hex string '{hex_str}'")
                return f'"bytes:{hex_str}"'
            return str(node.value)

        if isinstance(node, ast.Name):
            if node.id == 'True': return 'true'
            if node.id == 'False': return 'false'
            if node.id == 'None': return 'empty'
            # Remap vararg references to _va_args (avoids collision with UL's 'args'/'userinput' alias)
            if self._vararg_name and node.id == self._vararg_name:
                return "_va_args"
            # Remap bare references to hoisted nested functions to their mangled names
            for _, orig_name, mangled in self._hoisted_funcs:
                if node.id == orig_name:
                    return mangled
            return _safe_ident(node.id)

        if isinstance(node, ast.BinOp):
            left = self.expr(node.left)
            right = self.expr(node.right)
            op = self._binop(node.op)
            return f"{left} {op} {right}"

        if isinstance(node, ast.UnaryOp):
            operand = self.expr(node.operand)
            if isinstance(node.op, ast.USub):
                return f"-{operand}"
            if isinstance(node.op, ast.Not):
                return f"not {operand}"
            if isinstance(node.op, ast.Invert):
                return f"bitflip({operand})"
            self._warnings.append(f"py2ul: unhandled unary op {type(node.op).__name__}")
            return f"/* ERROR: unhandled unary {type(node.op).__name__} */ {operand}"

        if isinstance(node, ast.BoolOp):
            op = "and" if isinstance(node.op, ast.And) else "or"
            parts = [self.expr(v) for v in node.values]
            return f" {op} ".join(parts)

        if isinstance(node, ast.Compare):
            left = self.expr(node.left)
            # Handle `x in collection` and `x not in collection`
            if len(node.ops) == 1 and isinstance(node.ops[0], ast.In):
                return f"{self.expr(node.comparators[0])}.contains({left})"
            if len(node.ops) == 1 and isinstance(node.ops[0], ast.NotIn):
                return f"not {self.expr(node.comparators[0])}.contains({left})"
            # Chained comparison: 300 <= x <= 399 → split into individual comparisons joined by 'and'
            # UL parser is left-associative and doesn't support chained comparisons
            if len(node.ops) > 1:
                parts = []
                operands = [node.left] + list(node.comparators)
                for i, (op, right) in enumerate(zip(node.ops, node.comparators)):
                    lhs = self.expr(operands[i])
                    rhs = self.expr(operands[i + 1])
                    parts.append(f"{lhs} {self._cmpop(op)} {rhs}")
                return " and ".join(parts)
            parts = [left]
            for op, comp in zip(node.ops, node.comparators):
                parts.append(self._cmpop(op))
                parts.append(self.expr(comp))
            return " ".join(parts)

        if isinstance(node, ast.Call):
            func = self.expr(node.func)
            args = [self.expr(a) for a in node.args]
            # Include keyword arguments as positional — UL has no keyword argument syntax.
            # This preserves arity for calls with keyword args that have defaults in the def.
            for kw in node.keywords:
                if kw.arg is not None:  # skip **kwargs (arg is None)
                    args.append(self.expr(kw.value))
            # Remap hoisted nested function calls to mangled names
            for _, orig_name, mangled in self._hoisted_funcs:
                if func == orig_name:
                    func = mangled
                    break
            # print → print (statement, handled elsewhere)
            if func == 'print':
                return f'print({", ".join(args)})'
            if func == 'len':
                return f"{args[0]}.len()" if args else "0"
            if func == 'range':
                return f"range({', '.join(args)})"
            if func == 'int':
                return f"cast({args[0]}, int)" if args else "0"
            if func == 'float':
                return f"cast({args[0]}, float)" if args else "0.0"
            if func == 'str':
                return f"cast({args[0]}, string)" if args else '""'
            if func == 'abs':
                return f"absval({args[0]})" if args else "0"
            if func == 'ord':
                # ord(s) or ord(s[i]) → char_code(s, 0) or char_code(s, i)
                if args:
                    arg_node = node.args[0]
                    if isinstance(arg_node, ast.Subscript):
                        target = self.expr(arg_node.value)
                        idx = self.expr(arg_node.slice)
                        return f"char_code({target}, {idx})"
                    return f"char_code({args[0]}, 0)"
                return "0"
            if func == 'chr':
                return f"char_from_code({args[0]})" if args else '""'
            if func == 'input':
                return f"userinput({', '.join(args)})"
            return f"{func}({', '.join(args)})"

        if isinstance(node, ast.Attribute):
            target = self.expr(node.value)
            # Remap Python string method names to UL equivalents
            attr_map = {
                'strip': 'trim', 'lstrip': 'trim', 'rstrip': 'trim',
                'startswith': 'starts_with', 'endswith': 'ends_with',
            }
            attr = attr_map.get(node.attr, node.attr)
            return f"{target}.{attr}"

        if isinstance(node, ast.Subscript):
            target = self.expr(node.value)
            # Handle slice: items[1:3], items[:5], items[2:]
            if isinstance(node.slice, ast.Slice):
                lower = self.expr(node.slice.lower) if node.slice.lower else "0"
                upper = self.expr(node.slice.upper) if node.slice.upper else f"{target}.len()"
                if node.slice.step:
                    self.warn(node, "slice with step — step ignored")
                return f"{target}[{lower}:{upper}]"
            idx = self.expr(node.slice)
            return f"{target}[{idx}]"

        if isinstance(node, ast.List):
            elems = ", ".join(self.expr(e) for e in node.elts)
            return f"[{elems}]"

        if isinstance(node, ast.Tuple):
            elems = ", ".join(self.expr(e) for e in node.elts)
            return f"[{elems}]"

        if isinstance(node, ast.IfExp):
            # ternary: a if cond else b → hoist to temp var with if/else block
            tmp = f"_tern{getattr(self, '_ternary_counter', 0)}"
            self._ternary_counter = getattr(self, '_ternary_counter', 0) + 1
            cond = self.expr(node.test)
            self.emit(f"auto {tmp} = empty")
            self.emit(f"if {cond}")
            self.depth += 1
            self.emit(f"{tmp} = {self.expr(node.body)}")
            self.depth -= 1
            self.emit(f"else")
            self.depth += 1
            self.emit(f"{tmp} = {self.expr(node.orelse)}")
            self.depth -= 1
            self.emit(f"end if")
            return tmp

        if isinstance(node, ast.JoinedStr):
            # f-string → concatenation
            parts = []
            for val in node.values:
                if isinstance(val, ast.Constant):
                    parts.append(self.expr(val))  # use expr() to get proper escaping
                elif isinstance(val, ast.FormattedValue):
                    parts.append(f"cast({self.expr(val.value)}, string)")
            return " + ".join(parts) if parts else '""'

        if isinstance(node, ast.ListComp):
            tmp = f"_lc{getattr(self, '_lc_counter', 0)}"
            self._lc_counter = getattr(self, '_lc_counter', 0) + 1
            elem_type = self._infer_type(node.elt)
            # UL doesn't support array of arrays (no 2D array syntax) — fall back to int
            _ul_builtin_elem_types = {
                'int', 'float', 'double', 'string', 'bool', 'none',
                'int8', 'int16', 'int32', 'int64',
                'uint8', 'uint16', 'uint32', 'uint64',
            }
            if elem_type not in _ul_builtin_elem_types:
                elem_type = 'int'
            self.emit(f"array {elem_type} {tmp}")
            _lc_is_zip = []  # track which generators used zip expansion
            for gen in node.generators:
                # Tuple unpacking in comprehension: [x for (a,b) in zip(...)]
                if isinstance(gen.target, ast.Tuple) and len(gen.target.elts) == 2:
                    if (isinstance(gen.iter, ast.Call) and isinstance(gen.iter.func, ast.Name)
                            and gen.iter.func.id == 'zip' and len(gen.iter.args) == 2):
                        cnt = getattr(self, '_zip_counter', 0)
                        self._zip_counter = cnt + 1
                        va = self.expr(gen.target.elts[0])
                        vb = self.expr(gen.target.elts[1])
                        sa = self.expr(gen.iter.args[0])
                        sb = self.expr(gen.iter.args[1])
                        idx = f"__zip_i{cnt}"
                        self.emit(f"int {idx} = 0")
                        self.emit(f"while {idx} < {sa}.len()")
                        self.depth += 1
                        self.emit(f"auto {va} = {sa}[{idx}]")
                        self.emit(f"auto {vb} = {sb}[{idx}]")
                        _lc_is_zip.append(True)
                        for cond in gen.ifs:
                            self.emit(f"if {self.expr(cond)}")
                            self.depth += 1
                        continue
                target = self.expr(gen.target)
                iter_expr = self.expr(gen.iter)
                self.emit(f"for each {target} in {iter_expr}")
                self.depth += 1
                _lc_is_zip.append(False)
                for cond in gen.ifs:
                    self.emit(f"if {self.expr(cond)}")
                    self.depth += 1
            self.emit(f"{tmp}.push({self.expr(node.elt)})")
            rev_zip = list(reversed(_lc_is_zip))
            for i, gen in enumerate(reversed(node.generators)):
                for _ in gen.ifs:
                    self.depth -= 1
                    self.emit("end if")
                if i < len(rev_zip) and rev_zip[i]:
                    # zip-expanded: emit index increment + end while
                    cnt = getattr(self, '_zip_counter', 1) - 1 - i
                    idx = f"__zip_i{max(0, cnt)}"
                    self.emit(f"{idx} = {idx} + 1")
                    self.depth -= 1
                    self.emit("end while")
                else:
                    self.depth -= 1
                    self.emit("end for")
            return tmp

        if isinstance(node, ast.Dict):
            # Dict → emit map_new() as a VarDecl, then map_set() calls.
            # map_set() is an imperative call — not valid at UL file scope.
            # When at module scope, route map_set calls to __module_init__.
            tmp = f"_dict{getattr(self, '_dict_counter', 0)}"
            self._dict_counter = getattr(self, '_dict_counter', 0) + 1
            at_module_scope = (self.depth == 0 and not self.in_class and not self._current_func)
            self.emit(f"// TRANSPILER: dict values cast to string — may lose type information")
            self.emit(f"map {tmp} = map_new()")
            for k, v in zip(node.keys, node.values):
                if k is not None:
                    stmt = f"map_set({tmp}, {self.expr(k)}, cast({self.expr(v)}, string))"
                    if at_module_scope:
                        self.emit_init(stmt)
                    else:
                        self.emit(stmt)
            return tmp

        if isinstance(node, ast.Set):
            # Set → array (no dedup at compile time)
            self.emit(f"// TRANSPILER: set translated to array — no deduplication")
            elems = ", ".join(self.expr(e) for e in node.elts)
            return f"[{elems}]"

        if isinstance(node, ast.Lambda):
            self.warn(node, "lambda function — define as named function")
            return "/* ERROR: lambda not supported — use named function */"

        if isinstance(node, ast.Starred):
            return self.expr(node.value)

        # Unknown expression — explicit error, not silent fallback
        node_type = type(node).__name__
        self.warnings.append((getattr(node, 'lineno', 0), f"unsupported expr: {node_type}"))
        return f"/* ERROR: unsupported {node_type} */"

    def _binop(self, op):
        ops = {
            ast.Add: '+', ast.Sub: '-', ast.Mult: '*', ast.Div: '/',
            ast.Mod: '%', ast.FloorDiv: '/', ast.Pow: '**',
            ast.BitAnd: 'both1', ast.BitOr: 'either1', ast.BitXor: 'diff',
            ast.LShift: 'left', ast.RShift: 'right',
        }
        return ops.get(type(op), '+')

    def _cmpop(self, op):
        ops = {
            ast.Eq: '==', ast.NotEq: '!=', ast.Lt: '<', ast.LtE: '<=',
            ast.Gt: '>', ast.GtE: '>=', ast.Is: '==', ast.IsNot: '!=',
            ast.In: '==', ast.NotIn: '!=',
        }
        return ops.get(type(op), '==')

    # ── Statements ───────────────────────────────────────────────────────

    def visit_Module(self, node):
        # Pre-pass: register all class/enum names defined anywhere in this module
        # so that forward references in type annotations resolve correctly.
        for stmt in node.body:
            if isinstance(stmt, ast.ClassDef):
                self._defined_objects.add(_safe_ident(stmt.name))
        for stmt in node.body:
            self.visit(stmt)

    def visit_FunctionDef(self, node):
        name = _safe_ident(node.name)
        # Skip self parameter for methods
        # Build params with default values where available
        defaults = node.args.defaults
        all_args = node.args.args
        # Align defaults to args (defaults apply to the last N args)
        num_defaults = len(defaults)
        num_args = len(all_args)
        params = []
        for i, arg in enumerate(all_args):
            if arg.arg == 'self':
                continue
            ptype = self.map_type(arg.annotation) if arg.annotation else "int"
            arg_name = _safe_ident(arg.arg)
            # Determine if this arg has a default
            # defaults align to the end: default for arg[num_args - num_defaults + j]
            default_idx = i - (num_args - num_defaults)
            if default_idx >= 0:
                default_node = defaults[default_idx]
                default_val = self.expr(default_node)
                # UL parser doesn't allow dotted expressions as default values;
                # fall back to 'empty' for attribute-access defaults.
                if '.' in default_val:
                    default_val = "empty"
                params.append(f"{ptype} {arg_name} = {default_val}")
            else:
                params.append(f"{ptype} {arg_name}")

        # Handle *args → variadic
        has_varargs = False
        vararg_name = None
        if node.args.vararg:
            has_varargs = True
            vararg_name = node.args.vararg.arg  # e.g. "args"
            params.append("...")

        ret = self.map_type(node.returns) if node.returns else ""
        ret_clause = f" returns {ret}" if ret and ret != "none" else ""

        # Check for decorators
        for dec in node.decorator_list:
            dec_name = self._name(dec) if isinstance(dec, (ast.Name, ast.Attribute)) else ""
            if dec_name not in ('staticmethod', 'classmethod', 'property'):
                self.warn(dec, f"decorator @{dec_name}")

        # Detect nested function: if we're inside another function (not a class method)
        is_nested = self._current_func is not None and not self.in_class
        if is_nested:
            # Hoist to file scope with prefixed name
            mangled = f"{self._current_func}__{name}"
            # Detect closure variables: names used in nested func body that are
            # not its own params — these come from the enclosing function scope.
            # Promote them to module-level globals so both the hoisted function
            # and the enclosing function can share them.
            free_vars = _collect_free_vars(node)
            # Filter out builtins and module-level names (keep only likely locals)
            _builtins = {'True', 'False', 'None', 'print', 'len', 'range', 'str',
                         'int', 'float', 'bool', 'list', 'dict', 'set', 'tuple',
                         'enumerate', 'zip', 'sorted', 'reversed', 'type', 'isinstance',
                         'hasattr', 'getattr', 'setattr', 'append', 'extend'}
            closure_vars = free_vars - _builtins - self._imported_names - self._defined_objects
            if closure_vars:
                for cv in sorted(closure_vars):
                    self._closure_globals.add(cv)
            saved_lines = self.lines
            saved_depth = self.depth
            self.lines = []
            self.depth = 0
            param_str = ", ".join(params)
            self.emit(f"function {mangled}({param_str}){ret_clause}")
            self.depth += 1
            saved_func = self._current_func
            saved_vararg = self._vararg_name
            self._current_func = mangled
            self._vararg_name = vararg_name
            for stmt in node.body:
                self.visit(stmt)
            self._current_func = saved_func
            self._vararg_name = saved_vararg
            self.depth -= 1
            self.emit("end function")
            self.emit("")
            hoisted_lines = self.lines
            self.lines = saved_lines
            self.depth = saved_depth
            self._hoisted_funcs.append((hoisted_lines, name, mangled))
            # Emit a comment in place of the original nested def
            self.emit(f"// nested function '{name}' hoisted to file scope as '{mangled}'")
        else:
            param_str = ", ".join(params)
            self.emit(f"function {name}({param_str}){ret_clause}")
            self.depth += 1
            saved_func = self._current_func
            saved_vararg = self._vararg_name
            self._current_func = name
            self._vararg_name = vararg_name  # track so body refs can be remapped
            for stmt in node.body:
                self.visit(stmt)
            self._current_func = saved_func
            self._vararg_name = saved_vararg
            self.depth -= 1
            self.emit("end function")
            self.emit("")

    visit_AsyncFunctionDef = visit_FunctionDef

    _ENUM_BASES = {'Enum', 'IntEnum', 'IntFlag', 'Flag', 'StrEnum'}

    def _is_enum_class(self, node):
        """Return True if this class is a simple enum definition.

        Returns True only when:
          1. At least one base is a known enum base (Enum, IntEnum, etc.), AND
          2. The class body contains ONLY assignments and docstrings (no methods).

        If the class has methods (e.g. __repr__, __str__), it is an enum subclass
        that extends/overrides behaviour — treat it as a regular object, not a
        UL 'fixed' enum, because 'fixed' enums cannot carry methods.
        """
        has_enum_base = any(
            self._name(base) in self._ENUM_BASES
            for base in node.bases
        )
        if not has_enum_base:
            return False
        # If any method is defined in the body, this is a subclass with overrides
        for stmt in node.body:
            if isinstance(stmt, (ast.FunctionDef, ast.AsyncFunctionDef)):
                return False
        return True

    def visit_ClassDef(self, node):
        name = _safe_ident(node.name)
        self._defined_objects.add(name)

        # Python IntEnum/Enum → UL fixed enum
        if self._is_enum_class(node):
            self.emit(f"fixed {name}")
            self.depth += 1
            _auto_counter = 1  # auto() starts at 1 in Python's enum module
            for stmt in node.body:
                if isinstance(stmt, ast.Assign):
                    for t in stmt.targets:
                        if isinstance(t, ast.Name):
                            val_node = stmt.value
                            # auto() call → assign next auto integer
                            if (isinstance(val_node, ast.Call)
                                    and isinstance(val_node.func, ast.Name)
                                    and val_node.func.id == 'auto'
                                    and not val_node.args):
                                val = str(_auto_counter)
                                _auto_counter += 1
                            else:
                                val = self.expr(val_node)
                                # Track explicit integer values for auto() continuity
                                if isinstance(val_node, ast.Constant) and isinstance(val_node.value, int):
                                    _auto_counter = val_node.value + 1
                            self.emit(f"{_safe_ident(t.id)} = {val}")
                elif isinstance(stmt, ast.Expr) and isinstance(stmt.value, ast.Constant):
                    pass  # docstring
            self.depth -= 1
            self.emit(f"end fixed")
            self.emit("")
            return

        parent = None
        if node.bases:
            if len(node.bases) > 1:
                self.warn(node, "multiple inheritance — only first base used")
            base = node.bases[0]
            # For dotted attribute bases (e.g. types.ModuleType, enum.Enum),
            # extract just the attribute name as the parent. If that would create
            # a self-reference (class Foo(x.Foo) → object Foo forks Foo), skip the
            # parent entirely to avoid a circular reference that the UL parser rejects.
            if isinstance(base, ast.Attribute):
                attr_name = _safe_ident(base.attr)
                class_name = _safe_ident(node.name)
                if attr_name != class_name and attr_name != 'object':
                    parent_name = attr_name
                else:
                    parent_name = None  # would be self-reference or 'object' — skip
            else:
                parent_name = self._name(base)
            if parent_name and parent_name != 'object':
                parent = _safe_ident(parent_name)
                # If parent is not a locally-defined object, mark it for stub emission
                if (parent not in self._defined_objects and
                        parent not in self._needed_stubs):
                    self._needed_stubs.append(parent)

        inherits = f" forks {parent}" if parent else ""
        self.emit(f"object {name}{inherits}")
        self.depth += 1
        self.in_class = name

        # Pre-pass: collect self.X fields from __init__ assignments so they can
        # be declared before any methods reference them.
        init_fields = {}  # name -> ul_type, preserving insertion order
        for stmt in node.body:
            if isinstance(stmt, (ast.FunctionDef, ast.AsyncFunctionDef)) and stmt.name == '__init__':
                for s in ast.walk(stmt):
                    if isinstance(s, ast.Assign):
                        for t in s.targets:
                            if (isinstance(t, ast.Attribute) and
                                    isinstance(t.value, ast.Name) and
                                    t.value.id == 'self'):
                                fname = _safe_ident(t.attr)
                                if fname not in init_fields:
                                    init_fields[fname] = self._infer_type(s.value)
                    elif isinstance(s, ast.AnnAssign):
                        if (isinstance(s.target, ast.Attribute) and
                                isinstance(s.target.value, ast.Name) and
                                s.target.value.id == 'self'):
                            fname = _safe_ident(s.target.attr)
                            if fname not in init_fields:
                                init_fields[fname] = self.map_type(s.annotation)
                break  # only scan __init__
        # Collect ALL class-level field names (both annotated and bare assigns)
        # so we can deduplicate against init_fields below and prevent double-emit.
        annotated_fields = set()
        class_assign_fields = set()
        for stmt in node.body:
            if isinstance(stmt, ast.AnnAssign) and isinstance(stmt.target, ast.Name):
                annotated_fields.add(_safe_ident(stmt.target.id))
            elif (isinstance(stmt, ast.Assign) and len(stmt.targets) == 1
                  and isinstance(stmt.targets[0], ast.Name)):
                # Plain class-level assignment: message = "" etc.
                class_assign_fields.add(_safe_ident(stmt.targets[0].id))
        all_class_level_fields = annotated_fields | class_assign_fields

        # Emit class-level annotated fields first
        for stmt in node.body:
            if isinstance(stmt, ast.AnnAssign) and isinstance(stmt.target, ast.Name):
                ftype = self.map_type(stmt.annotation)
                self.emit(f"{ftype} {_safe_ident(stmt.target.id)}")

        # Emit self.X fields inferred from __init__ (not already declared at class level)
        for fname, ftype in init_fields.items():
            if fname not in all_class_level_fields:
                self.emit(f"{ftype} {fname}")

        # Emit methods
        for stmt in node.body:
            if isinstance(stmt, ast.AnnAssign) and isinstance(stmt.target, ast.Name):
                pass  # already emitted above
            elif isinstance(stmt, (ast.FunctionDef, ast.AsyncFunctionDef)):
                self.emit("")
                self.visit(stmt)
            elif isinstance(stmt, ast.Expr) and isinstance(stmt.value, ast.Constant):
                pass  # docstring
            else:
                self.visit(stmt)

        self.in_class = None
        self.depth -= 1
        self.emit(f"end object")
        self.emit("")

    def visit_Return(self, node):
        if node.value:
            self.emit(f"return {self.expr(node.value)}")
        else:
            self.emit("return")

    def _infer_type(self, node):
        """Infer UL type from a Python value AST node."""
        if isinstance(node, ast.Constant):
            if isinstance(node.value, bool): return "bool"
            if isinstance(node.value, int): return "int"
            if isinstance(node.value, float): return "float"
            if isinstance(node.value, str): return "string"
        if isinstance(node, ast.List):
            if node.elts:
                inner = self._infer_type(node.elts[0])
                return f"array {inner}"
            return "array int"
        if isinstance(node, ast.Tuple):
            return "array int"
        if isinstance(node, ast.Call):
            # Can't infer return type of arbitrary call — use auto
            return "auto"
        if isinstance(node, ast.Name):
            if node.id in ('True', 'False'): return "bool"
            if node.id == 'None': return "none"
        # Cannot infer — use auto, let semcheck catch real issues
        return "auto"

    def visit_Assign(self, node):
        if len(node.targets) == 1:
            target_node = node.targets[0]

            # Tuple/list unpacking: [a, b] = [x, y] or (a, b) = expr
            if isinstance(target_node, (ast.Tuple, ast.List)):
                elts = target_node.elts
                # If RHS is also a tuple/list literal of same length, use temp vars for swap
                if isinstance(node.value, (ast.Tuple, ast.List)) and len(node.value.elts) == len(elts):
                    cnt = getattr(self, '_unpack_counter', 0)
                    self._unpack_counter = cnt + 1
                    # Save RHS to temp vars first
                    temps = []
                    for i, rhs_elt in enumerate(node.value.elts):
                        tmp = f"__unpack_{cnt}_{i}"
                        temps.append(tmp)
                        self.emit(f"auto {tmp} = {self.expr(rhs_elt)}")
                    # Assign from temp vars to targets
                    for i, lhs_elt in enumerate(elts):
                        self.emit(f"{self.expr(lhs_elt)} = {temps[i]}")
                else:
                    # General unpacking from a single expression
                    value = self.expr(node.value)
                    for i, elt in enumerate(elts):
                        self.emit(f"{self.expr(elt)} = {value}[{i}]")
                return

            target = self.expr(target_node)

            # Skip dunder assignments
            if isinstance(target_node, ast.Name) and target_node.id.startswith('__') and target_node.id.endswith('__'):
                self.emit(f"// Python module metadata skipped: {target_node.id}")
                return

            # Module-level assignment (depth == 0, not in class)
            if self.depth == 0 and not self.in_class:
                inferred = self._infer_type(node.value)
                is_literal = isinstance(node.value, ast.Constant)
                is_simple_list = isinstance(node.value, (ast.List, ast.Tuple)) and all(
                    isinstance(e, ast.Constant) for e in node.value.elts
                )
                is_any_list = isinstance(node.value, (ast.List, ast.Tuple))
                is_dict = isinstance(node.value, ast.Dict)
                if is_literal:
                    value = self.expr(node.value)
                    self.emit(f"const {inferred} {target} = {value}")
                elif is_simple_list:
                    value = self.expr(node.value)
                    self.emit(f"// TRANSPILER: inferred type for {target} — verify this is correct")
                    # Array/tuple literals are not compile-time in UL — use module-level var, not const
                    self.emit(f"{inferred} {target} = {value}")
                elif is_any_list:
                    # List with non-constant elements (e.g. [VAR1, VAR2, ...]) —
                    # emit as typed array var; init is done in __module_init__.
                    elem_type = "string"  # default; will be overridden for int lists
                    if node.value.elts:
                        elem_type = self._infer_type(node.value.elts[0])
                        if elem_type not in ('int','float','double','string','bool',
                                             'int8','int16','int32','int64',
                                             'uint8','uint16','uint32','uint64'):
                            elem_type = "string"
                    self.emit(f"// TRANSPILER: inferred type for {target} — verify this is correct")
                    self.emit(f"array {elem_type} {target}")
                    # Populate the array in __module_init__
                    value = self.expr(node.value)
                    self.emit_init(f"{target} = {value}")
                elif is_dict:
                    # Fall through to the dict-literal handler below (do NOT return here).
                    pass
                else:
                    # Complex expression: declare typed var at module scope,
                    # perform the assignment inside __module_init__.
                    self.emit(f"// TRANSPILER: module-level variable {target} — verify type is correct")
                    self.emit(f"auto {target} = empty")
                    value = self.expr(node.value)
                    if value != target:
                        self.emit_init(f"{target} = {value}")
                    return
                if not is_dict:
                    return

            # Class-level field assignment (no type annotation) — infer type
            # UL object declarations don't support field initializers (type field = value)
            # so we emit just the field declaration. The default value is noted in a comment.
            if self.in_class and not self._current_func:
                inferred = self._infer_type(node.value)
                value = self.expr(node.value)
                if value and value != target:
                    self.emit(f"// class field default: {target} = {value}")
                self.emit(f"{inferred} {target}")
                return

            # Dict literal: emit "map target = map_new()" directly to avoid
            # the tmp-name self-assignment that expr(Dict) would produce.
            if isinstance(node.value, ast.Dict) and isinstance(target_node, ast.Name):
                at_module_scope = (self.depth == 0 and not self.in_class and not self._current_func)
                self.emit(f"// TRANSPILER: dict values cast to string — may lose type information")
                self.emit(f"map {target} = map_new()")
                for k, v in zip(node.value.keys, node.value.values):
                    if k is not None:
                        stmt = f"map_set({target}, {self.expr(k)}, cast({self.expr(v)}, string))"
                        if at_module_scope:
                            self.emit_init(stmt)
                        else:
                            self.emit(stmt)
            else:
                value = self.expr(node.value)
                # Suppress self-assignment (e.g. x = x when expr() side-emitted "map x = ...")
                if value != target:
                    self.emit(f"{target} = {value}")
        else:
            value = self.expr(node.value)
            for t in node.targets:
                self.emit(f"{self.expr(t)} = {value}")

    def visit_AnnAssign(self, node):
        target = self.expr(node.target)
        ul_type = self.map_type(node.annotation)
        # Skip annotation-only hints where target is an attribute (self.X: Type) — these
        # are just type hints in Python, not executable statements, and UL can't declare
        # 'type self.attr' syntax.
        is_attr_target = isinstance(node.target, ast.Attribute)
        if node.value:
            value = self.expr(node.value)
            if self.depth == 0 and not self.in_class:
                # Only use 'const' if the value is a compile-time literal.
                # Non-literal values (variables, function calls, dicts) must be plain vars.
                is_literal = isinstance(node.value, ast.Constant)
                if is_literal:
                    self.emit(f"const {ul_type} {target} = {value}")
                else:
                    self.emit(f"{ul_type} {target} = {value}")
            else:
                if is_attr_target:
                    # Assignment to self.attr — emit as plain assignment (no type prefix)
                    self.emit(f"{target} = {value}")
                else:
                    self.emit(f"{ul_type} {target} = {value}")
        else:
            if is_attr_target:
                # Annotation-only on attribute — pure type hint, skip it
                return
            if self.depth == 0 and not self.in_class:
                self.emit(f"// {ul_type} {target}  // module-level declaration")
            else:
                self.emit(f"{ul_type} {target}")

    def visit_AugAssign(self, node):
        target = self.expr(node.target)
        value = self.expr(node.value)
        op = self._binop(node.op)
        # UL only supports +=, -=, *=, /= as compound assignment operators.
        # For bitwise/other ops, expand to full form: target = target op value.
        simple_ops = {'+', '-', '*', '/'}
        if op in simple_ops:
            self.emit(f"{target} {op}= {value}")
        else:
            self.emit(f"{target} = {target} {op} {value}")

    def visit_If(self, node):
        # Strip `if __name__ == "__main__":` guard — UL uses main() as entry point
        if (isinstance(node.test, ast.Compare)
                and isinstance(node.test.left, ast.Name)
                and node.test.left.id == '__name__'
                and len(node.test.ops) == 1
                and isinstance(node.test.ops[0], ast.Eq)
                and len(node.test.comparators) == 1
                and isinstance(node.test.comparators[0], ast.Constant)
                and node.test.comparators[0].value == '__main__'):
            # UL auto-calls main() — skip the guard entirely
            return

        # Module-level if blocks (depth == 0, outside class/function) cannot be
        # emitted in UL — only declarations are valid at file scope. These arise
        # from patterns like `if sys.version_info >= (3, 9): X = Y else: X = Z`
        # or `if TYPE_CHECKING: import ...`. Skip entirely with a comment.
        if self.depth == 0 and not self.in_class and not self._current_func:
            cond_src = ast.unparse(node.test) if hasattr(ast, 'unparse') else "..."
            self.emit(f"// TRANSPILER: module-level if ({cond_src}) skipped — UL has no top-level conditionals")
            return

        cond = self.expr(node.test)
        self.emit(f"if {cond}")
        self.depth += 1
        for stmt in node.body:
            self.visit(stmt)
        self.depth -= 1
        if node.orelse:
            if len(node.orelse) == 1 and isinstance(node.orelse[0], ast.If):
                # elif → UL chained "else if" form (parser treats this as one if, one end if closes all)
                elif_node = node.orelse[0]
                elif_cond = self.expr(elif_node.test)
                self.emit(f"else if {elif_cond}")
                self.depth += 1
                for stmt in elif_node.body:
                    self.visit(stmt)
                self.depth -= 1
                # Recurse into further elif/else chains without emitting extra "end if"
                self._visit_elif_chain(elif_node.orelse)
            else:
                self.emit("else")
                self.depth += 1
                for stmt in node.orelse:
                    self.visit(stmt)
                self.depth -= 1
        self.emit("end if")

    def _visit_elif_chain(self, orelse):
        """Emit chained else-if or else blocks without closing end if (caller emits it)."""
        if not orelse:
            return
        if len(orelse) == 1 and isinstance(orelse[0], ast.If):
            elif_node = orelse[0]
            elif_cond = self.expr(elif_node.test)
            self.emit(f"else if {elif_cond}")
            self.depth += 1
            for stmt in elif_node.body:
                self.visit(stmt)
            self.depth -= 1
            self._visit_elif_chain(elif_node.orelse)
        else:
            self.emit("else")
            self.depth += 1
            for stmt in orelse:
                self.visit(stmt)
            self.depth -= 1

    def _at_module_scope(self):
        """Return True when we are at file scope (depth==0, no class, no function)."""
        return self.depth == 0 and not self.in_class and not self._current_func

    def _redirect_to_init(self, visit_fn):
        """Temporarily redirect self.lines/depth into __module_init__ buffer,
        call visit_fn(), then restore.  Used for module-scope loops/calls."""
        saved_lines = self.lines
        saved_depth = self.depth
        # Emit into a temporary list; each line goes into _module_init_stmts with one indent.
        self.lines = []
        self.depth = 1  # one level of indent inside __module_init__
        visit_fn()
        for line in self.lines:
            self._module_init_stmts.append(line)
        self.lines = saved_lines
        self.depth = saved_depth

    def visit_While(self, node):
        if self._at_module_scope():
            # Module-level while loop — route body to __module_init__
            self.emit("// TRANSPILER: module-level while loop moved to __module_init__")
            def _emit_while():
                cond = self.expr(node.test)
                self.emit(f"while {cond}")
                self.depth += 1
                for stmt in node.body:
                    self.visit(stmt)
                self.depth -= 1
                self.emit("end while")
            self._redirect_to_init(_emit_while)
            return
        cond = self.expr(node.test)
        self.emit(f"while {cond}")
        self.depth += 1
        for stmt in node.body:
            self.visit(stmt)
        self.depth -= 1
        self.emit("end while")

    def visit_For(self, node):
        if self._at_module_scope():
            # Module-level for loop — route body to __module_init__
            self.emit("// TRANSPILER: module-level for loop moved to __module_init__")
            def _emit_for():
                self.visit_For(node)
            self._redirect_to_init(_emit_for)
            return
        # Tuple unpacking: for (a, b) in zip(x, y) or for (i, item) in enumerate(items)
        if isinstance(node.target, ast.Tuple) and len(node.target.elts) == 2:
            var_a = self.expr(node.target.elts[0])
            var_b = self.expr(node.target.elts[1])

            # enumerate(iterable)
            if (isinstance(node.iter, ast.Call) and isinstance(node.iter.func, ast.Name)
                    and node.iter.func.id == 'enumerate' and node.iter.args):
                iterable = self.expr(node.iter.args[0])
                self.emit(f"int {var_a} = 0")
                self.emit(f"for each {var_b} in {iterable}")
                self.depth += 1
                for stmt in node.body:
                    self.visit(stmt)
                self.emit(f"{var_a} = {var_a} + 1")
                self.depth -= 1
                self.emit("end for")
                return

            # zip(a, b)
            if (isinstance(node.iter, ast.Call) and isinstance(node.iter.func, ast.Name)
                    and node.iter.func.id == 'zip' and len(node.iter.args) == 2):
                cnt = getattr(self, '_zip_counter', 0)
                self._zip_counter = cnt + 1
                src_a = self.expr(node.iter.args[0])
                src_b = self.expr(node.iter.args[1])
                idx = f"__zip_i{cnt}"
                self.emit(f"int {idx} = 0")
                self.emit(f"while {idx} < {src_a}.len()")
                self.depth += 1
                self.emit(f"auto {var_a} = {src_a}[{idx}]")
                self.emit(f"auto {var_b} = {src_b}[{idx}]")
                for stmt in node.body:
                    self.visit(stmt)
                self.emit(f"{idx} = {idx} + 1")
                self.depth -= 1
                self.emit("end while")
                return

            # General 2-tuple unpacking: for (a, b) in iterable
            # Emit as for each with temp var and destructure
            iter_expr = self.expr(node.iter)
            cnt = getattr(self, '_tup_counter', 0)
            self._tup_counter = cnt + 1
            tmp_pair = f"__pair{cnt}"
            self.emit(f"for each {tmp_pair} in {iter_expr}")
            self.depth += 1
            self.emit(f"auto {var_a} = {tmp_pair}[0]")
            self.emit(f"auto {var_b} = {tmp_pair}[1]")
            for stmt in node.body:
                self.visit(stmt)
            self.depth -= 1
            self.emit("end for")
            return

        # 3+ tuple unpacking — handle with temp variable + indexed access
        if isinstance(node.target, ast.Tuple) and len(node.target.elts) >= 3:
            self.warn(node, f"tuple unpacking with {len(node.target.elts)} elements")
            elts = node.target.elts
            iter_expr = self.expr(node.iter)
            cnt = getattr(self, '_tup_counter', 0)
            self._tup_counter = cnt + 1
            tmp_tup = f"__tup{cnt}"
            self.emit(f"for each {tmp_tup} in {iter_expr}")
            self.depth += 1
            for i, elt in enumerate(elts):
                self.emit(f"auto {self.expr(elt)} = {tmp_tup}[{i}]")
            for stmt in node.body:
                self.visit(stmt)
            self.depth -= 1
            self.emit("end for")
            return

        target = self.expr(node.target)
        iter_expr = self.expr(node.iter)
        self.emit(f"for each {target} in {iter_expr}")
        self.depth += 1
        for stmt in node.body:
            self.visit(stmt)
        self.depth -= 1
        self.emit("end for")

    def visit_Expr(self, node):
        # Docstrings — bare string constants at module/function level → comment
        if isinstance(node.value, ast.Constant) and isinstance(node.value.value, str):
            first_line = node.value.value.split('\n')[0].strip()
            if first_line:
                self.emit(f"// {first_line}")
            return
        if isinstance(node.value, ast.Call):
            func = node.value.func
            func_name = self._name(func) if isinstance(func, (ast.Name, ast.Attribute)) else ""
            if func_name == 'print':
                args = node.value.args
                if args:
                    if len(args) == 1:
                        self.emit(f"print {self.expr(args[0])}")
                    else:
                        # Multi-arg print: concatenate with spaces via cast
                        parts = [f"cast({self.expr(a)}, string)" for a in args]
                        sep = '" "'
                        self.emit(f'print {(" + " + sep + " + ").join(parts)}')
                else:
                    self.emit('print ""')
                return
            # Suppress bare top-level entry point call — UL invokes main() automatically
            if (self.depth == 0 and not self._current_func and not self.in_class
                    and func_name == 'main' and not node.value.args
                    and not node.value.keywords):
                self.emit(f"// UL runtime invokes main() automatically")
                return
        self.emit(self.expr(node.value))

    def visit_Pass(self, node):
        # Inside an object body (class scope, not inside a method) a bare '0' is
        # not valid UL — object bodies only allow field declarations and methods.
        # In function bodies '0' is a valid noop expression.
        if self.in_class and not self._current_func:
            return  # Skip pass inside class body entirely
        self.emit("0")  # UL noop; codegen_python emits 'pass' for empty blocks

    def visit_Break(self, node):
        self.emit("escape")

    def visit_Continue(self, node):
        self.emit("continue")

    def visit_Try(self, node):
        # Module-level try/except (conditional imports etc.) → emit body as comments
        if self.depth == 0 and not self._current_func:
            self.emit("// TRANSPILER: module-level try/except block — try branch emitted")
            for stmt in node.body:
                self.visit(stmt)
            return
        self.emit("try")
        self.depth += 1
        for stmt in node.body:
            self.visit(stmt)
        self.depth -= 1
        for handler in node.handlers:
            if handler.type:
                # Get the exception type name — for dotted names (re.error, click.Abort)
                # use just the last component (the class name).
                raw_type = self._name(handler.type)
                if not raw_type and isinstance(handler.type, ast.Attribute):
                    raw_type = handler.type.attr
                exc_type = _safe_ident(raw_type) if raw_type else ""
            else:
                exc_type = ""
            if handler.name:
                if exc_type:
                    self.emit(f"catch {exc_type} as {_safe_ident(handler.name)}")
                else:
                    self.emit(f"catch as {_safe_ident(handler.name)}")
            elif exc_type:
                self.emit(f"catch {exc_type}")
            else:
                self.emit("catch")
            self.depth += 1
            for stmt in handler.body:
                self.visit(stmt)
            self.depth -= 1
        if node.finalbody:
            self.emit("finally")
            self.depth += 1
            for stmt in node.finalbody:
                self.visit(stmt)
            self.depth -= 1
        self.emit("end try")

    def _exc_name(self, node):
        """Get a UL-safe exception type name (simple IDENT, no dots)."""
        name = self.expr(node)
        # UL throw requires a simple identifier — strip module prefix (e.g. click.FileError → FileError)
        if '.' in name:
            name = name.rsplit('.', 1)[-1]
        return name

    def visit_Raise(self, node):
        if node.exc:
            exc = node.exc
            # raise ExcType(msg) → throw ExcType "msg"
            if isinstance(exc, ast.Call) and exc.args:
                exc_name = self._exc_name(exc.func)
                msg = self.expr(exc.args[0])
                self.emit(f"throw {exc_name} {msg}")
            elif isinstance(exc, ast.Call):
                # raise ExcType() → throw ExcType "error"
                exc_name = self._exc_name(exc.func)
                self.emit(f'throw {exc_name} "error"')
            else:
                # raise ExcType (bare name) → throw ExcType "error"
                exc_name = self._exc_name(exc)
                self.emit(f'throw {exc_name} "error"')
        else:
            # bare raise (re-raise) — no direct UL equivalent, emit as comment
            self.emit('// raise  // re-raise current exception')

    def visit_Import(self, node):
        for alias in node.names:
            # Track the imported name so we can stub it if used as a base class
            imported_as = alias.asname if alias.asname else alias.name.split('.')[-1]
            self._imported_names.add(_safe_ident(imported_as))
            # Record as whole-module import using sentinel symbol "_whole_"
            # (or "_as_ALIAS_" if 'import json as j')
            safe_name = alias.name.replace('.', 'dot')
            as_part = alias.asname if alias.asname else ""
            sentinel = f"_as_{as_part}_" if as_part else "_whole_"
            self._py_imports.append((safe_name, [sentinel]))
            self.warn(node, f"import {alias.name} — convert to UL import syntax")

    def visit_ImportFrom(self, node):
        # Track each imported name so we can stub it if used as a base class
        for alias in node.names:
            imported_as = alias.asname if alias.asname else alias.name
            self._imported_names.add(_safe_ident(imported_as))
        # Record the import for re-emission at the top of the UL output
        module = node.module or ""
        names = [alias.name for alias in node.names if alias.name != '*']
        if names:
            self._py_imports.append((module, names))
        self.warn(node, f"from {module} import ... — convert to UL import syntax")

    def visit_With(self, node):
        for item in node.items:
            ctx = self.expr(item.context_expr)
            if item.optional_vars:
                var = self.expr(item.optional_vars)
                self.emit(f"with {ctx} as {var}")
            else:
                # UL requires 'with expr as var' — synthesize a dummy var name
                cnt = getattr(self, '_with_counter', 0)
                self._with_counter = cnt + 1
                self.emit(f"with {ctx} as __with_{cnt}")
        self.depth += 1
        for stmt in node.body:
            self.visit(stmt)
        self.depth -= 1
        self.emit("end with")

    def visit_Assert(self, node):
        cond = self.expr(node.test)
        self.emit(f"if not {cond}")
        self.depth += 1
        msg = self.expr(node.msg) if node.msg else '"assertion failed"'
        self.emit(f"print {msg}")
        self.emit("exit(1)")
        self.depth -= 1
        self.emit("end if")

    def visit_Global(self, node):
        self.emit(f"// global {', '.join(node.names)}")

    def visit_Nonlocal(self, node):
        self.emit(f"// nonlocal {', '.join(node.names)}")

    def visit_Delete(self, node):
        self.warn(node, "del statement")

    def generic_visit(self, node):
        # Fallback — skip silently for known harmless nodes
        pass


def transpile(source, filename="<source>"):
    """Transpile Python source to UniLogic source. Returns (ul_source, warnings)."""
    tree = ast.parse(source, filename)
    source_lines = source.splitlines()
    converter = Py2UL(source_lines)
    converter.visit(tree)
    # Build prefix: DR directives + stub objects + hoisted nested functions
    prefix_lines = []
    # python_compat directive: relaxes semcheck for transpiled code
    prefix_lines.append("@dr python_compat = true")
    prefix_lines.append("")
    # Emit Python import directives (one @dr per name, encoded as IDENT-safe strings)
    # Format: @dr pyimport_N = MODULEdotNAME__SYMBOLNAME  (dots→dot, separator = __)
    # Multiple @dr with different keys (pyimport_0, pyimport_1, ...) to preserve all.
    idx = 0
    for module, names in converter._py_imports:
        # Encode module name for use as an IDENT-safe string.
        # Order matters: replace '.' first to avoid 'dot' containing double-underscores,
        # then replace '__' (dunder) with 'DUNDER' so modules like __future__ don't
        # accidentally split on the __ separator used to divide module from symbol name.
        safe_module = (module.replace('.', 'dot').replace('__', 'DUNDER')
                       if module else 'relimport')
        for name in names:
            prefix_lines.append(f"@dr pyimport_{idx} = {safe_module}__{name}")
            idx += 1
    if converter._py_imports:
        prefix_lines.append("")
    # Emit stub object declarations for imported base types used in inheritance
    for stub_name in converter._needed_stubs:
        prefix_lines.append(f"// TRANSPILER: stub for imported base type '{stub_name}'")
        prefix_lines.append(f"object {stub_name}")
        prefix_lines.append(f"end object")
        prefix_lines.append("")
    # Detect cross-function shared state: top-level functions that reference names
    # stored by OTHER top-level functions (e.g. callbacks that access a list defined
    # in main()). Promote those names to module-level globals.
    _toplevel_funcs = [n for n in ast.walk(tree)
                       if isinstance(n, (ast.FunctionDef, ast.AsyncFunctionDef))
                       and not any(isinstance(p, (ast.FunctionDef, ast.AsyncFunctionDef,
                                                   ast.ClassDef))
                                   for p in ast.walk(tree) if n in ast.walk(p) and p is not n)]
    # Collect all names Stored by any top-level function
    _all_stored = set()
    for fn in _toplevel_funcs:
        for node2 in ast.walk(fn):
            if isinstance(node2, ast.Name) and isinstance(node2.ctx, ast.Store):
                _all_stored.add(node2.id)
    # Collect names Loaded by top-level functions that are in _all_stored but not
    # locally defined in the same function — these need module-level globals
    _builtins_set = {'True', 'False', 'None', 'print', 'len', 'range', 'str',
                     'int', 'float', 'bool', 'list', 'dict', 'set', 'tuple',
                     'enumerate', 'zip', 'sorted', 'reversed', 'type', 'isinstance',
                     'hasattr', 'getattr', 'setattr', 'append', 'extend', 'repr',
                     'open', 'super', 'next', 'iter', 'map', 'filter', 'any', 'all',
                     'min', 'max', 'abs', 'sum', 'id', 'hash', 'ord', 'chr',
                     'NotImplementedError', 'ValueError', 'KeyError', 'Exception'}
    _cross_func_globals = set()
    for fn in _toplevel_funcs:
        fn_params = set()
        for arg in fn.args.args + fn.args.posonlyargs + fn.args.kwonlyargs:
            fn_params.add(arg.arg)
        if fn.args.vararg:
            fn_params.add(fn.args.vararg.arg)
        fn_stored = set()
        for node2 in ast.walk(fn):
            if isinstance(node2, ast.Name) and isinstance(node2.ctx, ast.Store):
                fn_stored.add(node2.id)
        for node2 in ast.walk(fn):
            if isinstance(node2, ast.Name) and isinstance(node2.ctx, ast.Load):
                name = node2.id
                if (name in _all_stored
                        and name not in fn_params
                        and name not in fn_stored
                        and name not in _builtins_set
                        and name not in converter._imported_names
                        and name not in converter._defined_objects):
                    _cross_func_globals.add(name)
    # Merge with closure_globals
    all_globals = converter._closure_globals | _cross_func_globals
    # Emit module-level declarations for closure-captured variables
    # These promote shared state between hoisted nested functions and their
    # enclosing functions — the only way to approximate closures in UL.
    if all_globals:
        for var in sorted(all_globals):
            prefix_lines.append(f"auto {var} = empty")
        prefix_lines.append("")
    # Insert hoisted nested functions
    if converter._hoisted_funcs:
        for lines, orig_name, mangled in converter._hoisted_funcs:
            prefix_lines.extend(lines)
    converter.lines = prefix_lines + converter.lines
    # Emit __module_init__ function for module-level imperative statements (e.g. map_set calls)
    # These cannot live at UL file scope — only declarations are valid there.
    if converter._module_init_stmts:
        converter.lines.append("")
        converter.lines.append("function __module_init__() returns none")
        converter.lines.extend(converter._module_init_stmts)
        converter.lines.append("end function")
        converter.lines.append("")
    return "\n".join(converter.lines) + "\n", converter.warnings, converter.translated_lines


def main():
    if len(sys.argv) < 2:
        print("Usage: python py2ul.py input.py [-o output.ul]")
        sys.exit(1)

    input_path = sys.argv[1]
    output_path = None
    if "-o" in sys.argv:
        idx = sys.argv.index("-o")
        output_path = sys.argv[idx + 1]
    else:
        output_path = os.path.splitext(input_path)[0] + ".ul"

    source = open(input_path, encoding='utf-8').read()
    ul_source, warnings, line_count = transpile(source, input_path)

    with open(output_path, 'w', encoding='utf-8') as f:
        f.write(ul_source)

    print(f"Translated: {line_count} lines")
    if warnings:
        print(f"Warnings: {len(warnings)} constructs could not be translated (see comments in output)")
        for lineno, reason in warnings:
            print(f"  line {lineno}: {reason}")
    else:
        print("Warnings: 0")
    print(f"Output: {output_path}")


if __name__ == "__main__":
    main()
