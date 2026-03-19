# ulc/codegen_python.py — UniLogic → Python code generator
# Emits Python 3 source with dataclasses for user-defined types.

from ast_nodes import *

INDENT = "    "

UL_TO_PY = {
    "int": "int", "int8": "int", "int16": "int", "int32": "int", "int64": "int",
    "uint8": "int", "uint16": "int", "uint32": "int", "uint64": "int",
    "float": "float", "double": "float",
    "string": "str", "bool": "bool", "none": "None",
}

INT_TYPES = {"int", "int8", "int16", "int32", "int64", "uint8", "uint16", "uint32", "uint64"}


class PyGen:
    def __init__(self, profile=False):
        self.lines = []
        self.depth = 0
        self.type_decls = {}
        self._result_funcs = set()  # names of functions that return ok|error
        self._tmp_counter = 0
        self._current_decl = None
        self._needs_sys = False
        self._profile = profile
        self.var_types = {}  # name -> TypeName, for integer division detection
        self._current_object = None
        self._object_fields = set()
        self._needs_str_helper = False
        self._needs_sort_helper = False
        self._func_decls = {}  # name -> FunctionDecl for default arg injection
        self._needs_exception_class = False  # true if try/throw used
        self._safety_checked = False
        self._needs_time = False
        self._needs_random = False
        self._needs_os = False

    def emit(self, text=""):
        self.lines.append(INDENT * self.depth + text)

    def indent(self): self.depth += 1
    def dedent(self): self.depth -= 1

    def result(self):
        return "\n".join(self.lines) + "\n"

    def py_type(self, type_node):
        return UL_TO_PY.get(type_node.name, type_node.name)

    def py_default(self, type_node):
        n = type_node.name
        if n in ("int","int8","int16","int32","int64",
                  "uint8","uint16","uint32","uint64"):
            return "0"
        if n in ("float", "double"): return "0.0"
        if n == "string": return '""'
        if n == "bool":   return "False"
        return f"{n}()"

    def _scan_for_memtake(self, program):
        """Check if any function body uses memtake()."""
        def _scan_expr(node):
            if isinstance(node, Call) and node.name == "memtake":
                return True
            if isinstance(node, Call):
                return any(_scan_expr(a) for a in node.args)
            if isinstance(node, BinaryOp):
                return _scan_expr(node.left) or _scan_expr(node.right)
            return False
        def _scan_stmts(stmts):
            for s in stmts:
                if isinstance(s, VarDecl) and s.init and _scan_expr(s.init):
                    return True
                if isinstance(s, ExprStmt) and _scan_expr(s.expr):
                    return True
            return False
        for decl in program.decls:
            if isinstance(decl, FunctionDecl):
                if _scan_stmts(decl.body):
                    return True
        return False

    # ── Program ──────────────────────────────────────────────────────────

    def gen_program(self, program):
        # Emit DR directives as comments at the top
        dr_directives = [d for d in program.decls if isinstance(d, DrDirective)]
        dr = {d.key: d.value for d in dr_directives}
        self._safety_checked = dr.get("safety") == "checked"
        if dr_directives:
            for d in dr_directives:
                self.emit(f"# @dr {d.key} = {d.value}")
            self.emit("")

        # Register function declarations for default arg injection
        for decl in program.decls:
            if isinstance(decl, FunctionDecl):
                self._func_decls[decl.name] = decl

        # Pre-scan for memtake usage to decide if str helper is needed
        self._needs_str_helper = self._scan_for_memtake(program)

        self.emit("from dataclasses import dataclass, field")
        if self._profile:
            self.emit("import time, json, atexit")
        self._sys_import_idx = len(self.lines)

        # FFI imports — group by library
        ffi_by_lib = {}
        for decl in program.decls:
            if isinstance(decl, ForeignImport):
                ffi_by_lib.setdefault(decl.lib, []).append(decl.name)
        for lib, names in ffi_by_lib.items():
            if lib == "math":
                unique = sorted(set(names))
                self.emit(f"from math import {', '.join(unique)}")
            elif lib == "stdio":
                # stdio functions like puts → print wrapper
                for name in names:
                    if name == "puts":
                        self.emit(f"def puts(s): print(s); return 0")
                    else:
                        self.emit(f"# FFI: {lib}.{name} not supported in Python target")
            elif lib == "string":
                for name in sorted(set(names)):
                    if name == "strlen":
                        self.emit("def strlen(s): return len(s)")
                    elif name == "strcmp":
                        self.emit("def strcmp(a, b): return (a > b) - (a < b)")
                    elif name == "strncmp":
                        self.emit("def strncmp(a, b, n): return (a[:n] > b[:n]) - (a[:n] < b[:n])")
                    elif name == "strstr":
                        self.emit("def strstr(s, sub): idx = s.find(sub); return s[idx:] if idx >= 0 else None")
                    elif name == "strcpy":
                        self.emit("def strcpy(dest, src):")
                        self.indent()
                        self.emit("enc = src.encode() if isinstance(src, str) else src")
                        self.emit("dest[:len(enc)] = enc; dest[len(enc)] = 0")
                        self.emit("return dest")
                        self.dedent()
                    elif name == "strcat":
                        self.emit("def strcat(dest, src):")
                        self.indent()
                        self.emit("enc = src.encode() if isinstance(src, str) else src")
                        self.emit("try: end = dest.index(0)")
                        self.emit("except ValueError: end = len(dest)")
                        self.emit("dest[end:end+len(enc)] = enc; dest[end+len(enc)] = 0")
                        self.emit("return dest")
                        self.dedent()
                    elif name == "str_char_at":
                        self.emit("def str_char_at(s, i): return s[i] if 0 <= i < len(s) else ''")
                    elif name == "str_substr":
                        self.emit("def str_substr(s, start, length): return s[start:start+length]")
                    elif name == "str_index_of":
                        self.emit("def str_index_of(s, sub): return s.find(sub)")
                    else:
                        self.emit(f"# FFI: string.{name} not supported in Python target")
            elif lib == "stdlib":
                for name in sorted(set(names)):
                    if name == "memtake":
                        self.emit("def memtake(n): return bytearray(n)")
                        self._needs_str_helper = True
                    elif name == "memgive":
                        self.emit("def memgive(p): pass")
                    else:
                        self.emit(f"# FFI: stdlib.{name} not supported in Python target")
            else:
                self.emit(f"# FFI: import \"{lib}\" not supported in Python target")

        self.emit("")

        if self._needs_str_helper:
            self.emit("def __ul_to_str(v):")
            self.indent()
            self.emit("if isinstance(v, (bytearray, bytes)):")
            self.indent()
            self.emit("idx = v.find(0)")
            self.emit("return v[:idx].decode() if idx >= 0 else v.decode()")
            self.dedent()
            self.emit("return v")
            self.dedent()
            self.emit("")

        # Sort helper — sort first n elements in-place
        self.emit("def __ul_sort(arr, n): arr[:n] = sorted(arr[:n])")
        self.emit("")

        # Arena simulation for Python target
        if dr.get("memory") == "arena":
            self.emit("# @dr memory = arena -- arena allocation simulated")
            self.emit("# Python uses reference counting; this mode disables explicit del calls")
            self.emit("# and relies on scope-based cleanup via context managers")
            self.emit("__ul_arena_refs = []")
            self.emit("")
            self.emit("def __ul_arena_alloc(obj):")
            self.emit("    __ul_arena_refs.append(obj)")
            self.emit("    return obj")
            self.emit("")
            self.emit("def __ul_arena_free():")
            self.emit("    __ul_arena_refs.clear()")
            self.emit("")
            self.emit("import atexit")
            self.emit("atexit.register(__ul_arena_free)")
            self.emit("")

        # Exception class for try/catch/throw
        self.emit("class __ul_Exception(Exception):")
        self.emit("    def __init__(self, type_name, message):")
        self.emit("        self.type_name = type_name")
        self.emit("        self.message = message")
        self.emit("        super().__init__(message)")
        self.emit("")

        if self._profile:
            self.emit("_ul_profile_log = []")
            self.emit("")
            self.emit("def __ul_profile(fn):")
            self.indent()
            self.emit("def wrapper(*args, **kwargs):")
            self.indent()
            self.emit('_ul_profile_log.append({"event": "enter", "func": fn.__name__, "time_ns": time.time_ns()})')
            self.emit("_start = time.time_ns()")
            self.emit("_result = fn(*args, **kwargs)")
            self.emit("_elapsed = time.time_ns() - _start")
            self.emit('_ul_profile_log.append({"event": "exit", "func": fn.__name__, "time_ns": time.time_ns(), "elapsed_ns": _elapsed})')
            self.emit("return _result")
            self.dedent()
            self.emit("return wrapper")
            self.dedent()
            self.emit("")
            self.emit("def __ul_profile_dump():")
            self.indent()
            self.emit('with open("ul_profile.log", "w") as f:')
            self.indent()
            self.emit("for entry in _ul_profile_log:")
            self.indent()
            self.emit("f.write(json.dumps(entry) + chr(10))")
            self.dedent()
            self.dedent()
            self.dedent()
            self.emit("")
            self.emit("atexit.register(__ul_profile_dump)")
            self.emit("")
            self.emit("def __ul_gc_collect():")
            self.indent()
            self.emit('_ul_profile_log.append({"event": "gc_collect", "time_ns": time.time_ns()})')
            self.dedent()
            self.emit("")
            self.emit("def __ul_refcount(ptr, delta):")
            self.indent()
            self.emit('_ul_profile_log.append({"event": "refcount", "ptr": str(id(ptr)), "delta": delta, "time_ns": time.time_ns()})')
            self.dedent()
            self.emit("")
            self.emit("def __ul_arena_reset():")
            self.indent()
            self.emit('_ul_profile_log.append({"event": "arena_reset", "time_ns": time.time_ns()})')
            self.dedent()
            self.emit("")

        for decl in program.decls:
            if isinstance(decl, TypeDecl):
                self.type_decls[decl.name] = decl
                self.gen_type_decl(decl)
            elif isinstance(decl, ObjectDecl):
                self.type_decls[decl.name] = decl
                self.gen_object_decl(decl)

        for decl in program.decls:
            if isinstance(decl, FunctionDecl) and decl.returns_result:
                self._result_funcs.add(decl.name)

        # Emit enumerations as classes with class variables
        self._enums = {}  # name -> dict of member -> value
        for decl in program.decls:
            if isinstance(decl, EnumDecl):
                self._enums[decl.name] = {name: val for name, val in decl.members}
                self.emit(f"class {decl.name}:")
                self.indent()
                for mname, mval in decl.members:
                    self.emit(f"{mname} = {self.gen_expr(mval)}")
                self.dedent()
                self.emit("")

        # Emit module-level constants
        for decl in program.decls:
            if isinstance(decl, ConstDecl):
                self.emit(f"{decl.name} = {self.gen_expr(decl.value)}")

        # Emit module-level variable declarations
        self._module_vars = set()
        for decl in program.decls:
            if isinstance(decl, VarDecl):
                self._module_vars.add(decl.name)
        for decl in program.decls:
            if isinstance(decl, VarDecl):
                if decl.type_.is_array and isinstance(decl.init, ArrayLiteral):
                    elems = ", ".join(self.gen_expr(e) for e in decl.init.elements)
                    self.emit(f"{decl.name} = [{elems}]")
                elif decl.init is not None:
                    self.emit(f"{decl.name} = {self.gen_expr(decl.init)}")
                else:
                    defaults = {"int": "0", "float": "0.0", "string": '""', "bool": "False"}
                    self.emit(f"{decl.name} = {defaults.get(decl.type_.name, 'None')}")
        self.emit("")

        for decl in program.decls:
            if isinstance(decl, (ForeignImport, TypeDecl, ObjectDecl, ConstDecl, DrDirective, NormDirective, AsmBlock, EnumDecl, VarDecl)):
                continue
            self.gen_function(decl)

        self.emit("")
        self.emit('if __name__ == "__main__":')
        self.indent()
        self.emit("main()")
        self.dedent()

        if self._needs_sys:
            self.lines.insert(self._sys_import_idx, "import sys")
        if self._needs_time:
            self.lines.insert(self._sys_import_idx, "import time")
        if self._needs_random:
            self.lines.insert(self._sys_import_idx, "import random")
        if self._needs_os:
            self.lines.insert(self._sys_import_idx, "import os")

        return self.result()

    def gen_type_decl(self, decl):
        self.emit("@dataclass")
        if decl.parent:
            self.emit(f"class {decl.name}({decl.parent}):")
        else:
            self.emit(f"class {decl.name}:")
        self.indent()
        if not decl.fields:
            self.emit("pass")
        else:
            for f in decl.fields:
                pt = self.py_type(f.type_)
                default = self.py_default(f.type_)
                # Mutable defaults (other dataclasses) need field(default_factory=...)
                if f.type_.name in self.type_decls:
                    self.emit(f"{f.name}: {pt} = field(default_factory={pt})")
                else:
                    self.emit(f"{f.name}: {pt} = {default}")
        self.dedent()
        self.emit("")

    def gen_object_decl(self, decl):
        if decl.parent:
            self.emit(f"class {decl.name}({decl.parent}):")
        else:
            self.emit(f"class {decl.name}:")
        self.indent()
        # __init__ with fields
        all_fields = []
        if decl.parent and decl.parent in self.type_decls:
            parent = self.type_decls[decl.parent]
            if hasattr(parent, 'fields'):
                all_fields.extend(parent.fields)
        all_fields.extend(decl.fields)
        params = ", ".join(f"{f.name}={self.py_default(f.type_)}" for f in all_fields)
        self.emit(f"def __init__(self{', ' + params if params else ''}):")
        self.indent()
        if decl.parent:
            parent_fields = []
            parent_decl = self.type_decls.get(decl.parent)
            if parent_decl and hasattr(parent_decl, 'fields'):
                parent_fields = parent_decl.fields
            p_args = ", ".join(f.name for f in parent_fields)
            self.emit(f"super().__init__({p_args})")
        for f in decl.fields:
            self.emit(f"self.{f.name} = {f.name}")
        if not all_fields and not decl.parent:
            self.emit("pass")
        self.dedent()
        # Methods
        all_field_names = set(f.name for f in all_fields)
        for method in decl.methods:
            self._current_object = decl.name
            self._object_fields = all_field_names
            params = ", ".join(p.name for p in method.params)
            self.emit(f"def {method.name}(self{', ' + params if params else ''}):")
            self.indent()
            for stmt in method.body:
                self.gen_stmt(stmt)
            if not method.body:
                self.emit("pass")
            self.dedent()
            self._current_object = None
            self._object_fields = set()
        self.dedent()
        self.emit("")

    # ── Function ─────────────────────────────────────────────────────────

    def _is_main_void(self, decl):
        """Check if this is main() declared with returns none or no return type."""
        return (decl.name == "main" and not decl.is_generator
                and (decl.return_type is None or decl.return_type.name == "none"))

    def gen_function(self, decl):
        self._current_decl = decl
        for p in decl.params:
            self.var_types[p.name] = p.type_
        param_parts = []
        for p in decl.params:
            if p.default is not None:
                param_parts.append(f"{p.name}={self.gen_expr(p.default)}")
            else:
                param_parts.append(p.name)
        if getattr(decl, 'variadic', False):
            param_parts.append("*_va_args")
        params = ", ".join(param_parts)
        if self._profile:
            self.emit("@__ul_profile")
        self.emit(f"def {decl.name}({params}):")
        self.indent()
        # Emit global declarations for module-level vars used in this function
        if hasattr(self, '_module_vars') and self._module_vars:
            used = self._scan_used_globals(decl.body)
            if used:
                self.emit(f"global {', '.join(sorted(used))}")
        # Emit nested functions as nested defs
        for nf in getattr(decl, 'nested_functions', []):
            self.gen_function(nf)
        if not decl.body:
            self.emit("pass")
        else:
            for stmt in decl.body:
                self.gen_stmt(stmt)
        # void main → implicit return 0
        if self._is_main_void(decl):
            self.emit("return 0")
        self.dedent()
        self.emit("")
        self._current_decl = None

    def _scan_used_globals(self, stmts):
        """Find module-level variable names referenced in a statement list."""
        found = set()
        def _scan_expr(node):
            if isinstance(node, Identifier) and node.name in self._module_vars:
                found.add(node.name)
            elif isinstance(node, BinaryOp):
                _scan_expr(node.left); _scan_expr(node.right)
            elif isinstance(node, UnaryOp):
                _scan_expr(node.operand)
            elif isinstance(node, Call):
                for a in node.args: _scan_expr(a)
            elif isinstance(node, Index):
                _scan_expr(node.target); _scan_expr(node.index)
            elif isinstance(node, FieldAccess):
                _scan_expr(node.target)
            elif isinstance(node, MethodCall):
                _scan_expr(node.target)
                for a in node.args: _scan_expr(a)
        def _scan_stmts(stmts):
            for s in stmts:
                if isinstance(s, VarDecl) and s.init:
                    _scan_expr(s.init)
                elif isinstance(s, Assign):
                    _scan_expr(s.target); _scan_expr(s.value)
                elif isinstance(s, Print):
                    _scan_expr(s.value)
                elif isinstance(s, Return) and s.value:
                    _scan_expr(s.value)
                elif isinstance(s, ExprStmt):
                    _scan_expr(s.expr)
                elif isinstance(s, If):
                    _scan_expr(s.condition)
                    _scan_stmts(s.then_body); _scan_stmts(s.else_body)
                elif isinstance(s, While):
                    _scan_expr(s.condition); _scan_stmts(s.body)
                elif isinstance(s, For):
                    _scan_expr(s.iterable); _scan_stmts(s.body)
                elif isinstance(s, TryStmt):
                    _scan_stmts(s.body)
                    for h in s.handlers: _scan_stmts(h.body)
                    _scan_stmts(s.finally_body)
        _scan_stmts(stmts)
        return found

    # ── Statements ───────────────────────────────────────────────────────

    def gen_stmt(self, node):
        t = type(node)

        if t == VarDecl:
            self.var_types[node.name] = node.type_
            if node.type_.is_list:
                # list T → name = [] or name = [elems]
                if node.init is not None:
                    self.emit(f"{node.name} = {self.gen_expr(node.init)}")
                else:
                    self.emit(f"{node.name} = []")
            elif node.init is not None:
                init_expr = self.gen_expr(node.init)
                # int c = string[i] → c = ord(string[i]) — only for plain string, not array string
                if (node.type_.name in INT_TYPES
                        and isinstance(node.init, Index)
                        and isinstance(node.init.target, Identifier)):
                    target_type = self.var_types.get(node.init.target.name)
                    if target_type and target_type.name == "string" and not getattr(target_type, 'is_array', False):
                        init_expr = f"ord({init_expr})"
                self.emit(f"{node.name} = {init_expr}")
            elif node.type_.name in self.type_decls:
                self.emit(f"{node.name} = {node.type_.name}()")
            else:
                self.emit(f"{node.name} = {self.py_default(node.type_)}")

        elif t == Assign:
            target = self.gen_expr(node.target)
            value = self.gen_expr(node.value)
            # buffer[i] = string[j] → wrap with ord() — only for plain string, not array string
            if (isinstance(node.value, Index)
                    and isinstance(node.value.target, Identifier)):
                vtype = self.var_types.get(node.value.target.name)
                if vtype and vtype.name == "string" and not getattr(vtype, 'is_array', False):
                    value = f"ord({value})"
            self.emit(f"{target} {node.op} {value}")

        elif t == If:
            self.emit(f"if {self.gen_expr(node.condition)}:")
            self.indent()
            for s in node.then_body: self.gen_stmt(s)
            self.dedent()
            if node.else_body:
                self.emit("else:")
                self.indent()
                for s in node.else_body: self.gen_stmt(s)
                self.dedent()

        elif t == While:
            self.emit(f"while {self.gen_expr(node.condition)}:")
            self.indent()
            for s in node.body: self.gen_stmt(s)
            self.dedent()

        elif t == DoWhile:
            self.emit("while True:")
            self.indent()
            for s in node.body: self.gen_stmt(s)
            self.emit(f"if not ({self.gen_expr(node.condition)}):")
            self.indent()
            self.emit("break")
            self.dedent()
            self.dedent()

        elif t == For:
            if getattr(node, 'parallel', False):
                self.emit(f"# parallel for -- using multiprocessing.Pool")
                self.emit(f"import multiprocessing")
                func_name = f"_parallel_body_{self._tmp_counter}"
                self._tmp_counter += 1
                self.emit(f"def {func_name}({node.var}):")
                self.indent()
                for s in node.body: self.gen_stmt(s)
                self.dedent()
                self.emit(f"with multiprocessing.Pool() as _pool:")
                self.indent()
                self.emit(f"_pool.map({func_name}, {self.gen_expr(node.iterable)})")
                self.dedent()
            else:
                self.emit(f"for {node.var} in {self.gen_expr(node.iterable)}:")
                self.indent()
                for s in node.body: self.gen_stmt(s)
                self.dedent()

        elif t == CFor:
            # C-style for → emit init, then while loop with update
            self.gen_stmt(node.init)
            self.emit(f"while {self.gen_expr(node.condition)}:")
            self.indent()
            for s in node.body: self.gen_stmt(s)
            if node.update:
                # Unwrap ExprStmt for PostIncrement/PostDecrement
                upd = node.update
                if isinstance(upd, ExprStmt) and isinstance(upd.expr, (PostIncrement, PostDecrement)):
                    self.gen_stmt(upd.expr)
                else:
                    self.gen_stmt(upd)
            self.dedent()

        elif t == Return:
            if node.value is not None and isinstance(node.value, OkResult):
                self.emit(f"return (True, {self.gen_expr(node.value.value)})")
            elif node.value is not None and isinstance(node.value, ErrorResult):
                self.emit(f"return (False, {self.gen_expr(node.value.value)})")
            elif node.value is not None:
                val_expr = self.gen_expr(node.value)
                if (self._needs_str_helper
                        and self._current_decl
                        and self._current_decl.return_type
                        and self._current_decl.return_type.name == "string"):
                    val_expr = f"__ul_to_str({val_expr})"
                self.emit(f"return {val_expr}")
            else:
                self.emit("return")

        elif t == Print:
            self.emit(f"print({self.gen_expr(node.value)})")

        elif t == ExprStmt:
            self.emit(self.gen_expr(node.expr))

        elif t == Escape:
            self.emit("break")

        elif t == Continue:
            self.emit("continue")

        elif t == Match:
            self.emit(f"match {self.gen_expr(node.subject)}:")
            self.indent()
            for case in node.cases:
                if case.value is None:
                    self.emit("case _:")
                else:
                    self.emit(f"case {self.gen_expr(case.value)}:")
                self.indent()
                if not case.body:
                    self.emit("pass")
                else:
                    for s in case.body: self.gen_stmt(s)
                self.dedent()
            self.dedent()

        elif t == Yield:
            self.emit(f"yield {self.gen_expr(node.value)}")

        elif t == PostIncrement:
            self.emit(f"{self.gen_expr(node.operand)} += 1")

        elif t == PostDecrement:
            self.emit(f"{self.gen_expr(node.operand)} -= 1")

        elif t == TupleDestructure:
            names = ", ".join(name for _, name in node.targets)
            self.emit(f"{names} = {self.gen_expr(node.value)}")
            for t_, name in node.targets:
                self.var_types[name] = t_

        elif t == KillswitchStmt:
            if self._safety_checked:
                cond = self.gen_expr(node.condition)
                self.emit(f"if not ({cond}):")
                self.indent()
                self._needs_sys = True
                self.emit(f'print(f"killswitch failed at line {node.line}", file=sys.stderr)')
                self.emit("sys.exit(1)")
                self.dedent()

        elif t == PortalDecl:
            self.emit(f"# portal {node.name}  (goto not supported in Python - restructure as loop)")

        elif t == GotoStmt:
            self.emit(f"pass  # goto {node.target}  (goto not supported in Python - restructure as loop)")

        elif t == TryStmt:
            self.emit("try:")
            self.indent()
            if node.body:
                for s in node.body:
                    self.gen_stmt(s)
            else:
                self.emit("pass")
            self.dedent()
            if node.handlers:
                # Catch all as __ul_Exception, then dispatch by type
                self.emit("except Exception as __ex:")
                self.indent()
                self.emit("__ex_type = __ex.type_name if isinstance(__ex, __ul_Exception) else type(__ex).__name__")
                self.emit("__ex_msg = __ex.message if isinstance(__ex, __ul_Exception) else str(__ex)")
                first = True
                for handler in node.handlers:
                    if handler.exception_type:
                        kw = "if" if first else "elif"
                        self.emit(f'{kw} __ex_type == "{handler.exception_type}":')
                    else:
                        if first:
                            self.emit("if True:")
                        else:
                            self.emit("else:")
                    self.indent()
                    if handler.binding_name:
                        self.emit(f"{handler.binding_name} = __ex_msg")
                    if handler.body:
                        for s in handler.body:
                            self.gen_stmt(s)
                    else:
                        self.emit("pass")
                    self.dedent()
                    first = False
                has_catchall = any(h.exception_type is None for h in node.handlers)
                if not has_catchall:
                    self.emit("else:")
                    self.indent()
                    self.emit("raise")
                    self.dedent()
                self.dedent()
            if node.finally_body:
                self.emit("finally:")
                self.indent()
                for s in node.finally_body:
                    self.gen_stmt(s)
                self.dedent()

        elif t == ThrowStmt:
            self.emit(f'raise __ul_Exception("{node.exception_type}", {self.gen_expr(node.message)})')

        elif t == WithStmt:
            self.emit(f"{node.binding} = {self.gen_expr(node.expr)}")
            self.emit("try:")
            self.indent()
            for s in node.body: self.gen_stmt(s)
            if not node.body:
                self.emit("pass")
            self.dedent()
            self.emit("finally:")
            self.indent()
            self.emit(f"if hasattr({node.binding}, 'close'): {node.binding}.close()")
            self.dedent()

        elif t == NormDirective:
            self.emit(f"# @norm {node.level}")

    # ── Expressions ──────────────────────────────────────────────────────

    def _is_int_expr(self, node):
        """Check if an expression resolves to an integer type."""
        t = type(node)
        if t == IntLiteral:    return True
        if t == BoolLiteral:   return True
        if t == FloatLiteral:  return False
        if t == StringLiteral: return False
        if t == Identifier:
            vtype = self.var_types.get(node.name)
            return vtype is not None and vtype.name in INT_TYPES
        if t == BinaryOp:
            return self._is_int_expr(node.left) and self._is_int_expr(node.right)
        if t == UnaryOp and node.op == "-":
            return self._is_int_expr(node.operand)
        if t == Cast:
            return node.target_type.name in INT_TYPES
        return False

    def gen_expr(self, node):
        t = type(node)

        if t == IntLiteral:    return str(node.value)
        if t == FloatLiteral:  return repr(node.value)
        if t == StringLiteral: return repr(node.value)
        if t == BoolLiteral:   return "True" if node.value else "False"
        if t == EmptyLiteral:  return "None"

        if t == Identifier:
            if self._current_object and node.name in self._object_fields:
                return f"self.{node.name}"
            return node.name

        if t == BinaryOp:
            if node.op == "|>":
                return f"{self.gen_expr(node.right)}({self.gen_expr(node.left)})"
            if node.op == "in":
                return f"({self.gen_expr(node.left)} in {self.gen_expr(node.right)})"
            op = {"and": "and", "or": "or",
                  "bit_and": "&", "bit_or": "|", "bit_xor": "^",
                  "bit_left": "<<", "bit_right": ">>"}.get(node.op, node.op)
            if op == "/" and self._is_int_expr(node.left) and self._is_int_expr(node.right):
                op = "//"
            return f"({self.gen_expr(node.left)} {op} {self.gen_expr(node.right)})"

        if t == MethodCall:
            # String method sugar: s.len() → len(s), s.method() → str_method(s)
            if isinstance(node.target, Identifier):
                vtype = self.var_types.get(node.target.name)
                if vtype and vtype.name == "string":
                    target = self.gen_expr(node.target)
                    if node.method == "len":
                        return f"len({target})"
                    args = ", ".join(self.gen_expr(a) for a in node.args)
                    all_args = f"{target}, {args}" if args else target
                    return f"str_{node.method}({all_args})"
                if vtype and (vtype.is_list or vtype.is_array):
                    target = self.gen_expr(node.target)
                    if node.method in ("append", "push"):
                        val = self.gen_expr(node.args[0])
                        return f"{target}.append({val})"
                    if node.method == "pop":
                        return f"{target}.pop()"
                    if node.method == "drop":
                        if node.args:
                            idx = self.gen_expr(node.args[0])
                            return f"{target}.pop({idx})"
                        return f"{target}.pop()"
                    if node.method == "insert":
                        idx = self.gen_expr(node.args[0])
                        val = self.gen_expr(node.args[1])
                        return f"{target}.insert({idx}, {val})"
                    if node.method == "remove":
                        val = self.gen_expr(node.args[0])
                        return f"{target}.remove({val})"
                    if node.method == "clear":
                        return f"{target}.clear()"
                    if node.method == "sort":
                        return f"{target}.sort()"
                    if node.method == "reverse":
                        return f"{target}.reverse()"
                    if node.method == "contains":
                        val = self.gen_expr(node.args[0])
                        return f"({val} in {target})"
                    if node.method == "len":
                        return f"len({target})"
                    if node.method == "sum":
                        return f"sum({target})"
                    if node.method == "min":
                        return f"min({target})"
                    if node.method == "max":
                        return f"max({target})"
                    # Fallback for arrays: array_method(target, len, ...)
                    if vtype.is_array:
                        extra_args = ", ".join(self.gen_expr(a) for a in node.args)
                        all_args = f"{target}, len({target}), {extra_args}" if extra_args else f"{target}, len({target})"
                        return f"array_{node.method}({all_args})"
                    args = ", ".join(self.gen_expr(a) for a in node.args)
                    return f"{target}.{node.method}({args})"
            target = self.gen_expr(node.target)
            args = ", ".join(self.gen_expr(a) for a in node.args)
            return f"{target}.{node.method}({args})"

        if t == StructLiteral:
            fields = ", ".join(f"{fname}={self.gen_expr(fval)}" for fname, fval in node.fields)
            return f"{node.type_name}({fields})"

        if t == UnaryOp:
            if node.op == "not":     return f"(not {self.gen_expr(node.operand)})"
            if node.op == "-":       return f"(-{self.gen_expr(node.operand)})"
            if node.op == "bit_not": return f"(~{self.gen_expr(node.operand)})"

        if t == Cast:
            inner = node.expr
            if isinstance(inner, Call) and inner.name == "change" and len(inner.args) == 1:
                inner = inner.args[0]
            target = self.py_type(node.target_type)
            return f"{target}({self.gen_expr(inner)})"

        if t == Call:
            # Fill in default parameter values for missing args
            call_args = list(node.args)
            fn_decl = self._func_decls.get(node.name)
            if fn_decl and len(call_args) < len(fn_decl.params):
                for i in range(len(call_args), len(fn_decl.params)):
                    if fn_decl.params[i].default is not None:
                        call_args.append(fn_decl.params[i].default)
            args = ", ".join(self.gen_expr(a) for a in call_args)
            if node.name == "size":
                return f"len({args})"
            if node.name == "exit":
                self._needs_sys = True
                return f"sys.exit({args})"
            if node.name == "memtake":
                return f"bytearray({args})"
            if node.name == "memgive":
                return "None"
            if node.name == "char_code":
                s_expr = self.gen_expr(call_args[0])
                i_expr = self.gen_expr(call_args[1])
                return f"ord({s_expr}[{i_expr}])"
            if node.name == "sort":
                self._needs_sort_helper = True
                return f"__ul_sort({args})"
            # System functions (Task 11)
            if node.name == "time":
                self._needs_time = True
                return "int(time.time())"
            if node.name == "clock":
                self._needs_time = True
                return "time.time_ns()"
            if node.name == "sleep":
                self._needs_time = True
                return f"time.sleep({self.gen_expr(call_args[0])} / 1000)"
            if node.name == "random":
                self._needs_random = True
                return "random.random()"
            if node.name == "random_int":
                self._needs_random = True
                lo = self.gen_expr(call_args[0])
                hi = self.gen_expr(call_args[1])
                return f"random.randint({lo}, {hi})"
            if node.name == "random_seed":
                self._needs_random = True
                return f"random.seed({self.gen_expr(call_args[0])})"
            if node.name == "userinput":
                self._needs_sys = True
                return "sys.argv"
            if node.name == "vault":
                self._needs_os = True
                return f"os.environ.get({self.gen_expr(call_args[0])}, '')"
            if node.name == "abort":
                self._needs_sys = True
                return "sys.exit(1)"
            if node.name == "prompt":
                return f"input({self.gen_expr(call_args[0])})"
            # Python builtin equivalents (Task 15)
            if node.name == "zip":
                return f"list(zip({self.gen_expr(call_args[0])}, {self.gen_expr(call_args[1])}))"
            if node.name == "enumerate":
                return f"list(enumerate({self.gen_expr(call_args[0])}))"
            if node.name == "map_fn":
                return f"list(map({self.gen_expr(call_args[0])}, {self.gen_expr(call_args[1])}))"
            if node.name == "filter_fn":
                return f"list(filter({self.gen_expr(call_args[0])}, {self.gen_expr(call_args[1])}))"
            return f"{node.name}({args})"

        if t == Index:
            return f"{self.gen_expr(node.target)}[{self.gen_expr(node.index)}]"

        if t == SliceExpr:
            return f"{self.gen_expr(node.target)}[{self.gen_expr(node.start)}:{self.gen_expr(node.end)}]"

        if t == FieldAccess:
            return f"{self.gen_expr(node.target)}.{node.field}"

        if t == TupleLiteral:
            elems = ", ".join(self.gen_expr(e) for e in node.elements)
            return f"({elems})"

        if t == ArrayLiteral:
            elems = ", ".join(self.gen_expr(e) for e in node.elements)
            return f"[{elems}]"

        if t == ArrayComprehension:
            return f"[{self.gen_expr(node.expr)} for {node.var} in {self.gen_expr(node.iterable)}]"

        if t == OkResult:
            return f"(True, {self.gen_expr(node.value)})"

        if t == ErrorResult:
            return f"(False, {self.gen_expr(node.value)})"

        if t == ResultPropagation:
            tmp = f"_r{self._tmp_counter}"
            self._tmp_counter += 1
            call_expr = self.gen_expr(node.expr)
            self.emit(f"{tmp} = {call_expr}")
            self.emit(f"if not {tmp}[0]:")
            self.indent()
            if self._current_decl and self._current_decl.returns_result:
                self.emit(f"return {tmp}")
            else:
                self._needs_sys = True
                self.emit(f"print(\"error: \" + str({tmp}[1]), file=sys.stderr)")
                self.emit(f"sys.exit(1)")
            self.dedent()
            return f"{tmp}[1]"

        return "None"


def generate(program, profile=False):
    return PyGen(profile=profile).gen_program(program)
