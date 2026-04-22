# ulc/codegen_js.py — UniLogic → JavaScript code generator
# Emits plain JS with factory functions for user-defined types.

from ast_nodes import *

INDENT = "    "

JS_DEFAULT = {
    "int": "0", "int8": "0", "int16": "0", "int32": "0", "int64": "0",
    "uint8": "0", "uint16": "0", "uint32": "0", "uint64": "0",
    "float": "0.0", "double": "0.0",
    "string": '""', "bool": "false", "none": "null",
}


class JSGen:
    def __init__(self, profile=False):
        self.lines = []
        self.depth = 0
        self.type_decls = {}
        self._result_funcs = set()
        self._tmp_counter = 0
        self._current_decl = None
        self._profile = profile
        self._needs_range = False
        self._current_object = None
        self._object_fields = set()
        self._safety_checked = False
        self._array_sizes = {}
        self.var_types = {}
        self._needs_str_helper = False
        self._func_decls = {}  # name -> FunctionDecl for default arg injection
        self._needs_sleep = False
        self._needs_prompt = False

    def emit(self, text=""):
        self.lines.append(INDENT * self.depth + text)

    def indent(self): self.depth += 1
    def dedent(self): self.depth -= 1

    def result(self):
        return "\n".join(self.lines) + "\n"

    def js_default(self, type_node):
        if type_node.name in self.type_decls:
            decl = self.type_decls[type_node.name]
            if isinstance(decl, ObjectDecl):
                return f"new {type_node.name}()"
            return f"create{type_node.name}()"
        return JS_DEFAULT.get(type_node.name, "null")

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

    # Functions that exist directly on Math.*
    _MATH_DIRECT = {
        "sqrt", "abs", "ceil", "floor", "round", "log", "log2", "log10",
        "sin", "cos", "tan", "asin", "acos", "atan", "atan2",
        "exp", "pow", "min", "max", "trunc", "sign", "cbrt", "hypot",
    }

    # Functions that need a custom JS shim
    _MATH_SHIMS = {
        "fmod": "(x, y) => x % y",
        "fabs": "Math.abs",
    }

    def gen_program(self, program):
        # Emit DR directives as comments at the top
        dr_directives = [d for d in program.decls if isinstance(d, DrDirective)]
        dr = {}
        if dr_directives:
            for d in dr_directives:
                self.emit(f"// @dr {d.key} = {d.value}")
                dr[d.key] = d.value
            self.emit("")
        self._safety_checked = dr.get("safety") == "checked"

        if self._safety_checked:
            self.emit("function __ul_checked_access(arr, idx, name, line) {")
            self.indent()
            self.emit("if (idx < 0 || idx >= arr.length) {")
            self.indent()
            self.emit('process.stderr.write(`bounds check error: ${name}[${idx}] out of range (size ${arr.length}) at line ${line}\\n`);')
            self.emit("process.exit(1);")
            self.dedent()
            self.emit("}")
            self.emit("return arr[idx];")
            self.dedent()
            self.emit("}")
            self.emit("")

        # Register function declarations for default arg injection
        for decl in program.decls:
            if isinstance(decl, FunctionDecl):
                self._func_decls[decl.name] = decl

        # Pre-scan for memtake to emit string helper
        self._needs_str_helper = self._scan_for_memtake(program)
        if self._needs_str_helper:
            self.emit("function __ul_to_str(v) {")
            self.indent()
            self.emit("if (v instanceof Uint8Array) {")
            self.indent()
            self.emit("let end = v.indexOf(0); if (end < 0) end = v.length;")
            self.emit("return String.fromCharCode(...v.slice(0, end));")
            self.dedent()
            self.emit("}")
            self.emit("return v;")
            self.dedent()
            self.emit("}")
            self.emit("")

        if self._profile:
            self.emit("const __ul_profile_log = [];")
            self.emit("const __ul_enter_stack = [];")
            self.emit("function __ul_enter(func, line) {")
            self.indent()
            self.emit("const t = performance.now() * 1e6;")
            self.emit("__ul_enter_stack.push(t);")
            self.emit('__ul_profile_log.push({event: "enter", func, time_ns: t, line});')
            self.dedent()
            self.emit("}")
            self.emit("function __ul_exit(func, line) {")
            self.indent()
            self.emit("const start = __ul_enter_stack.pop();")
            self.emit("const t = performance.now() * 1e6;")
            self.emit('__ul_profile_log.push({event: "exit", func, time_ns: t, elapsed_ns: t - start, line});')
            self.dedent()
            self.emit("}")
            self.emit("function __ul_profile_dump() {")
            self.indent()
            self.emit('const fs = typeof require !== "undefined" ? require("fs") : null;')
            self.emit("if (fs) {")
            self.indent()
            self.emit('fs.writeFileSync("ul_profile.log", __ul_profile_log.map(e => JSON.stringify(e)).join("\\n") + "\\n");')
            self.dedent()
            self.emit("} else {")
            self.indent()
            self.emit('__ul_profile_log.forEach(e => console.log(JSON.stringify(e)));')
            self.dedent()
            self.emit("}")
            self.dedent()
            self.emit("}")
            self.emit("function __ul_gc_collect() {")
            self.indent()
            self.emit('__ul_profile_log.push({event: "gc_collect", time_ns: performance.now() * 1e6});')
            self.dedent()
            self.emit("}")
            self.emit("function __ul_refcount(ptr, delta) {")
            self.indent()
            self.emit('__ul_profile_log.push({event: "refcount", ptr: String(ptr), delta, time_ns: performance.now() * 1e6});')
            self.dedent()
            self.emit("}")
            self.emit("function __ul_arena_reset() {")
            self.indent()
            self.emit('__ul_profile_log.push({event: "arena_reset", time_ns: performance.now() * 1e6});')
            self.dedent()
            self.emit("}")
            self.emit("")

        # FFI imports — group by library
        ffi_by_lib = {}
        for decl in program.decls:
            if isinstance(decl, ForeignImport):
                ffi_by_lib.setdefault(decl.lib, []).append(decl.name)
        for lib, names in ffi_by_lib.items():
            if lib == "math":
                for name in sorted(set(names)):
                    if name in self._MATH_DIRECT:
                        self.emit(f"const {name} = Math.{name};")
                    elif name in self._MATH_SHIMS:
                        self.emit(f"const {name} = {self._MATH_SHIMS[name]};")
                    else:
                        self.emit(f"// FFI: math.{name} not available in JS Math")
            elif lib == "stdio":
                for name in names:
                    if name == "puts":
                        self.emit(f"function puts(s) {{ console.log(s); return 0; }}")
                    else:
                        self.emit(f"// FFI: {lib}.{name} not supported in JS target")
            elif lib == "string":
                for name in sorted(set(names)):
                    if name == "strlen":
                        self.emit("function strlen(s) { return s.length; }")
                    elif name == "strcmp":
                        self.emit("function strcmp(a, b) { return a < b ? -1 : a > b ? 1 : 0; }")
                    elif name == "strncmp":
                        self.emit("function strncmp(a, b, n) { const sa = a.slice(0, n), sb = b.slice(0, n); return sa < sb ? -1 : sa > sb ? 1 : 0; }")
                    elif name == "strstr":
                        self.emit("function strstr(s, sub) { const i = s.indexOf(sub); return i >= 0 ? s.slice(i) : null; }")
                    elif name == "strcpy":
                        self.emit("function strcpy(dest, src) { for (let i = 0; i < src.length; i++) dest[i] = src.charCodeAt(i); dest[src.length] = 0; return dest; }")
                    elif name == "strcat":
                        self.emit("function strcat(dest, src) { let e = dest.indexOf(0); if (e < 0) e = dest.length; for (let i = 0; i < src.length; i++) dest[e+i] = src.charCodeAt(i); dest[e+src.length] = 0; return dest; }")
                    elif name == "str_char_at":
                        self.emit("function str_char_at(s, i) { return i >= 0 && i < s.length ? s[i] : ''; }")
                    elif name == "str_substr":
                        self.emit("function str_substr(s, start, len) { return s.substr(start, len); }")
                    elif name == "str_index_of":
                        self.emit("function str_index_of(s, sub) { return s.indexOf(sub); }")
                    else:
                        self.emit(f"// FFI: string.{name} not available in JS")
            else:
                self.emit(f"// FFI: import \"{lib}\" not supported in JS target")
        if ffi_by_lib:
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

        # Emit enumerations as frozen objects
        self._enums = {}  # name -> dict of member -> value
        for decl in program.decls:
            if isinstance(decl, EnumDecl):
                self._enums[decl.name] = {name: val for name, val in decl.members}
                members = ", ".join(f"{mname}: {self.gen_expr(mval)}" for mname, mval in decl.members)
                self.emit(f"const {decl.name} = Object.freeze({{{members}}});")
        self.emit("")

        # Emit module-level constants
        for decl in program.decls:
            if isinstance(decl, ConstDecl):
                self.emit(f"const {decl.name} = {self.gen_expr(decl.value)};")

        # Emit module-level variable declarations
        for decl in program.decls:
            if isinstance(decl, VarDecl):
                if decl.type_.is_array and isinstance(decl.init, ArrayLiteral):
                    elems = ", ".join(self.gen_expr(e) for e in decl.init.elements)
                    self.emit(f"let {decl.name} = [{elems}];")
                elif decl.init is not None:
                    self.emit(f"let {decl.name} = {self.gen_expr(decl.init)};")
                else:
                    defaults = {"int": "0", "float": "0.0", "string": '""', "bool": "false"}
                    self.emit(f"let {decl.name} = {defaults.get(decl.type_.name, 'null')};")

        for decl in program.decls:
            if isinstance(decl, (ForeignImport, TypeDecl, ObjectDecl, ConstDecl, DrDirective, NormDirective, AsmBlock, EnumDecl, VarDecl)):
                continue
            self.gen_function(decl)

        if self._needs_range:
            self.emit("")
            self.emit("function __ul_range(...args) {")
            self.indent()
            self.emit("let start, end, step;")
            self.emit("if (args.length === 1) { start = 0; end = args[0]; step = 1; }")
            self.emit("else if (args.length === 2) { start = args[0]; end = args[1]; step = 1; }")
            self.emit("else { start = args[0]; end = args[1]; step = args[2]; }")
            self.emit("const result = [];")
            self.emit("for (let i = start; step > 0 ? i < end : i > end; i += step) result.push(i);")
            self.emit("return result;")
            self.dedent()
            self.emit("}")

        # Sort helper — sort first n elements in-place
        self.emit("function __ul_sort(arr, n) { const sub = arr.slice(0, n).sort((a, b) => a < b ? -1 : a > b ? 1 : 0); for (let i = 0; i < n; i++) arr[i] = sub[i]; }")

        if self._needs_sleep:
            self.emit("function __ul_sleep(ms) { const end = Date.now() + ms; while (Date.now() < end) {} }")

        if self._needs_prompt:
            self.emit("function __ul_prompt(msg) {")
            self.indent()
            self.emit("const fs = require('fs');")
            self.emit("process.stdout.write(msg);")
            self.emit("const buf = Buffer.alloc(4096);")
            self.emit("const n = fs.readSync(0, buf, 0, 4096);")
            self.emit("return buf.toString('utf8', 0, n).replace(/\\r?\\n$/, '');")
            self.dedent()
            self.emit("}")

        self.emit("")
        self.emit("main();")

        return self.result()

    def gen_type_decl(self, decl):
        if getattr(decl, 'packed', False):
            self.emit("// @packed - no padding (packed struct)")
        self.emit(f"function create{decl.name}() {{")
        self.indent()
        fields = ", ".join(
            f"{f.name}: {self.js_default(f.type_)}" for f in decl.fields
        )
        if decl.parent:
            if fields:
                self.emit(f"return {{ ...create{decl.parent}(), {fields} }};")
            else:
                self.emit(f"return {{ ...create{decl.parent}() }};")
        else:
            self.emit(f"return {{ {fields} }};")
        self.dedent()
        self.emit("}")
        self.emit("")

    def gen_object_decl(self, decl):
        # Collect all fields including inherited
        all_fields = []
        if decl.parent and decl.parent in self.type_decls:
            parent = self.type_decls[decl.parent]
            if hasattr(parent, 'fields'):
                all_fields.extend(parent.fields)
        all_fields.extend(decl.fields)
        all_field_names = set(f.name for f in all_fields)

        if decl.parent:
            self.emit(f"class {decl.name} extends {decl.parent} {{")
        else:
            self.emit(f"class {decl.name} {{")
        self.indent()

        # Constructor
        params = ", ".join(
            f"{f.name} = {self.js_default(f.type_)}" for f in all_fields
        )
        self.emit(f"constructor({params}) {{")
        self.indent()
        if decl.parent:
            parent_fields = []
            parent_decl = self.type_decls.get(decl.parent)
            if parent_decl and hasattr(parent_decl, 'fields'):
                parent_fields = parent_decl.fields
            p_args = ", ".join(f.name for f in parent_fields)
            self.emit(f"super({p_args});")
        for f in decl.fields:
            self.emit(f"this.{f.name} = {f.name};")
        self.dedent()
        self.emit("}")

        # Methods
        for method in decl.methods:
            self._current_object = decl.name
            self._object_fields = all_field_names
            params = ", ".join(p.name for p in method.params)
            self.emit(f"{method.name}({params}) {{")
            self.indent()
            for stmt in method.body:
                self.gen_stmt(stmt)
            self.dedent()
            self.emit("}")
            self._current_object = None
            self._object_fields = set()

        self.dedent()
        self.emit("}")
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
                param_parts.append(f"{p.name} = {self.gen_expr(p.default)}")
            else:
                param_parts.append(p.name)
        if getattr(decl, 'variadic', False):
            param_parts.append("..._va_args")
        params = ", ".join(param_parts)
        star = "*" if decl.is_generator else ""
        self.emit(f"function{star} {decl.name}({params}) {{")
        self.indent()
        if self._profile:
            self.emit(f'__ul_enter("{decl.name}", {decl.line});')
        # Emit nested functions as nested function declarations
        for nf in getattr(decl, 'nested_functions', []):
            self.gen_function(nf)
        for stmt in decl.body:
            self.gen_stmt(stmt)
        if self._profile:
            self.emit(f'__ul_exit("{decl.name}", {decl.line});')
        # void main → implicit return 0
        if self._is_main_void(decl):
            if self._profile:
                self.emit("__ul_profile_dump();")
            self.emit("return 0;")
        self.dedent()
        self.emit("}")
        self.emit("")
        self._current_decl = None

    # ── Statements ───────────────────────────────────────────────────────

    def gen_stmt(self, node):
        t = type(node)

        if t == VarDecl:
            self.var_types[node.name] = node.type_
            kw = "const" if node.fixed else "let"
            if node.type_.is_list:
                # list T → let name = []; or let name = [elems];
                if node.init is not None:
                    self.emit(f"{kw} {node.name} = {self.gen_expr(node.init)};")
                else:
                    self.emit(f"{kw} {node.name} = [];")
            elif node.init is not None:
                init_expr = self.gen_expr(node.init)
                # int c = string[i] → c = string.charCodeAt(i) — only for plain string, not array string
                if (node.type_.name in ("int","int8","int16","int32","int64",
                                        "uint8","uint16","uint32","uint64")
                        and isinstance(node.init, Index)
                        and isinstance(node.init.target, Identifier)):
                    target_type = self.var_types.get(node.init.target.name)
                    if target_type and target_type.name == "string" and not getattr(target_type, 'is_array', False):
                        target_expr = self.gen_expr(node.init.target)
                        index_expr = self.gen_expr(node.init.index)
                        init_expr = f"{target_expr}.charCodeAt({index_expr})"
                self.emit(f"{kw} {node.name} = {init_expr};")
                if isinstance(node.init, ArrayLiteral):
                    self._array_sizes[node.name] = str(len(node.init.elements))
            else:
                self.emit(f"{kw} {node.name} = {self.js_default(node.type_)};")

        elif t == Assign:
            val_expr = self.gen_expr(node.value)
            # buffer[i] = string[j] → wrap with .charCodeAt() — only for plain string, not array string
            if (isinstance(node.value, Index)
                    and isinstance(node.value.target, Identifier)):
                vtype = self.var_types.get(node.value.target.name)
                if vtype and vtype.name == "string" and not getattr(vtype, 'is_array', False):
                    target_e = self.gen_expr(node.value.target)
                    index_e = self.gen_expr(node.value.index)
                    val_expr = f"{target_e}.charCodeAt({index_e})"
            self.emit(f"{self.gen_expr(node.target)} {node.op} {val_expr};")

        elif t == If:
            self.emit(f"if ({self.gen_expr(node.condition)}) {{")
            self.indent()
            for s in node.then_body: self.gen_stmt(s)
            self.dedent()
            if node.else_body:
                self.emit("} else {")
                self.indent()
                for s in node.else_body: self.gen_stmt(s)
                self.dedent()
            self.emit("}")

        elif t == While:
            self.emit(f"while ({self.gen_expr(node.condition)}) {{")
            self.indent()
            for s in node.body: self.gen_stmt(s)
            self.dedent()
            self.emit("}")

        elif t == DoWhile:
            self.emit("do {")
            self.indent()
            for s in node.body: self.gen_stmt(s)
            self.dedent()
            self.emit(f"}} while ({self.gen_expr(node.condition)});")

        elif t == For:
            if getattr(node, 'parallel', False):
                self.emit(f"// NOTE: parallel for not supported in JS -- falling back to sequential")
            self.emit(f"for (const {node.var} of {self.gen_expr(node.iterable)}) {{")
            self.indent()
            for s in node.body: self.gen_stmt(s)
            self.dedent()
            self.emit("}")

        elif t == CFor:
            # C-style for → emit as JS for loop
            # Generate init, condition, update
            init_type = type(node.init)
            if init_type == VarDecl:
                self.var_types[node.init.name] = node.init.type_
                init_str = f"let {node.init.name} = {self.gen_expr(node.init.init)}" if node.init.init else f"let {node.init.name} = 0"
            elif init_type == Assign:
                init_str = f"{self.gen_expr(node.init.target)} {node.init.op} {self.gen_expr(node.init.value)}"
            else:
                init_str = ""
            cond_str = self.gen_expr(node.condition)
            if node.update:
                update_type = type(node.update)
                if update_type == PostIncrement:
                    update_str = f"{self.gen_expr(node.update.operand)}++"
                elif update_type == PostDecrement:
                    update_str = f"{self.gen_expr(node.update.operand)}--"
                elif update_type == Assign:
                    update_str = f"{self.gen_expr(node.update.target)} {node.update.op} {self.gen_expr(node.update.value)}"
                elif update_type == ExprStmt:
                    update_str = self.gen_expr(node.update.expr)
                else:
                    update_str = ""
            else:
                update_str = ""
            self.emit(f"for ({init_str}; {cond_str}; {update_str}) {{")
            self.indent()
            for s in node.body: self.gen_stmt(s)
            self.dedent()
            self.emit("}")

        elif t == Return:
            if self._profile and self._current_decl:
                self.emit(f'__ul_exit("{self._current_decl.name}", {self._current_decl.line});')
                if self._current_decl.name == "main":
                    self.emit("__ul_profile_dump();")
            if node.value is not None and isinstance(node.value, OkResult):
                self.emit(f"return {{ok: true, value: {self.gen_expr(node.value.value)}}};")
            elif node.value is not None and isinstance(node.value, ErrorResult):
                self.emit(f"return {{ok: false, error: {self.gen_expr(node.value.value)}}};")
            elif node.value is not None:
                val_expr = self.gen_expr(node.value)
                if (self._needs_str_helper
                        and self._current_decl
                        and self._current_decl.return_type
                        and self._current_decl.return_type.name == "string"):
                    val_expr = f"__ul_to_str({val_expr})"
                self.emit(f"return {val_expr};")
            else:
                self.emit("return;")

        elif t == Print:
            self.emit(f"console.log({self.gen_expr(node.value)});")

        elif t == ExprStmt:
            self.emit(f"{self.gen_expr(node.expr)};")

        elif t == Escape:
            self.emit("break;")

        elif t == Continue:
            self.emit("continue;")

        elif t == Match:
            self.emit(f"switch ({self.gen_expr(node.subject)}) {{")
            self.indent()
            for case in node.cases:
                if case.value is None:
                    self.emit("default:")
                else:
                    self.emit(f"case {self.gen_expr(case.value)}:")
                self.indent()
                for s in case.body: self.gen_stmt(s)
                self.emit("break;")
                self.dedent()
            self.dedent()
            self.emit("}")

        elif t == Yield:
            self.emit(f"yield {self.gen_expr(node.value)};")

        elif t == PostIncrement:
            self.emit(f"{self.gen_expr(node.operand)}++;")

        elif t == PostDecrement:
            self.emit(f"{self.gen_expr(node.operand)}--;")

        elif t == TupleDestructure:
            names = ", ".join(name for _, name in node.targets)
            self.emit(f"let [{names}] = {self.gen_expr(node.value)};")
            for t_, name in node.targets:
                self.var_types[name] = t_

        elif t == KillswitchStmt:
            if self._safety_checked:
                cond = self.gen_expr(node.condition)
                self.emit(f"if (!({cond})) {{")
                self.indent()
                self.emit(f'process.stderr.write("killswitch failed at line {node.line}\\n");')
                self.emit("process.exit(1);")
                self.dedent()
                self.emit("}")

        elif t == PortalDecl:
            self.emit(f"// portal {node.name}  (goto not supported in JS - restructure as loop)")

        elif t == GotoStmt:
            self.emit(f"// goto {node.target}  (goto not supported in JS - restructure as loop)")

        elif t == TryStmt:
            self.emit("try {")
            self.indent()
            for s in node.body:
                self.gen_stmt(s)
            self.dedent()
            if node.handlers:
                self.emit("} catch (__ex) {")
                self.indent()
                self.emit('let __ex_type = __ex && __ex.__ul_type ? __ex.__ul_type : (__ex && __ex.constructor ? __ex.constructor.name : "Error");')
                self.emit("let __ex_msg = __ex && __ex.__ul_msg ? __ex.__ul_msg : (__ex ? String(__ex) : \"\");")
                first = True
                for handler in node.handlers:
                    if handler.exception_type:
                        kw = "if" if first else "} else if"
                        self.emit(f'{kw} (__ex_type === "{handler.exception_type}") {{')
                    else:
                        if first:
                            self.emit("{")
                        else:
                            self.emit("} else {")
                    self.indent()
                    if handler.binding_name:
                        self.emit(f"let {handler.binding_name} = __ex_msg;")
                    for s in handler.body:
                        self.gen_stmt(s)
                    self.dedent()
                    first = False
                has_catchall = any(h.exception_type is None for h in node.handlers)
                if not has_catchall:
                    self.emit("} else {")
                    self.indent()
                    self.emit("throw __ex;")
                    self.dedent()
                self.emit("}")
                self.dedent()
                if node.finally_body:
                    self.emit("} finally {")
                    self.indent()
                    for s in node.finally_body:
                        self.gen_stmt(s)
                    self.dedent()
                    self.emit("}")
                else:
                    self.emit("}")
            elif node.finally_body:
                self.emit("} finally {")
                self.indent()
                for s in node.finally_body:
                    self.gen_stmt(s)
                self.dedent()
                self.emit("}")
            else:
                self.emit("}")

        elif t == ThrowStmt:
            msg = self.gen_expr(node.message)
            self.emit(f'throw {{__ul_type: "{node.exception_type}", __ul_msg: {msg}, message: {msg}}};')

        elif t == WithStmt:
            expr = self.gen_expr(node.expr)
            self.emit(f"let {node.binding} = {expr};")
            self.emit("try {")
            self.indent()
            for s in node.body: self.gen_stmt(s)
            self.dedent()
            self.emit("} finally {")
            self.indent()
            self.emit(f"if ({node.binding} && typeof {node.binding}.close === 'function') {node.binding}.close();")
            self.dedent()
            self.emit("}")

        elif t == ConstDecl:
            self.emit(f"const {node.name} = {self.gen_expr(node.value)};")

        elif t == NormDirective:
            self.emit(f"// @norm {node.level}")

    # ── Expressions ──────────────────────────────────────────────────────

    def _is_float_expr(self, node):
        """Check if an expression is float/double typed (for division truncation)."""
        if isinstance(node, FloatLiteral):
            return True
        if isinstance(node, IntLiteral):
            return False
        if isinstance(node, Identifier):
            vt = self.var_types.get(node.name)
            if vt and hasattr(vt, 'name') and vt.name in ("float", "double"):
                return True
        if isinstance(node, Cast):
            return hasattr(node.target_type, 'name') and node.target_type.name in ("float", "double")
        if isinstance(node, BinaryOp):
            return self._is_float_expr(node.left) or self._is_float_expr(node.right)
        if isinstance(node, Call):
            fd = self._func_decls.get(node.name)
            if fd and fd.return_type and hasattr(fd.return_type, 'name') and fd.return_type.name in ("float", "double"):
                return True
        return False

    def gen_expr(self, node):
        t = type(node)

        if t == IntLiteral:    return str(node.value)
        if t == FloatLiteral:  return repr(node.value)
        if t == StringLiteral:
            escaped = node.value.replace("\\", "\\\\").replace('"', '\\"').replace("\n", "\\n").replace("\t", "\\t")
            return f'"{escaped}"'
        if t == BoolLiteral:   return "true" if node.value else "false"
        if t == EmptyLiteral:  return "null"

        if t == Identifier:
            if self._current_object and node.name in self._object_fields:
                return f"this.{node.name}"
            return node.name

        if t == BinaryOp:
            if node.op == "|>":
                return f"{self.gen_expr(node.right)}({self.gen_expr(node.left)})"
            if node.op == "in":
                return f"{self.gen_expr(node.right)}.includes({self.gen_expr(node.left)})"
            op_map = {"and": "&&", "or": "||", "==": "===", "!=": "!==",
                      "both1": "&", "either1": "|", "diff": "^",
                      "left": "<<", "right": ">>"}
            op = op_map.get(node.op, node.op)
            # Integer division: JS / always gives float, need Math.trunc for int/int
            if node.op == "/" and not self._is_float_expr(node.left) and not self._is_float_expr(node.right):
                return f"Math.trunc({self.gen_expr(node.left)} {op} {self.gen_expr(node.right)})"
            return f"({self.gen_expr(node.left)} {op} {self.gen_expr(node.right)})"

        if t == MethodCall:
            if isinstance(node.target, Identifier):
                vtype = self.var_types.get(node.target.name)
                if vtype and (vtype.is_list or vtype.is_array):
                    target = self.gen_expr(node.target)
                    if node.method in ("append", "push", "add"):
                        val = self.gen_expr(node.args[0])
                        return f"{target}.push({val})"
                    if node.method == "pop":
                        return f"{target}.pop()"
                    if node.method == "drop":
                        if node.args:
                            idx = self.gen_expr(node.args[0])
                            return f"{target}.splice({idx}, 1)[0]"
                        return f"{target}.pop()"
                    if node.method == "insert":
                        idx = self.gen_expr(node.args[0])
                        val = self.gen_expr(node.args[1])
                        return f"({target}.splice({idx}, 0, {val}), undefined)"
                    if node.method == "remove":
                        val = self.gen_expr(node.args[0])
                        return f"{target}.splice({target}.indexOf({val}), 1)"
                    if node.method == "clear":
                        return f"({target}.length = 0)"
                    if node.method == "sort":
                        return f"{target}.sort((a, b) => a < b ? -1 : a > b ? 1 : 0)"
                    if node.method == "reverse":
                        return f"{target}.reverse()"
                    if node.method == "contains":
                        val = self.gen_expr(node.args[0])
                        return f"{target}.includes({val})"
                    if node.method == "len":
                        return f"{target}.length"
                    if node.method == "sum":
                        return f"{target}.reduce((a, b) => a + b, 0)"
                    if node.method == "min":
                        return f"Math.min(...{target})"
                    if node.method == "max":
                        return f"Math.max(...{target})"
                    # Fallback for arrays: array_method(target, len, ...)
                    if vtype.is_array:
                        extra_args = ", ".join(self.gen_expr(a) for a in node.args)
                        all_args = f"{target}, {target}.length, {extra_args}" if extra_args else f"{target}, {target}.length"
                        return f"array_{node.method}({all_args})"
                    args = ", ".join(self.gen_expr(a) for a in node.args)
                    return f"{target}.{node.method}({args})"
                if vtype and vtype.name == "string":
                    target = self.gen_expr(node.target)
                    args = ", ".join(self.gen_expr(a) for a in node.args)
                    all_args = f"{target}, {args}" if args else target
                    return f"str_{node.method}({all_args})"
            target = self.gen_expr(node.target)
            args = ", ".join(self.gen_expr(a) for a in node.args)
            return f"{target}.{node.method}({args})"

        if t == StructLiteral:
            fields = ", ".join(f"{fname}: {self.gen_expr(fval)}" for fname, fval in node.fields)
            return f"{{{fields}}}"

        if t == UnaryOp:
            if node.op == "not":     return f"(!{self.gen_expr(node.operand)})"
            if node.op == "-":       return f"(-{self.gen_expr(node.operand)})"
            if node.op == "bitflip": return f"(~{self.gen_expr(node.operand)})"

        if t == Cast:
            inner = node.expr
            if isinstance(inner, Call) and inner.name == "change" and len(inner.args) == 1:
                inner = inner.args[0]
            target = node.target_type.name
            if target in ("int","int8","int16","int32","int64"):
                return f"Math.trunc({self.gen_expr(inner)})"
            if target in ("float", "double"):
                return f"Number({self.gen_expr(inner)})"
            if target == "string":
                return f"String({self.gen_expr(inner)})"
            if target == "bool":
                return f"Boolean({self.gen_expr(inner)})"
            return self.gen_expr(inner)

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
                return f"{self.gen_expr(call_args[0])}.length"
            if node.name == "exit":
                return f"process.exit({args})"
            if node.name == "range":
                self._needs_range = True
                return f"__ul_range({args})"
            if node.name == "memtake":
                return f"new Uint8Array({args})"
            if node.name == "memgive":
                return "undefined"
            if node.name == "char_code":
                s_expr = self.gen_expr(call_args[0])
                i_expr = self.gen_expr(call_args[1])
                return f"{s_expr}.charCodeAt({i_expr})"
            if node.name == "char_from_code":
                n_expr = self.gen_expr(call_args[0])
                return f"String.fromCharCode({n_expr})"
            if node.name == "sort":
                arr_expr = self.gen_expr(call_args[0])
                n_expr = self.gen_expr(call_args[1])
                return f"__ul_sort({arr_expr}, {n_expr})"
            # System functions (Task 11)
            if node.name == "time":
                return "Math.floor(Date.now() / 1000)"
            if node.name == "clock":
                return "(typeof process !== 'undefined' ? Number(process.hrtime.bigint()) : Math.floor(performance.now() * 1e6))"
            if node.name == "sleep":
                ms_expr = self.gen_expr(call_args[0])
                self._needs_sleep = True
                return f"__ul_sleep({ms_expr})"
            if node.name == "random":
                return "Math.random()"
            if node.name == "random_int":
                lo = self.gen_expr(call_args[0])
                hi = self.gen_expr(call_args[1])
                return f"(Math.floor(Math.random() * ({hi} - {lo} + 1)) + {lo})"
            if node.name == "random_seed":
                return "undefined /* random_seed not supported in JS */"
            if node.name == "userinput":
                return "(typeof process !== 'undefined' ? process.argv.slice(1) : [])"
            if node.name == "vault":
                return f"(typeof process !== 'undefined' ? (process.env[{self.gen_expr(call_args[0])}] || '') : '')"
            if node.name == "abort":
                return "process.exit(1)"
            if node.name == "prompt":
                self._needs_prompt = True
                return f"__ul_prompt({self.gen_expr(call_args[0])})"
            # Python builtin equivalents (Task 15)
            if node.name == "zip":
                a = self.gen_expr(call_args[0])
                b = self.gen_expr(call_args[1])
                return f"{a}.map((e, i) => [e, {b}[i]])"
            if node.name == "enumerate":
                return f"{self.gen_expr(call_args[0])}.map((e, i) => [i, e])"
            if node.name == "map_fn":
                return f"{self.gen_expr(call_args[1])}.map({self.gen_expr(call_args[0])})"
            if node.name == "filter_fn":
                return f"{self.gen_expr(call_args[1])}.filter({self.gen_expr(call_args[0])})"
            return f"{node.name}({args})"

        if t == Index:
            target_expr = self.gen_expr(node.target)
            index_expr = self.gen_expr(node.index)
            if self._safety_checked and isinstance(node.target, Identifier):
                return f'__ul_checked_access({target_expr}, {index_expr}, "{node.target.name}", {node.line})'
            return f"{target_expr}[{index_expr}]"

        if t == SliceExpr:
            return f"{self.gen_expr(node.target)}.slice({self.gen_expr(node.start)}, {self.gen_expr(node.end)})"

        if t == FieldAccess:
            # self.field inside method → this.field in JS
            if (isinstance(node.target, Identifier) and node.target.name == "self"
                    and self._current_object):
                return f"this.{node.field}"
            return f"{self.gen_expr(node.target)}.{node.field}"

        if t == TupleLiteral:
            elems = ", ".join(self.gen_expr(e) for e in node.elements)
            return f"[{elems}]"

        if t == ArrayLiteral:
            elems = ", ".join(self.gen_expr(e) for e in node.elements)
            return f"[{elems}]"

        if t == ArrayComprehension:
            iterable = self.gen_expr(node.iterable)
            expr = self.gen_expr(node.expr)
            return f"[...{iterable}].map({node.var} => {expr})"

        if t == PostIncrement:
            return f"{self.gen_expr(node.operand)}++"
        if t == PostDecrement:
            return f"{self.gen_expr(node.operand)}--"

        if t == OkResult:
            return f"{{ok: true, value: {self.gen_expr(node.value)}}}"

        if t == ErrorResult:
            return f"{{ok: false, error: {self.gen_expr(node.value)}}}"

        if t == ResultPropagation:
            tmp = f"_r{self._tmp_counter}"
            self._tmp_counter += 1
            call_expr = self.gen_expr(node.expr)
            self.emit(f"const {tmp} = {call_expr};")
            self.emit(f"if (!{tmp}.ok) {{")
            self.indent()
            if self._current_decl and self._current_decl.returns_result:
                self.emit(f"return {tmp};")
            else:
                self.emit(f'console.error("error: " + {tmp}.error);')
                self.emit(f"process.exit(1);")
            self.dedent()
            self.emit("}")
            return f"{tmp}.value"

        return "null"


def generate(program, profile=False):
    return JSGen(profile=profile).gen_program(program)
