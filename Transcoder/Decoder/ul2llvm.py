# ulc/codegen_llvm.py — UniLogic → LLVM IR code generator
# Walks the AST produced by parser.py and emits LLVM IR text (.ll files).
# Uses alloca/load/store for all locals — mem2reg optimizes to SSA.
# Output can be compiled via: clang output.ll -o output

from ast_nodes import *

# UL type → LLVM IR type
UL_TO_LLVM = {
    "int":    "i32",
    "int8":   "i8",
    "int16":  "i16",
    "int32":  "i32",
    "int64":  "i64",
    "uint8":  "i8",
    "uint16": "i16",
    "uint32": "i32",
    "uint64": "i64",
    "float":  "float",
    "double": "double",
    "string": "i8*",
    "bool":   "i1",
    "none":   "void",
    "char":   "i8",
}

# LLVM type → printf format specifier
LLVM_FMT = {
    "i32":    "%d",
    "i8":     "%d",
    "i16":    "%d",
    "i64":    "%lld",
    "float":  "%f",
    "double": "%f",
    "i8*":    "%s",
    "i1":     "%d",
}


class LLVMGen:
    def __init__(self, target_triple=None, profile=False):
        self.target_triple  = target_triple
        self.lines          = []
        self.strings        = []      # (global_name, encoded_text, byte_count)
        self.string_cache   = {}      # raw value → (global_name, byte_count)
        self.tmp_counter    = 0
        self.label_counter  = 0
        self.var_addrs      = {}      # var name → (alloca_reg, llvm_type)
        self.var_types      = {}      # var name → TypeName
        self.var_array_info = {}      # var name → (elem_llvm_type, count)
        self.func_sigs      = {}      # func name → (ret_llvm_type, [param_llvm_types])
        self.current_func   = None    # current FunctionDecl being generated
        self.block_terminated = False
        self.needs_printf   = False
        self.used_builtins  = set()   # tracks which external builtins are called
        self._loop_labels   = None    # (cond_label, end_label) for escape/continue
        self.struct_layouts = {}      # type_name → [(field_name, llvm_type)]
        self._generators    = {}     # name → TypeName (yield type)
        self._gen_callbacks = []     # buffered callback function lines
        self._cb_counter    = 0
        self._result_funcs  = {}     # name → TypeName (ok payload type)
        self._result_types  = {}     # result struct name → payload llvm type
        self._needs_exit    = False
        self._needs_stderr  = False
        self._profile       = profile

    # ── Helpers ──────────────────────────────────────────────────────────

    def emit(self, text=""):
        self.lines.append(text)

    def tmp(self):
        name = f"%t{self.tmp_counter}"
        self.tmp_counter += 1
        return name

    def label(self, prefix="L"):
        name = f"{prefix}.{self.label_counter}"
        self.label_counter += 1
        return name

    def llvm_type(self, type_node):
        if type_node is None:
            return "void"
        if type_node.name in self.struct_layouts:
            base = f"%{type_node.name}"
        else:
            base = UL_TO_LLVM.get(type_node.name, "i32")
        if type_node.pointer:
            return base + "*"
        return base

    def llvm_param_type(self, type_node):
        """LLVM type for function parameters — arrays decay to pointers."""
        if type_node is None:
            return "void"
        if type_node.is_array:
            base = UL_TO_LLVM.get(type_node.name, "i32")
            return base + "*"
        return self.llvm_type(type_node)

    def llvm_field_type(self, type_node):
        """LLVM type for struct fields — arrays keep their dimensions."""
        if type_node is None:
            return "void"
        if type_node.is_array and type_node.array_size > 0:
            if type_node.name in self.struct_layouts:
                base = f"%{type_node.name}"
            else:
                base = UL_TO_LLVM.get(type_node.name, "i32")
            return f"[{type_node.array_size} x {base}]"
        return self.llvm_type(type_node)

    def _result_struct_name(self, payload_type):
        """Return LLVM struct name for a result type, e.g. %_Result_i32."""
        lt = self.llvm_type(payload_type)
        safe = lt.replace("*", "_ptr").replace("%", "").replace(" ", "_")
        return f"%_Result_{safe}"

    def _ensure_result_type(self, payload_type):
        """Register a result struct type if not already done."""
        name = self._result_struct_name(payload_type)
        if name not in self._result_types:
            lt = self.llvm_type(payload_type)
            self._result_types[name] = lt
        return name

    def _get_empty_string(self):
        """Return a GEP pointer to an empty string constant."""
        name, length = self.add_string("")
        ptr = self.tmp()
        self.emit(f"  {ptr} = getelementptr [{length} x i8], [{length} x i8]* {name}, i32 0, i32 0")
        return ptr

    def add_string(self, value):
        """Register a global string constant. value is a Python string with real chars."""
        if value in self.string_cache:
            return self.string_cache[value]

        name = f"@.str.{len(self.strings)}"
        encoded = ""
        byte_count = 0
        for ch in value:
            code = ord(ch)
            if code == 10:       encoded += "\\0A"
            elif code == 9:      encoded += "\\09"
            elif code == 0:      encoded += "\\00"
            elif code == 34:     encoded += "\\22"
            elif code == 92:     encoded += "\\5C"
            elif 32 <= code <= 126: encoded += ch
            else:                encoded += f"\\{code:02X}"
            byte_count += 1
        byte_count += 1  # null terminator

        self.strings.append((name, encoded, byte_count))
        self.string_cache[value] = (name, byte_count)
        return name, byte_count

    def ensure_i1(self, val, vt):
        """Convert a value to i1 for use as a branch condition."""
        if vt == "i1":
            return val
        result = self.tmp()
        if vt in ("float", "double"):
            self.emit(f"  {result} = fcmp one {vt} {val}, 0.0")
        else:
            self.emit(f"  {result} = icmp ne {vt} {val}, 0")
        return result

    def result(self):
        return "\n".join(self.lines) + "\n"

    def _hoist_allocas(self):
        """Move all alloca instructions in the last emitted function to the entry block.
        LLVM requires allocas in the entry block for correct stack frame setup.
        Allocas in loop bodies cause stack overflow.
        """
        # Find the last function: scan backwards for "define"
        func_start = None
        entry_line = None
        func_end = None
        for i in range(len(self.lines) - 1, -1, -1):
            line = self.lines[i]
            if line == "}":
                func_end = i
            elif line.startswith("define "):
                func_start = i
                break
        if func_start is None or func_end is None:
            return
        # Find entry: label
        for i in range(func_start, func_end):
            if self.lines[i] == "entry:":
                entry_line = i
                break
        if entry_line is None:
            return
        # Collect allocas that are NOT right after entry
        # First, find where entry-block allocas end (first non-alloca, non-store after entry)
        entry_allocas_end = entry_line + 1
        while entry_allocas_end < func_end:
            stripped = self.lines[entry_allocas_end].strip()
            if "= alloca " in stripped or stripped.startswith("store "):
                entry_allocas_end += 1
            else:
                break
        # Now scan the rest for stray allocas
        stray_allocas = []
        new_lines = self.lines[:func_end]  # copy up to }
        i = entry_allocas_end
        while i < func_end:
            stripped = self.lines[i].strip()
            if "= alloca " in stripped:
                stray_allocas.append(self.lines[i])
                new_lines[i] = "  ; (alloca hoisted to entry)"
            i += 1
        if stray_allocas:
            # Insert stray allocas right after entry block allocas
            for j, alloca_line in enumerate(stray_allocas):
                new_lines.insert(entry_allocas_end + j, alloca_line)
            # Recalculate func_end
            new_lines.extend(self.lines[func_end:])
            self.lines = new_lines

    # ── Program ──────────────────────────────────────────────────────────

    def gen_program(self, program):
        # Collect DR directives for output header
        self._dr_directives = [d for d in program.decls if isinstance(d, DrDirective)]

        # Collect type declarations and register struct layouts
        type_decls = []
        for decl in program.decls:
            if isinstance(decl, TypeDecl):
                type_decls.append(decl)
                self.struct_layouts[decl.name] = []  # placeholder for forward refs

        for td in type_decls:
            inherited = []
            if td.parent and td.parent in self.struct_layouts:
                inherited = list(self.struct_layouts[td.parent])
            own = [(f.name, self.llvm_field_type(f.type_)) for f in td.fields]
            self.struct_layouts[td.name] = inherited + own

        # Object declarations — register struct layouts and vtable skeletons
        for decl in program.decls:
            if isinstance(decl, ObjectDecl):
                self._gen_object_skeleton(decl)

        # Separate foreign imports from function declarations
        foreign_imports = []
        function_decls = []
        for decl in program.decls:
            if isinstance(decl, ForeignImport):
                foreign_imports.append(decl)
            elif isinstance(decl, (TypeDecl, ObjectDecl, ConstDecl, DrDirective, NormDirective, AsmBlock)):
                pass  # already handled
            else:
                function_decls.append(decl)
        # Flatten nested functions into the function list
        expanded = []
        for fn in function_decls:
            for nf in getattr(fn, 'nested_functions', []):
                expanded.append(nf)
            expanded.append(fn)
        function_decls = expanded

        # Register foreign import signatures
        for fi in foreign_imports:
            ret = self.llvm_type(fi.return_type)
            params = [self.llvm_param_type(p.type_) for p in fi.params]
            self.func_sigs[fi.name] = (ret, params)
            # Store variadic flag
            if fi.variadic:
                self._variadic_funcs = getattr(self, '_variadic_funcs', set())
                self._variadic_funcs.add(fi.name)

        # Register generators and result functions
        for decl in function_decls:
            if decl.is_generator:
                self._generators[decl.name] = decl.return_type
            if decl.returns_result:
                payload_type = decl.return_type if decl.return_type else TypeName("int")
                self._result_funcs[decl.name] = payload_type
                self._ensure_result_type(payload_type)

        # First pass: collect function signatures for call resolution
        for decl in function_decls:
            if decl.is_generator:
                yield_lt = self.llvm_type(decl.return_type)
                cb_type = f"void ({yield_lt})*"
                params = [self.llvm_param_type(p.type_) for p in decl.params] + [cb_type]
                self.func_sigs[decl.name] = ("void", params)
            elif decl.returns_result:
                payload_type = decl.return_type if decl.return_type else TypeName("int")
                rtype = self._result_struct_name(payload_type)
                params = [self.llvm_param_type(p.type_) for p in decl.params]
                self.func_sigs[decl.name] = (rtype, params)
            else:
                ret = self.llvm_type(decl.return_type)
                # C standard requires int main — override void main
                if decl.name == "main" and ret == "void":
                    ret = "i32"
                params = [self.llvm_param_type(p.type_) for p in decl.params]
                self.func_sigs[decl.name] = (ret, params)

        # Generate all function bodies into a buffer (including imported .ul functions)
        func_lines = []
        for decl in function_decls:
            self.lines = []
            self.gen_function(decl)
            func_lines.extend(self.lines)

        # Assemble final output: DR comments, types, globals, declarations, functions
        self.lines = []

        # Target triple (when targeting a specific backend like wasm32)
        if self.target_triple:
            self.emit(f'target triple = "{self.target_triple}"')
            self.emit("")

        # DR directives as comments at the top
        if self._dr_directives:
            for d in self._dr_directives:
                self.emit(f"; @dr {d.key} = {d.value}")
            self.emit("")

        # Struct type definitions
        for td in type_decls:
            field_types = ", ".join(lt for _, lt in self.struct_layouts[td.name])
            self.emit(f"%{td.name} = type {{ {field_types} }}")

        # Result type struct definitions: { i1, payload_type, i8* }
        for rname, payload_lt in self._result_types.items():
            self.emit(f"{rname} = type {{ i1, {payload_lt}, i8* }}")

        if type_decls or self._result_types:
            self.emit("")

        # String constants
        for name, encoded, length in self.strings:
            self.emit(f'{name} = private unnamed_addr constant [{length} x i8] c"{encoded}\\00"')
        if self.strings:
            self.emit("")

        # Profile function declarations
        if self._profile:
            self.emit("declare void @__ul_enter(i8*, i8*, i32)")
            self.emit("declare void @__ul_exit(i8*, i8*, i32)")
            self.emit("declare void @__ul_profile_dump()")
            self.emit("declare i8* @__ul_alloc(i64)")
            self.emit("declare void @__ul_free(i8*)")
            # DR behavior monitor hooks
            self.emit("declare void @__ul_gc_collect()")
            self.emit("declare void @__ul_refcount(i8*, i32)")
            self.emit("declare void @__ul_arena_reset()")

        # External declarations
        if getattr(self, '_needs_snprintf', False):
            self.emit("declare i32 @snprintf(i8*, i64, i8*, ...)")
        if self.needs_printf:
            self.emit("declare i32 @printf(i8*, ...)")
        if self._needs_exit:
            if "exit" not in self.used_builtins:
                self.emit("declare void @exit(i32)")
            self.emit("declare i32 @fprintf(i8*, i8*, ...)")
        if self._needs_stderr:
            self.emit("@stderr = external global i8*")
        for name in sorted(self.used_builtins):
            self.emit(BUILTIN_DECLS[name])

        # Foreign import declarations
        for fi in foreign_imports:
            # Skip printf if already declared above
            if fi.name == "printf" and self.needs_printf:
                continue
            ret = self.llvm_type(fi.return_type)
            param_types = ", ".join(self.llvm_param_type(p.type_) for p in fi.params)
            if fi.variadic:
                if param_types:
                    param_types += ", ..."
                else:
                    param_types = "..."
            self.emit(f"declare {ret} @{fi.name}({param_types})")

        if self.needs_printf or self.used_builtins or foreign_imports:
            self.emit("")

        # Generator callback functions
        self.lines.extend(self._gen_callbacks)

        # Function definitions
        self.lines.extend(func_lines)

        return self.result()

    # ── Object (vtable skeleton) ─────────────────────────────────────────

    def _gen_object_skeleton(self, decl):
        """Emit LLVM IR struct definitions for an object type.
        TODO: Full vtable dispatch — currently emits struct layout only."""
        # Register struct layout with fields
        inherited = []
        if decl.parent and decl.parent in self.struct_layouts:
            inherited = list(self.struct_layouts[decl.parent])
        own = [(f.name, self.llvm_type(f.type_)) for f in decl.fields]
        # Add vtable pointer slot and type tag
        vtable_fields = [("_vtable", "i8*"), ("__type", "i8*")]
        self.struct_layouts[decl.name] = inherited + vtable_fields + own

        # TODO: Emit vtable struct type
        # %Animal_VTable = type { i8* (Animal*)* speak_ptr, ... }

        # TODO: Emit static vtable instance
        # @_Animal_vtable = internal constant %Animal_VTable { ... }

        # TODO: Emit method implementations as regular functions
        # define i8* @Animal_speak(%Animal* %self) { ... }
        for method in decl.methods:
            params = ", ".join(self.llvm_param_type(p.type_) for p in method.params)
            self_param = f"%{decl.name}*"
            ret = self.llvm_type(method.return_type) if method.return_type else "void"
            self.emit(f"; TODO: {ret} @{decl.name}_{method.name}({self_param}, {params})")

    # ── Function ─────────────────────────────────────────────────────────

    def gen_function(self, decl):
        self.tmp_counter = 0
        self.label_counter = 0
        self.var_addrs = {}
        self.block_terminated = False
        self.current_func = decl
        self._loop_labels = None

        if decl.is_generator:
            yield_lt = self.llvm_type(decl.return_type)
            params = ", ".join(
                f"{self.llvm_param_type(p.type_)} %{p.name}.arg" for p in decl.params
            )
            cb_param = f"void ({yield_lt})* %_yield_cb"
            if params:
                params += ", " + cb_param
            else:
                params = cb_param
            ret = "void"
        elif decl.returns_result:
            payload_type = decl.return_type if decl.return_type else TypeName("int")
            ret = self._result_struct_name(payload_type)
            params = ", ".join(
                f"{self.llvm_param_type(p.type_)} %{p.name}.arg" for p in decl.params
            )
        else:
            ret = self.llvm_type(decl.return_type)
            # C standard requires int main — override void main
            if decl.name == "main" and ret == "void":
                ret = "i32"
            params = ", ".join(
                f"{self.llvm_param_type(p.type_)} %{p.name}.arg" for p in decl.params
            )
        self.emit(f"define {ret} @{decl.name}({params}) {{")
        self.emit("entry:")

        # Alloca + store for each parameter
        for p in decl.params:
            lt = self.llvm_param_type(p.type_)
            addr = f"%{p.name}.addr"
            self.emit(f"  {addr} = alloca {lt}")
            self.emit(f"  store {lt} %{p.name}.arg, {lt}* {addr}")
            self.var_addrs[p.name] = (addr, lt)
            self.var_types[p.name] = p.type_

        # Body
        for stmt in decl.body:
            self.gen_stmt(stmt)

        # Safety net: ensure every function ends with a terminator
        if not self.block_terminated:
            if ret == "void":
                self.emit("  ret void")
            elif ret.endswith("*") or ret == "ptr":
                self.emit(f"  ret {ret} null")
            else:
                self.emit(f"  ret {ret} 0")

        self.emit("}")
        self.emit("")
        # Post-process: move all allocas to the entry block
        self._hoist_allocas()
        self.current_func = None

    # ── Statements ───────────────────────────────────────────────────────

    def gen_stmt(self, node):
        if self.block_terminated:
            return  # dead code after ret/br

        t = type(node)

        if t == VarDecl:
            lt = self.llvm_type(node.type_)
            addr = f"%{node.name}.addr"

            # If variable already has an alloca (re-declaration in loop body), reuse it
            already_exists = node.name in self.var_addrs

            # auto type: infer from initializer
            if node.type_.name == "auto" and node.init is not None:
                init_val, init_lt = self.gen_expr(node.init)
                lt = init_lt
                self.var_types[node.name] = node.type_
                if not already_exists:
                    self.emit(f"  {addr} = alloca {lt}")
                    self.var_addrs[node.name] = (addr, lt)
                else:
                    addr, lt = self.var_addrs[node.name]
                self.emit(f"  store {lt} {init_val}, {lt}* {addr}")
                return

            self.var_types[node.name] = node.type_

            if node.type_.is_array and isinstance(node.init, ArrayLiteral):
                count = len(node.init.elements)
                arr_type = f"[{count} x {lt}]"
                if not already_exists:
                    self.emit(f"  {addr} = alloca {arr_type}")
                    self.var_addrs[node.name] = (addr, arr_type)
                else:
                    addr, arr_type = self.var_addrs[node.name]
                self.var_array_info[node.name] = (lt, count)
                for i, elem in enumerate(node.init.elements):
                    val, vt = self.gen_expr(elem)
                    val, _ = self.cast_if_needed(val, vt, lt)
                    ptr = self.tmp()
                    self.emit(f"  {ptr} = getelementptr {arr_type}, {arr_type}* {addr}, i32 0, i32 {i}")
                    self.emit(f"  store {lt} {val}, {lt}* {ptr}")
            else:
                if not already_exists:
                    self.emit(f"  {addr} = alloca {lt}")
                    self.var_addrs[node.name] = (addr, lt)
                else:
                    addr, lt = self.var_addrs[node.name]
                if node.init is not None:
                    # Struct zero-init: store zeroinitializer instead of casting 0
                    if lt.startswith("%") and isinstance(node.init, IntLiteral) and node.init.value == 0:
                        self.emit(f"  store {lt} zeroinitializer, {lt}* {addr}")
                    elif lt.startswith("%") and isinstance(node.init, Cast) and isinstance(node.init.expr, IntLiteral) and node.init.expr.value == 0:
                        self.emit(f"  store {lt} zeroinitializer, {lt}* {addr}")
                    else:
                        val, vt = self.gen_expr(node.init)
                        val, _ = self.cast_if_needed(val, vt, lt)
                        self.emit(f"  store {lt} {val}, {lt}* {addr}")

        elif t == Assign:
            val, vt = self.gen_expr(node.value)
            if isinstance(node.target, FieldAccess):
                ptr, field_lt = self.gen_field_ptr(node.target)
                if node.op == "=":
                    val, _ = self.cast_if_needed(val, vt, field_lt)
                    self.emit(f"  store {field_lt} {val}, {field_lt}* {ptr}")
                else:
                    cur = self.tmp()
                    self.emit(f"  {cur} = load {field_lt}, {field_lt}* {ptr}")
                    val, _ = self.cast_if_needed(val, vt, field_lt)
                    result = self.tmp()
                    if field_lt in ("float", "double"):
                        ops = {"+=":"fadd", "-=":"fsub", "*=":"fmul", "/=":"fdiv", "%=":"frem"}
                    else:
                        ops = {"+=":"add", "-=":"sub", "*=":"mul", "/=":"sdiv", "%=":"srem"}
                    inst = ops.get(node.op, "add")
                    self.emit(f"  {result} = {inst} {field_lt} {cur}, {val}")
                    self.emit(f"  store {field_lt} {result}, {field_lt}* {ptr}")
            elif isinstance(node.target, Index):
                # Array element assignment: arr[i] = value or s.data[i] = value
                idx, it = self.gen_expr(node.target.index)
                # FieldAccess target: s.data[i] = value
                if isinstance(node.target.target, FieldAccess):
                    field_ptr, field_lt = self.gen_field_ptr(node.target.target)
                    if field_lt.startswith("["):
                        elem_type = field_lt.split("x ")[-1].rstrip("]").strip()
                        ptr = self.tmp()
                        self.emit(f"  {ptr} = getelementptr {field_lt}, {field_lt}* {field_ptr}, i32 0, {it} {idx}")
                        if node.op == "=":
                            val, _ = self.cast_if_needed(val, vt, elem_type)
                            self.emit(f"  store {elem_type} {val}, {elem_type}* {ptr}")
                        else:
                            cur = self.tmp()
                            self.emit(f"  {cur} = load {elem_type}, {elem_type}* {ptr}")
                            val, _ = self.cast_if_needed(val, vt, elem_type)
                            result = self.tmp()
                            inst = {"+=":"add", "-=":"sub", "*=":"mul", "/=":"sdiv", "%=":"srem"}.get(node.op, "add")
                            self.emit(f"  {result} = {inst} {elem_type} {cur}, {val}")
                            self.emit(f"  store {elem_type} {result}, {elem_type}* {ptr}")
                elif isinstance(node.target.target, Identifier) and node.target.target.name in self.var_addrs:
                    arr_addr, arr_type = self.var_addrs[node.target.target.name]
                    if arr_type.startswith("["):
                        elem_type = arr_type.split("x ")[-1].rstrip("]").strip()
                        ptr = self.tmp()
                        self.emit(f"  {ptr} = getelementptr {arr_type}, {arr_type}* {arr_addr}, i32 0, {it} {idx}")
                        if node.op == "=":
                            val, _ = self.cast_if_needed(val, vt, elem_type)
                            self.emit(f"  store {elem_type} {val}, {elem_type}* {ptr}")
                        else:
                            cur = self.tmp()
                            self.emit(f"  {cur} = load {elem_type}, {elem_type}* {ptr}")
                            val, _ = self.cast_if_needed(val, vt, elem_type)
                            result = self.tmp()
                            inst = {"+=":"add", "-=":"sub", "*=":"mul", "/=":"sdiv", "%=":"srem"}.get(node.op, "add")
                            self.emit(f"  {result} = {inst} {elem_type} {cur}, {val}")
                            self.emit(f"  store {elem_type} {result}, {elem_type}* {ptr}")
                    elif arr_type.endswith("*"):
                        # Pointer indexing: load pointer, GEP, store
                        loaded = self.tmp()
                        self.emit(f"  {loaded} = load {arr_type}, {arr_type}* {arr_addr}")
                        elem_type = arr_type.rstrip("*")
                        ptr = self.tmp()
                        self.emit(f"  {ptr} = getelementptr {elem_type}, {arr_type} {loaded}, {it} {idx}")
                        val, _ = self.cast_if_needed(val, vt, elem_type)
                        self.emit(f"  store {elem_type} {val}, {elem_type}* {ptr}")
            elif isinstance(node.target, Identifier):
                addr, lt = self.var_addrs[node.target.name]
                if node.op == "=":
                    val, _ = self.cast_if_needed(val, vt, lt)
                    self.emit(f"  store {lt} {val}, {lt}* {addr}")
                else:
                    # Compound: +=, -=, *=, /=, %=
                    cur = self.tmp()
                    self.emit(f"  {cur} = load {lt}, {lt}* {addr}")
                    val, _ = self.cast_if_needed(val, vt, lt)
                    result = self.tmp()
                    if lt in ("float", "double"):
                        ops = {"+=":"fadd", "-=":"fsub", "*=":"fmul", "/=":"fdiv", "%=":"frem"}
                    else:
                        ops = {"+=":"add", "-=":"sub", "*=":"mul", "/=":"sdiv", "%=":"srem"}
                    inst = ops.get(node.op, "add")
                    self.emit(f"  {result} = {inst} {lt} {cur}, {val}")
                    self.emit(f"  store {lt} {result}, {lt}* {addr}")

        elif t == If:
            cond_val, cond_type = self.gen_expr(node.condition)
            cond_val = self.ensure_i1(cond_val, cond_type)

            then_lbl = self.label("if.then")
            end_lbl  = self.label("if.end")

            if node.else_body:
                else_lbl = self.label("if.else")
                self.emit(f"  br i1 {cond_val}, label %{then_lbl}, label %{else_lbl}")
            else:
                self.emit(f"  br i1 {cond_val}, label %{then_lbl}, label %{end_lbl}")

            # Then block
            self.emit(f"{then_lbl}:")
            self.block_terminated = False
            for s in node.then_body:
                self.gen_stmt(s)
            if not self.block_terminated:
                self.emit(f"  br label %{end_lbl}")

            # Else block
            if node.else_body:
                self.emit(f"{else_lbl}:")
                self.block_terminated = False
                for s in node.else_body:
                    self.gen_stmt(s)
                if not self.block_terminated:
                    self.emit(f"  br label %{end_lbl}")

            # Merge
            self.emit(f"{end_lbl}:")
            self.block_terminated = False

        elif t == While:
            cond_lbl = self.label("while.cond")
            body_lbl = self.label("while.body")
            end_lbl  = self.label("while.end")

            self.emit(f"  br label %{cond_lbl}")

            self.emit(f"{cond_lbl}:")
            self.block_terminated = False
            cond_val, cond_type = self.gen_expr(node.condition)
            cond_val = self.ensure_i1(cond_val, cond_type)
            self.emit(f"  br i1 {cond_val}, label %{body_lbl}, label %{end_lbl}")

            self.emit(f"{body_lbl}:")
            self.block_terminated = False
            prev_loop = self._loop_labels
            self._loop_labels = (cond_lbl, end_lbl)
            for s in node.body:
                self.gen_stmt(s)
            self._loop_labels = prev_loop
            if not self.block_terminated:
                self.emit(f"  br label %{cond_lbl}")

            self.emit(f"{end_lbl}:")
            self.block_terminated = False

        elif t == DoWhile:
            body_lbl = self.label("do.body")
            cond_lbl = self.label("do.cond")
            end_lbl  = self.label("do.end")

            self.emit(f"  br label %{body_lbl}")

            self.emit(f"{body_lbl}:")
            self.block_terminated = False
            prev_loop = self._loop_labels
            self._loop_labels = (cond_lbl, end_lbl)
            for s in node.body:
                self.gen_stmt(s)
            self._loop_labels = prev_loop
            if not self.block_terminated:
                self.emit(f"  br label %{cond_lbl}")

            self.emit(f"{cond_lbl}:")
            self.block_terminated = False
            cond_val, cond_type = self.gen_expr(node.condition)
            cond_val = self.ensure_i1(cond_val, cond_type)
            self.emit(f"  br i1 {cond_val}, label %{body_lbl}, label %{end_lbl}")

            self.emit(f"{end_lbl}:")
            self.block_terminated = False

        elif t == For:
            # Generator for-each → callback pattern
            if isinstance(node.iterable, Call) and node.iterable.name in self._generators:
                yield_type = self._generators[node.iterable.name]
                cb_name = self._gen_llvm_callback(node.var, yield_type, node.body)
                yield_lt = self.llvm_type(yield_type)
                # Evaluate generator call args
                args = []
                for arg in node.iterable.args:
                    val, vt = self.gen_expr(arg)
                    args.append((val, vt))
                arg_str = ", ".join(f"{vt} {val}" for val, vt in args)
                if arg_str:
                    arg_str += f", void ({yield_lt})* @{cb_name}"
                else:
                    arg_str = f"void ({yield_lt})* @{cb_name}"
                self.emit(f"  call void @{node.iterable.name}({arg_str})")
            else:
                # for each var in iterable — indexed loop over fixed-size stack array
                iterable_name = node.iterable.name if isinstance(node.iterable, Identifier) else None
                if iterable_name and iterable_name in self.var_array_info:
                    elem_lt, count = self.var_array_info[iterable_name]
                    arr_addr, arr_type = self.var_addrs[iterable_name]

                    # Alloca loop counter
                    counter_addr = f"%{node.var}._i.addr"
                    self.emit(f"  {counter_addr} = alloca i32")
                    self.emit(f"  store i32 0, i32* {counter_addr}")

                    # Alloca loop variable
                    var_addr = f"%{node.var}.addr"
                    self.emit(f"  {var_addr} = alloca {elem_lt}")
                    self.var_addrs[node.var] = (var_addr, elem_lt)
                    arr_type_node = self.var_types.get(iterable_name)
                    self.var_types[node.var] = TypeName(arr_type_node.name) if arr_type_node else TypeName("int")

                    cond_lbl = self.label("for.cond")
                    body_lbl = self.label("for.body")
                    end_lbl  = self.label("for.end")

                    self.emit(f"  br label %{cond_lbl}")

                    # Condition: counter < count
                    self.emit(f"{cond_lbl}:")
                    self.block_terminated = False
                    idx = self.tmp()
                    self.emit(f"  {idx} = load i32, i32* {counter_addr}")
                    cmp = self.tmp()
                    self.emit(f"  {cmp} = icmp slt i32 {idx}, {count}")
                    self.emit(f"  br i1 {cmp}, label %{body_lbl}, label %{end_lbl}")

                    # Body: load element, run body stmts, increment
                    self.emit(f"{body_lbl}:")
                    self.block_terminated = False
                    idx2 = self.tmp()
                    self.emit(f"  {idx2} = load i32, i32* {counter_addr}")
                    elem_ptr = self.tmp()
                    self.emit(f"  {elem_ptr} = getelementptr {arr_type}, {arr_type}* {arr_addr}, i32 0, i32 {idx2}")
                    elem_val = self.tmp()
                    self.emit(f"  {elem_val} = load {elem_lt}, {elem_lt}* {elem_ptr}")
                    self.emit(f"  store {elem_lt} {elem_val}, {elem_lt}* {var_addr}")

                    prev_loop = self._loop_labels
                    self._loop_labels = (cond_lbl, end_lbl)
                    for s in node.body:
                        self.gen_stmt(s)
                    self._loop_labels = prev_loop

                    if not self.block_terminated:
                        inc_idx = self.tmp()
                        self.emit(f"  {inc_idx} = load i32, i32* {counter_addr}")
                        inc_val = self.tmp()
                        self.emit(f"  {inc_val} = add i32 {inc_idx}, 1")
                        self.emit(f"  store i32 {inc_val}, i32* {counter_addr}")
                        self.emit(f"  br label %{cond_lbl}")

                    self.emit(f"{end_lbl}:")
                    self.block_terminated = False
                else:
                    self.emit(f"  ; for-each: unsupported iterable")

        elif t == Return:
            if node.value is not None and isinstance(node.value, OkResult):
                payload_type = self.current_func.return_type if self.current_func.return_type else TypeName("int")
                rtype = self._result_struct_name(payload_type)
                payload_lt = self.llvm_type(payload_type)
                val, vt = self.gen_expr(node.value.value)
                val, _ = self.cast_if_needed(val, vt, payload_lt)
                # Build { i1 1, payload val, i8* null }
                empty_str = self._get_empty_string()
                r1 = self.tmp()
                self.emit(f"  {r1} = insertvalue {rtype} undef, i1 1, 0")
                r2 = self.tmp()
                self.emit(f"  {r2} = insertvalue {rtype} {r1}, {payload_lt} {val}, 1")
                r3 = self.tmp()
                self.emit(f"  {r3} = insertvalue {rtype} {r2}, i8* {empty_str}, 2")
                self.emit(f"  ret {rtype} {r3}")
                self.block_terminated = True
            elif node.value is not None and isinstance(node.value, ErrorResult):
                payload_type = self.current_func.return_type if self.current_func.return_type else TypeName("int")
                rtype = self._result_struct_name(payload_type)
                payload_lt = self.llvm_type(payload_type)
                err_val, err_vt = self.gen_expr(node.value.value)
                if err_vt != "i8*":
                    err_val, _ = self.cast_if_needed(err_val, err_vt, "i8*")
                r1 = self.tmp()
                self.emit(f"  {r1} = insertvalue {rtype} undef, i1 0, 0")
                r2 = self.tmp()
                zero = "0" if payload_lt in self.INT_TYPES else ("0.0" if payload_lt in self.FLOAT_TYPES else "null")
                self.emit(f"  {r2} = insertvalue {rtype} {r1}, {payload_lt} {zero}, 1")
                r3 = self.tmp()
                self.emit(f"  {r3} = insertvalue {rtype} {r2}, i8* {err_val}, 2")
                self.emit(f"  ret {rtype} {r3}")
                self.block_terminated = True
            else:
                if self.current_func.returns_result:
                    payload_type = self.current_func.return_type if self.current_func.return_type else TypeName("int")
                    ret_type = self._result_struct_name(payload_type)
                else:
                    ret_type = self.llvm_type(self.current_func.return_type)
                if node.value is not None:
                    val, vt = self.gen_expr(node.value)
                    val, _ = self.cast_if_needed(val, vt, ret_type)
                    self.emit(f"  ret {ret_type} {val}")
                else:
                    self.emit("  ret void")
                self.block_terminated = True

        elif t == Print:
            self.gen_print(node.value)

        elif t == ExprStmt:
            self.gen_expr(node.expr)

        elif t == Escape:
            if self._loop_labels:
                self.emit(f"  br label %{self._loop_labels[1]}")
                dead = self.label("escape.after")
                self.emit(f"{dead}:")
                self.block_terminated = False
            else:
                self.emit("  ; escape outside loop")

        elif t == Continue:
            if self._loop_labels:
                self.emit(f"  br label %{self._loop_labels[0]}")
                dead = self.label("continue.after")
                self.emit(f"{dead}:")
                self.block_terminated = False
            else:
                self.emit("  ; continue outside loop")

        elif t == Yield:
            val, vt = self.gen_expr(node.value)
            yield_lt = self.llvm_type(self.current_func.return_type)
            val, _ = self.cast_if_needed(val, vt, yield_lt)
            self.emit(f"  call void %_yield_cb({yield_lt} {val})")

        elif t == Match:
            self.gen_match(node)

        elif t == PostIncrement or t == PostDecrement:
            # x++ as statement: load, add/sub 1, store back
            if isinstance(node.operand, Identifier) and node.operand.name in self.var_addrs:
                addr, lt = self.var_addrs[node.operand.name]
                cur = self.tmp()
                self.emit(f"  {cur} = load {lt}, {lt}* {addr}")
                result = self.tmp()
                if lt in ("float", "double"):
                    op = "fadd" if t == PostIncrement else "fsub"
                    self.emit(f"  {result} = {op} {lt} {cur}, 1.0")
                else:
                    op = "add" if t == PostIncrement else "sub"
                    self.emit(f"  {result} = {op} {lt} {cur}, 1")
                self.emit(f"  store {lt} {result}, {lt}* {addr}")

    # ── Generator callback ────────────────────────────────────────────

    def _gen_llvm_callback(self, loop_var, yield_type, body_stmts):
        """Generate a separate LLVM function for a generator for-each body."""
        cb_name = f"_gen_body_{self._cb_counter}"
        self._cb_counter += 1

        # Save all mutable state
        saved = (self.lines, self.tmp_counter, self.label_counter,
                 self.var_addrs, dict(self.var_types), self.block_terminated,
                 self.current_func, self._loop_labels)

        # Generate callback as a standalone function
        self.lines = []
        self.tmp_counter = 0
        self.label_counter = 0
        self.var_addrs = {}
        self.block_terminated = False
        self._loop_labels = None

        cb_decl = FunctionDecl(cb_name, [Param(yield_type, loop_var)], None, body_stmts)
        self.current_func = cb_decl
        self.gen_function(cb_decl)

        self._gen_callbacks.extend(self.lines)

        # Restore state
        (self.lines, self.tmp_counter, self.label_counter,
         self.var_addrs, self.var_types, self.block_terminated,
         self.current_func, self._loop_labels) = saved

        return cb_name

    # ── Match ─────────────────────────────────────────────────────────

    def gen_match(self, node):
        subj_val, subj_lt = self.gen_expr(node.subject)
        end_lbl = self.label("match.end")

        # Check if all non-default cases are int literals → use switch
        non_default = [c for c in node.cases if c.value is not None]
        default_case = next((c for c in node.cases if c.value is None), None)
        all_int = all(isinstance(c.value, IntLiteral) for c in non_default)

        if all_int and subj_lt in self.INT_TYPES:
            # LLVM switch instruction
            default_lbl = self.label("match.default") if default_case else end_lbl
            case_labels = []
            for c in non_default:
                lbl = self.label("match.case")
                case_labels.append((c.value.value, lbl, c))

            arms = " ".join(f"{subj_lt} {val}, label %{lbl}" for val, lbl, _ in case_labels)
            self.emit(f"  switch {subj_lt} {subj_val}, label %{default_lbl} [ {arms} ]")

            for val, lbl, case in case_labels:
                self.emit(f"{lbl}:")
                self.block_terminated = False
                for s in case.body:
                    self.gen_stmt(s)
                if not self.block_terminated:
                    self.emit(f"  br label %{end_lbl}")

            if default_case:
                self.emit(f"{default_lbl}:")
                self.block_terminated = False
                for s in default_case.body:
                    self.gen_stmt(s)
                if not self.block_terminated:
                    self.emit(f"  br label %{end_lbl}")

            self.emit(f"{end_lbl}:")
            self.block_terminated = False
        else:
            # Fallback: br chain with separate comparison blocks
            case_info = []
            for c in non_default:
                check_lbl = self.label("match.check")
                body_lbl = self.label("match.case")
                case_info.append((c, check_lbl, body_lbl))
            default_lbl = self.label("match.default") if default_case else None

            # Jump to first comparison block
            if case_info:
                self.emit(f"  br label %{case_info[0][0 + 1]}")
            elif default_lbl:
                self.emit(f"  br label %{default_lbl}")
            else:
                self.emit(f"  br label %{end_lbl}")

            for i, (case, check_lbl, body_lbl) in enumerate(case_info):
                # Comparison block
                self.emit(f"{check_lbl}:")
                self.block_terminated = False
                cv, ct = self.gen_expr(case.value)
                cv, ct = self.cast_if_needed(cv, ct, subj_lt)
                cmp = self.tmp()
                if subj_lt in self.FLOAT_TYPES:
                    self.emit(f"  {cmp} = fcmp oeq {subj_lt} {subj_val}, {cv}")
                else:
                    self.emit(f"  {cmp} = icmp eq {subj_lt} {subj_val}, {cv}")
                next_lbl = case_info[i + 1][1] if i + 1 < len(case_info) else (default_lbl or end_lbl)
                self.emit(f"  br i1 {cmp}, label %{body_lbl}, label %{next_lbl}")

                # Body block
                self.emit(f"{body_lbl}:")
                self.block_terminated = False
                for s in case.body:
                    self.gen_stmt(s)
                if not self.block_terminated:
                    self.emit(f"  br label %{end_lbl}")

            if default_case:
                self.emit(f"{default_lbl}:")
                self.block_terminated = False
                for s in default_case.body:
                    self.gen_stmt(s)
                if not self.block_terminated:
                    self.emit(f"  br label %{end_lbl}")

            self.emit(f"{end_lbl}:")
            self.block_terminated = False

    # ── Print ────────────────────────────────────────────────────────────

    def gen_print(self, node):
        """Emit a printf call for a print statement.
        For string concatenation (print "text" + cast(val, string) + ...),
        decompose into a single printf with format specifiers instead of
        trying to concatenate strings at the LLVM IR level.
        """
        self.needs_printf = True

        # Try to decompose string concatenation into printf format + args
        fmt_parts, args = self._decompose_print(node)
        if fmt_parts is not None:
            # Build format string: "part1%dpart2%s\n"
            fmt_str = "".join(fmt_parts) + "\\0A"  # \n
            # Use C-style \n
            fmt_str = "".join(fmt_parts) + "\n"
            name, length = self.add_string(fmt_str)
            ptr = self.tmp()
            self.emit(f"  {ptr} = getelementptr [{length} x i8], [{length} x i8]* {name}, i32 0, i32 0")
            # Build arg string for printf call
            arg_str = ""
            for val, vt in args:
                # Promote float to double for variadic
                if vt == "float":
                    promoted = self.tmp()
                    self.emit(f"  {promoted} = fpext float {val} to double")
                    val = promoted
                    vt = "double"
                # Promote i1 to i32
                if vt == "i1":
                    promoted = self.tmp()
                    self.emit(f"  {promoted} = zext i1 {val} to i32")
                    val = promoted
                    vt = "i32"
                arg_str += f", {vt} {val}"
            result = self.tmp()
            self.emit(f"  {result} = call i32 (i8*, ...) @printf(i8* {ptr}{arg_str})")
            return

        # Simple case — not a concatenation
        val, vt = self.gen_expr(node)
        fmt_spec = LLVM_FMT.get(vt, "%d")
        if vt == "float":
            promoted = self.tmp()
            self.emit(f"  {promoted} = fpext float {val} to double")
            val = promoted
            vt = "double"
        if vt == "i1":
            promoted = self.tmp()
            self.emit(f"  {promoted} = zext i1 {val} to i32")
            val = promoted
            vt = "i32"
        fmt_str = fmt_spec + "\n"
        name, length = self.add_string(fmt_str)
        ptr = self.tmp()
        self.emit(f"  {ptr} = getelementptr [{length} x i8], [{length} x i8]* {name}, i32 0, i32 0")
        result = self.tmp()
        self.emit(f"  {result} = call i32 (i8*, ...) @printf(i8* {ptr}, {vt} {val})")

    def _decompose_print(self, node):
        """Decompose a string concatenation tree into printf format parts and args.
        Returns (fmt_parts, args) or (None, None) if not a concat tree.
        fmt_parts: list of strings (literal text and format specifiers)
        args: list of (register, llvm_type) for each format specifier
        """
        parts = []
        args = []
        self._flatten_concat(node, parts, args)
        if not parts and not args:
            return None, None
        # Check if it's actually a concatenation (has format specifiers)
        if not args and len(parts) == 1:
            return None, None  # just a plain string, use simple path
        return parts, args

    def _flatten_concat(self, node, parts, args):
        """Recursively flatten a BinaryOp("+", ...) tree into format parts and args."""
        from ast_nodes import BinaryOp, StringLiteral, Cast, Identifier, IntLiteral, FloatLiteral, Call

        if isinstance(node, BinaryOp) and node.op == "+":
            self._flatten_concat(node.left, parts, args)
            self._flatten_concat(node.right, parts, args)
        elif isinstance(node, StringLiteral):
            # Literal text — escape % for printf
            parts.append(node.value.replace("%", "%%"))
        elif isinstance(node, Cast) and node.target_type.name == "string":
            # cast(expr, string) — evaluate the inner expr and add format specifier
            inner = node.expr
            if isinstance(inner, Call) and inner.name == "change" and len(inner.args) == 1:
                inner = inner.args[0]
            val, vt = self.gen_expr(inner)
            # Check UL type for unsigned — LLVM IR doesn't distinguish
            fmt = LLVM_FMT.get(vt, "%d")
            if isinstance(inner, Identifier):
                ul_type = self.var_types.get(inner.name)
                if ul_type and ul_type.name in ("uint8", "uint16", "uint32", "uint64"):
                    if vt == "i64":
                        fmt = "%llu"
                    else:
                        fmt = "%u"
            parts.append(fmt)
            args.append((val, vt))
        elif isinstance(node, Identifier):
            # Could be a string variable
            val, vt = self.gen_expr(node)
            if vt == "i8*" or vt.endswith("*"):
                parts.append("%s")
                args.append((val, vt))
            else:
                fmt = LLVM_FMT.get(vt, "%d")
                parts.append(fmt)
                args.append((val, vt))
        else:
            # Unknown node — evaluate and use %d
            val, vt = self.gen_expr(node)
            fmt = LLVM_FMT.get(vt, "%d")
            parts.append(fmt)
            args.append((val, vt))

    # ── Expressions ──────────────────────────────────────────────────────
    # Each returns (register_or_literal, llvm_type)

    def gen_expr(self, node):
        t = type(node)

        if t == IntLiteral:
            v = node.value
            # Use i64 for values outside signed i32 range
            if v < -2147483648 or v > 2147483647:
                return str(v), "i64"
            return str(v), "i32"

        if t == FloatLiteral:
            # LLVM accepts decimal float literals; default to double (C convention)
            return repr(node.value), "double"

        if t == StringLiteral:
            name, length = self.add_string(node.value)
            ptr = self.tmp()
            self.emit(f"  {ptr} = getelementptr [{length} x i8], [{length} x i8]* {name}, i32 0, i32 0")
            return ptr, "i8*"

        if t == BoolLiteral:
            return ("1" if node.value else "0"), "i1"

        if t == EmptyLiteral:
            return "null", "i8*"

        if t == ArrayComprehension:
            # Array comprehensions in LLVM IR: not yet supported
            # Falls back to an empty array pointer
            self.emit(f"  ; TODO: array comprehension not yet supported in LLVM target")
            return "null", "i8*"

        if t == Identifier:
            if node.name not in self.var_addrs:
                return f"@{node.name}", "i32"  # shouldn't happen in valid UL
            addr, lt = self.var_addrs[node.name]
            reg = self.tmp()
            self.emit(f"  {reg} = load {lt}, {lt}* {addr}")
            return reg, lt

        if t == BinaryOp:
            return self.gen_binop(node)

        if t == UnaryOp:
            return self.gen_unaryop(node)

        if t == Cast:
            return self.gen_cast(node)

        if t == Call:
            return self.gen_call(node)

        if t == Index:
            idx, it = self.gen_expr(node.index)
            # For array indexing, use the alloca address directly (don't load the array)
            if isinstance(node.target, Identifier) and node.target.name in self.var_addrs:
                addr, addr_type = self.var_addrs[node.target.name]
                # addr_type might be [N x i32] for arrays — get element type
                if addr_type.startswith("["):
                    # [N x elem_type] → extract elem_type
                    elem_type = addr_type.split("x ")[-1].rstrip("]").strip()
                    ptr = self.tmp()
                    self.emit(f"  {ptr} = getelementptr {addr_type}, {addr_type}* {addr}, i32 0, {it} {idx}")
                    reg = self.tmp()
                    self.emit(f"  {reg} = load {elem_type}, {elem_type}* {ptr}")
                    return reg, elem_type
                elif addr_type.endswith("*"):
                    # Pointer — load it first, then GEP
                    loaded = self.tmp()
                    self.emit(f"  {loaded} = load {addr_type}, {addr_type}* {addr}")
                    elem_type = addr_type.rstrip("*")
                    ptr = self.tmp()
                    self.emit(f"  {ptr} = getelementptr {elem_type}, {addr_type} {loaded}, {it} {idx}")
                    reg = self.tmp()
                    self.emit(f"  {reg} = load {elem_type}, {elem_type}* {ptr}")
                    return reg, elem_type
            # FieldAccess target: s.data[i] — get field pointer, then GEP
            if isinstance(node.target, FieldAccess):
                field_ptr, field_lt = self.gen_field_ptr(node.target)
                if field_lt.startswith("["):
                    # Array field — GEP into it
                    elem_type = field_lt.split("x ")[-1].rstrip("]").strip()
                    ptr = self.tmp()
                    self.emit(f"  {ptr} = getelementptr {field_lt}, {field_lt}* {field_ptr}, i32 0, {it} {idx}")
                    reg = self.tmp()
                    self.emit(f"  {reg} = load {elem_type}, {elem_type}* {ptr}")
                    return reg, elem_type
                elif field_lt.endswith("*"):
                    # Pointer field — load pointer, then GEP
                    loaded = self.tmp()
                    self.emit(f"  {loaded} = load {field_lt}, {field_lt}* {field_ptr}")
                    elem_type = field_lt.rstrip("*")
                    ptr = self.tmp()
                    self.emit(f"  {ptr} = getelementptr {elem_type}, {field_lt} {loaded}, {it} {idx}")
                    reg = self.tmp()
                    self.emit(f"  {reg} = load {elem_type}, {elem_type}* {ptr}")
                    return reg, elem_type
            # Fallback: evaluate target normally
            base, bt = self.gen_expr(node.target)
            elem_type = bt.rstrip("*") if bt.endswith("*") else bt
            ptr = self.tmp()
            self.emit(f"  {ptr} = getelementptr {elem_type}, {bt} {base}, {it} {idx}")
            reg = self.tmp()
            self.emit(f"  {reg} = load {elem_type}, {elem_type}* {ptr}")
            return reg, elem_type

        if t == FieldAccess:
            return self.gen_field_load(node)

        if t == PostIncrement or t == PostDecrement:
            # In expression context: return old value, then increment
            if isinstance(node.operand, Identifier) and node.operand.name in self.var_addrs:
                addr, lt = self.var_addrs[node.operand.name]
                old = self.tmp()
                self.emit(f"  {old} = load {lt}, {lt}* {addr}")
                result = self.tmp()
                if lt in ("float", "double"):
                    op = "fadd" if t == PostIncrement else "fsub"
                    self.emit(f"  {result} = {op} {lt} {old}, 1.0")
                else:
                    op = "add" if t == PostIncrement else "sub"
                    self.emit(f"  {result} = {op} {lt} {old}, 1")
                self.emit(f"  store {lt} {result}, {lt}* {addr}")
                return old, lt

        if t == OkResult:
            payload_type = self.current_func.return_type if self.current_func.return_type else TypeName("int")
            rtype = self._result_struct_name(payload_type)
            payload_lt = self.llvm_type(payload_type)
            val, vt = self.gen_expr(node.value)
            val, _ = self.cast_if_needed(val, vt, payload_lt)
            empty_str = self._get_empty_string()
            r1 = self.tmp()
            self.emit(f"  {r1} = insertvalue {rtype} undef, i1 1, 0")
            r2 = self.tmp()
            self.emit(f"  {r2} = insertvalue {rtype} {r1}, {payload_lt} {val}, 1")
            r3 = self.tmp()
            self.emit(f"  {r3} = insertvalue {rtype} {r2}, i8* {empty_str}, 2")
            return r3, rtype

        if t == ErrorResult:
            payload_type = self.current_func.return_type if self.current_func.return_type else TypeName("int")
            rtype = self._result_struct_name(payload_type)
            payload_lt = self.llvm_type(payload_type)
            err_val, err_vt = self.gen_expr(node.value)
            if err_vt != "i8*":
                err_val, _ = self.cast_if_needed(err_val, err_vt, "i8*")
            r1 = self.tmp()
            self.emit(f"  {r1} = insertvalue {rtype} undef, i1 0, 0")
            r2 = self.tmp()
            zero = "0" if payload_lt in self.INT_TYPES else ("0.0" if payload_lt in self.FLOAT_TYPES else "null")
            self.emit(f"  {r2} = insertvalue {rtype} {r1}, {payload_lt} {zero}, 1")
            r3 = self.tmp()
            self.emit(f"  {r3} = insertvalue {rtype} {r2}, i8* {err_val}, 2")
            return r3, rtype

        if t == ResultPropagation:
            inner_val, inner_vt = self.gen_expr(node.expr)
            call_name = node.expr.name if isinstance(node.expr, Call) else None
            payload_type = self._result_funcs.get(call_name, TypeName("int"))
            rtype = self._result_struct_name(payload_type)
            payload_lt = self.llvm_type(payload_type)
            ok_flag = self.tmp()
            self.emit(f"  {ok_flag} = extractvalue {rtype} {inner_val}, 0")
            then_lbl = self.label("prop.ok")
            err_lbl = self.label("prop.err")
            self.emit(f"  br i1 {ok_flag}, label %{then_lbl}, label %{err_lbl}")
            # Error path
            self.emit(f"{err_lbl}:")
            self.block_terminated = False
            err_msg = self.tmp()
            self.emit(f"  {err_msg} = extractvalue {rtype} {inner_val}, 2")
            if self.current_func.returns_result:
                # Propagate: build error result for caller
                caller_payload = self.current_func.return_type if self.current_func.return_type else TypeName("int")
                caller_rtype = self._result_struct_name(caller_payload)
                caller_payload_lt = self.llvm_type(caller_payload)
                e1 = self.tmp()
                self.emit(f"  {e1} = insertvalue {caller_rtype} undef, i1 0, 0")
                e2 = self.tmp()
                zero = "0" if caller_payload_lt in self.INT_TYPES else ("0.0" if caller_payload_lt in self.FLOAT_TYPES else "null")
                self.emit(f"  {e2} = insertvalue {caller_rtype} {e1}, {caller_payload_lt} {zero}, 1")
                e3 = self.tmp()
                self.emit(f"  {e3} = insertvalue {caller_rtype} {e2}, i8* {err_msg}, 2")
                self.emit(f"  ret {caller_rtype} {e3}")
            else:
                # Boundary: fprintf(stderr, "error: %s\n", msg); exit(1)
                self._needs_exit = True
                fmt_str = "error: %s\n"
                fname, flen = self.add_string(fmt_str)
                fptr = self.tmp()
                self.emit(f"  {fptr} = getelementptr [{flen} x i8], [{flen} x i8]* {fname}, i32 0, i32 0")
                self._needs_stderr = True
                stderr_load = self.tmp()
                self.emit(f"  {stderr_load} = load i8*, i8** @stderr")
                fp_result = self.tmp()
                self.emit(f"  {fp_result} = call i32 (i8*, i8*, ...) @fprintf(i8* {stderr_load}, i8* {fptr}, i8* {err_msg})")
                self.emit(f"  call void @exit(i32 1)")
                self.emit(f"  unreachable")
            # Ok path
            self.emit(f"{then_lbl}:")
            self.block_terminated = False
            ok_val = self.tmp()
            self.emit(f"  {ok_val} = extractvalue {rtype} {inner_val}, 1")
            return ok_val, payload_lt

        return "0", "i32"

    # ── Binary operations ────────────────────────────────────────────────

    def gen_binop(self, node):
        left, lt = self.gen_expr(node.left)
        right, rt = self.gen_expr(node.right)

        # Promote operands to matching types
        # BUT: skip promotion for pointer arithmetic (ptr + int or ptr - ptr)
        is_ptr_arith = (lt.endswith("*") or lt == "ptr" or rt.endswith("*") or rt == "ptr")
        if lt != rt and not is_ptr_arith:
            left, lt, right, rt = self.promote_binop(left, lt, right, rt)

        is_float = lt in ("float", "double")
        result = self.tmp()
        op = node.op

        if op == "+":
            # Pointer + integer: getelementptr
            if lt.endswith("*") or lt == "ptr":
                elem_type = lt.rstrip("*").strip() or "i8"
                self.emit(f"  {result} = getelementptr {elem_type}, {lt} {left}, {rt} {right}")
                return result, lt
            if rt.endswith("*") or rt == "ptr":
                elem_type = rt.rstrip("*").strip() or "i8"
                self.emit(f"  {result} = getelementptr {elem_type}, {rt} {right}, {lt} {left}")
                return result, rt
            self.emit(f"  {result} = {'fadd' if is_float else 'add'} {lt} {left}, {right}")
            return result, lt
        if op == "-":
            if lt.endswith("*") or lt == "ptr":
                # Pointer subtraction: ptrtoint both, sub, then result is integer
                tmp1 = self.tmp()
                tmp2 = self.tmp()
                self.emit(f"  {tmp1} = ptrtoint {lt} {left} to i64")
                self.emit(f"  {tmp2} = ptrtoint {lt} {right} to i64")
                self.emit(f"  {result} = sub i64 {tmp1}, {tmp2}")
                return result, "i64"
            self.emit(f"  {result} = {'fsub' if is_float else 'sub'} {lt} {left}, {right}")
            return result, lt
        if op == "*":
            self.emit(f"  {result} = {'fmul' if is_float else 'mul'} {lt} {left}, {right}")
            return result, lt
        if op == "/":
            self.emit(f"  {result} = {'fdiv' if is_float else 'sdiv'} {lt} {left}, {right}")
            return result, lt
        if op == "%":
            self.emit(f"  {result} = {'frem' if is_float else 'srem'} {lt} {left}, {right}")
            return result, lt

        if op in ("==", "!=", "<", ">", "<=", ">="):
            icmp_map = {"==":"eq", "!=":"ne", "<":"slt", ">":"sgt", "<=":"sle", ">=":"sge"}
            fcmp_map = {"==":"oeq", "!=":"one", "<":"olt", ">":"ogt", "<=":"ole", ">=":"oge"}
            if is_float:
                self.emit(f"  {result} = fcmp {fcmp_map[op]} {lt} {left}, {right}")
            else:
                self.emit(f"  {result} = icmp {icmp_map[op]} {lt} {left}, {right}")
            return result, "i1"

        if op == "and":
            left = self.ensure_i1(left, lt)
            right = self.ensure_i1(right, rt)
            self.emit(f"  {result} = and i1 {left}, {right}")
            return result, "i1"
        if op == "or":
            left = self.ensure_i1(left, lt)
            right = self.ensure_i1(right, rt)
            self.emit(f"  {result} = or i1 {left}, {right}")
            return result, "i1"

        # Bitwise operations
        if op == "both1":
            self.emit(f"  {result} = and {lt} {left}, {right}")
            return result, lt
        if op == "either1":
            self.emit(f"  {result} = or {lt} {left}, {right}")
            return result, lt
        if op == "diff":
            self.emit(f"  {result} = xor {lt} {left}, {right}")
            return result, lt
        if op == "left":
            self.emit(f"  {result} = shl {lt} {left}, {right}")
            return result, lt
        if op == "right":
            self.emit(f"  {result} = lshr {lt} {left}, {right}")
            return result, lt

        self.emit(f"  ; unknown binop '{op}'")
        return "0", "i32"

    # ── Unary operations ─────────────────────────────────────────────────

    def gen_unaryop(self, node):
        # Handle address-of without loading first
        if node.op == "address":
            if isinstance(node.operand, Identifier) and node.operand.name in self.var_addrs:
                addr, lt = self.var_addrs[node.operand.name]
                return addr, lt + "*"

        val, vt = self.gen_expr(node.operand)
        result = self.tmp()

        if node.op == "-":
            if vt in ("float", "double"):
                self.emit(f"  {result} = fneg {vt} {val}")
            else:
                self.emit(f"  {result} = sub {vt} 0, {val}")
            return result, vt

        if node.op == "not":
            val = self.ensure_i1(val, vt)
            self.emit(f"  {result} = xor i1 {val}, 1")
            return result, "i1"

        if node.op == "bitflip":
            self.emit(f"  {result} = xor {vt} {val}, -1")
            return result, vt

        if node.op == "deref":
            if vt.endswith("*"):
                elem = vt[:-1]
                self.emit(f"  {result} = load {elem}, {vt} {val}")
                return result, elem
            return val, vt

        # address fallback (non-identifier operand)
        if node.op == "address":
            return val, vt + "*"

        return val, vt

    # ── Cast ─────────────────────────────────────────────────────────────

    def gen_cast(self, node):
        # Unwrap change() wrapper: change(x)->type → cast x to type
        inner = node.expr
        if isinstance(inner, Call) and inner.name == "change" and len(inner.args) == 1:
            inner = inner.args[0]

        val, vt = self.gen_expr(inner)
        target = self.llvm_type(node.target_type)

        if vt == target:
            return val, vt

        return self.cast_if_needed(val, vt, target)

    # ── Field access ──────────────────────────────────────────────────────

    def gen_field_ptr(self, node):
        """Return (pointer_reg, field_llvm_type) for a FieldAccess node."""
        if isinstance(node.target, Identifier):
            base_name = node.target.name
            if base_name not in self.var_addrs:
                return "null", "i32"
            addr, struct_lt = self.var_addrs[base_name]
            # If it's a pointer to struct (%Stack*), load the pointer first
            if struct_lt.endswith("*") and struct_lt.startswith("%"):
                inner_type = struct_lt.rstrip("*")
                loaded = self.tmp()
                self.emit(f"  {loaded} = load {struct_lt}, {struct_lt}* {addr}")
                type_name = inner_type.lstrip("%")
                layout = self.struct_layouts.get(type_name, [])
                for idx, (fname, flt) in enumerate(layout):
                    if fname == node.field:
                        ptr = self.tmp()
                        self.emit(f"  {ptr} = getelementptr {inner_type}, {struct_lt} {loaded}, i32 0, i32 {idx}")
                        return ptr, flt
                return "null", "i32"
            type_name = struct_lt.lstrip("%")
        elif isinstance(node.target, FieldAccess):
            addr, inner_lt = self.gen_field_ptr(node.target)
            type_name = inner_lt.lstrip("%")
        elif isinstance(node.target, Index):
            # pool[i].field — index into an array, get pointer to struct element
            idx_val, idx_type = self.gen_expr(node.target.index)
            if isinstance(node.target.target, FieldAccess):
                # l.pool[i] — get pointer to the array field, then GEP to element
                arr_ptr, arr_lt = self.gen_field_ptr(node.target.target)
                if arr_lt.startswith("["):
                    # [N x %Struct] — get element pointer
                    elem_type = arr_lt.split("x ")[-1].rstrip("]").strip()
                    elem_ptr = self.tmp()
                    self.emit(f"  {elem_ptr} = getelementptr {arr_lt}, {arr_lt}* {arr_ptr}, i32 0, {idx_type} {idx_val}")
                    type_name = elem_type.lstrip("%")
                    # Now find the field in this struct
                    layout = self.struct_layouts.get(type_name, [])
                    for field_idx, (fname, flt) in enumerate(layout):
                        if fname == node.field:
                            ptr = self.tmp()
                            self.emit(f"  {ptr} = getelementptr {elem_type}, {elem_type}* {elem_ptr}, i32 0, i32 {field_idx}")
                            return ptr, flt
                    return "null", "i32"
            elif isinstance(node.target.target, Identifier) and node.target.target.name in self.var_addrs:
                arr_addr, arr_type = self.var_addrs[node.target.target.name]
                if arr_type.startswith("["):
                    elem_type = arr_type.split("x ")[-1].rstrip("]").strip()
                    elem_ptr = self.tmp()
                    self.emit(f"  {elem_ptr} = getelementptr {arr_type}, {arr_type}* {arr_addr}, i32 0, {idx_type} {idx_val}")
                    type_name = elem_type.lstrip("%")
                    layout = self.struct_layouts.get(type_name, [])
                    for field_idx, (fname, flt) in enumerate(layout):
                        if fname == node.field:
                            ptr = self.tmp()
                            self.emit(f"  {ptr} = getelementptr {elem_type}, {elem_type}* {elem_ptr}, i32 0, i32 {field_idx}")
                            return ptr, flt
                    return "null", "i32"
            return "null", "i32"
        else:
            return "null", "i32"

        layout = self.struct_layouts.get(type_name, [])
        for idx, (fname, flt) in enumerate(layout):
            if fname == node.field:
                ptr = self.tmp()
                self.emit(f"  {ptr} = getelementptr %{type_name}, %{type_name}* {addr}, i32 0, i32 {idx}")
                return ptr, flt
        return "null", "i32"

    def gen_field_load(self, node):
        """Load a struct field value. Returns (reg, llvm_type).
        For array fields, returns a pointer (arrays can't be loaded as values for indexing).
        """
        ptr, field_lt = self.gen_field_ptr(node)
        if field_lt.startswith("["):
            # Array field — return pointer to first element for indexing
            elem_type = field_lt.split("x ")[-1].rstrip("]").strip()
            gep = self.tmp()
            self.emit(f"  {gep} = getelementptr {field_lt}, {field_lt}* {ptr}, i32 0, i32 0")
            return gep, elem_type + "*"
        reg = self.tmp()
        self.emit(f"  {reg} = load {field_lt}, {field_lt}* {ptr}")
        return reg, field_lt

    # ── Function call ────────────────────────────────────────────────────

    def gen_call(self, node):
        name = BUILTIN_MAP.get(node.name, node.name)

        # change() standalone (without ->) is identity
        if node.name == "change" and len(node.args) == 1:
            return self.gen_expr(node.args[0])

        # Evaluate arguments
        args = []
        for arg in node.args:
            # For array identifiers, pass pointer to first element directly (no copy)
            if isinstance(arg, Identifier) and arg.name in self.var_addrs:
                addr, addr_type = self.var_addrs[arg.name]
                if addr_type.startswith("["):
                    elem_type = addr_type.split("x ")[-1].rstrip("]").strip()
                    ptr = self.tmp()
                    self.emit(f"  {ptr} = getelementptr {addr_type}, {addr_type}* {addr}, i32 0, i32 0")
                    args.append((ptr, elem_type + "*"))
                    continue
            val, vt = self.gen_expr(arg)
            args.append((val, vt))

        # Look up return type: user functions first, then builtins
        sig = self.func_sigs.get(name) or self.func_sigs.get(node.name)
        if not sig and name in BUILTIN_SIGS:
            sig = BUILTIN_SIGS[name]
            self.used_builtins.add(name)
        ret_type = sig[0] if sig else "i32"

        # Cast arguments to match signature parameter types if available
        if sig:
            param_types = sig[1]
            cast_args = []
            for i, (val, vt) in enumerate(args):
                if i < len(param_types) and vt != param_types[i]:
                    val, _ = self.cast_if_needed(val, vt, param_types[i])
                    cast_args.append((val, param_types[i]))
                else:
                    cast_args.append((val, vt))
            args = cast_args

        arg_str = ", ".join(f"{vt} {val}" for val, vt in args)

        # Variadic functions need the full type signature in the call
        variadic_funcs = getattr(self, '_variadic_funcs', set())
        is_variadic = name in variadic_funcs or node.name in variadic_funcs

        if is_variadic and sig:
            # Build full signature: ret_type (param_types, ...)
            param_sig = ", ".join(sig[1])
            if param_sig:
                call_sig = f"{ret_type} ({param_sig}, ...)"
            else:
                call_sig = f"{ret_type} (...)"
            if ret_type == "void":
                self.emit(f"  call {call_sig} @{name}({arg_str})")
                return "0", "void"
            else:
                result = self.tmp()
                self.emit(f"  {result} = call {call_sig} @{name}({arg_str})")
                return result, ret_type
        elif ret_type == "void":
            self.emit(f"  call void @{name}({arg_str})")
            return "0", "void"
        else:
            result = self.tmp()
            self.emit(f"  {result} = call {ret_type} @{name}({arg_str})")
            return result, ret_type

    # ── Type casting helpers ─────────────────────────────────────────────

    INT_TYPES   = {"i1", "i8", "i16", "i32", "i64"}
    FLOAT_TYPES = {"float", "double"}

    def promote_binop(self, left, lt, right, rt):
        """Promote operands to a common type for binary ops."""
        # int + float → float
        if lt in self.INT_TYPES and rt in self.FLOAT_TYPES:
            p = self.tmp()
            self.emit(f"  {p} = sitofp {lt} {left} to {rt}")
            return p, rt, right, rt
        if lt in self.FLOAT_TYPES and rt in self.INT_TYPES:
            p = self.tmp()
            self.emit(f"  {p} = sitofp {rt} {right} to {lt}")
            return left, lt, p, lt

        # float + double → double
        if lt == "float" and rt == "double":
            p = self.tmp()
            self.emit(f"  {p} = fpext float {left} to double")
            return p, "double", right, "double"
        if lt == "double" and rt == "float":
            p = self.tmp()
            self.emit(f"  {p} = fpext float {right} to double")
            return left, "double", p, "double"

        # int width mismatch → widen the narrower
        if lt in self.INT_TYPES and rt in self.INT_TYPES:
            lw = 1 if lt == "i1" else int(lt[1:])
            rw = 1 if rt == "i1" else int(rt[1:])
            if lw < rw:
                p = self.tmp()
                ext = "zext" if lt == "i1" else "sext"
                self.emit(f"  {p} = {ext} {lt} {left} to {rt}")
                return p, rt, right, rt
            if rw < lw:
                p = self.tmp()
                ext = "zext" if rt == "i1" else "sext"
                self.emit(f"  {p} = {ext} {rt} {right} to {lt}")
                return left, lt, p, lt

        return left, lt, right, rt

    def cast_if_needed(self, val, from_type, to_type):
        """Emit a cast instruction if from_type != to_type. Returns (reg, to_type)."""
        if from_type == to_type:
            return val, to_type

        result = self.tmp()

        if from_type in self.INT_TYPES and to_type in self.FLOAT_TYPES:
            self.emit(f"  {result} = sitofp {from_type} {val} to {to_type}")
        elif from_type in self.FLOAT_TYPES and to_type in self.INT_TYPES:
            self.emit(f"  {result} = fptosi {from_type} {val} to {to_type}")
        elif from_type == "float" and to_type == "double":
            self.emit(f"  {result} = fpext float {val} to double")
        elif from_type == "double" and to_type == "float":
            self.emit(f"  {result} = fptrunc double {val} to float")
        elif from_type in self.INT_TYPES and to_type in self.INT_TYPES:
            fw = 1 if from_type == "i1" else int(from_type[1:])
            tw = 1 if to_type == "i1" else int(to_type[1:])
            if tw > fw:
                ext = "zext" if from_type == "i1" else "sext"
                self.emit(f"  {result} = {ext} {from_type} {val} to {to_type}")
            else:
                self.emit(f"  {result} = trunc {from_type} {val} to {to_type}")
        elif from_type in self.INT_TYPES and (to_type.endswith("*") or to_type == "ptr"):
            # Integer to pointer — use inttoptr (not bitcast)
            self.emit(f"  {result} = inttoptr {from_type} {val} to {to_type}")
        elif (from_type.endswith("*") or from_type == "ptr") and to_type in self.INT_TYPES:
            # Pointer to integer — use ptrtoint (not bitcast)
            self.emit(f"  {result} = ptrtoint {from_type} {val} to {to_type}")
        elif from_type.startswith("[") and to_type.endswith("*"):
            # Array value to pointer — store to temp alloca, then GEP first element
            tmp_alloca = self.tmp()
            self.emit(f"  {tmp_alloca} = alloca {from_type}")
            self.emit(f"  store {from_type} {val}, {from_type}* {tmp_alloca}")
            self.emit(f"  {result} = getelementptr {from_type}, {from_type}* {tmp_alloca}, i32 0, i32 0")
            return result, to_type
        elif to_type.startswith("%") and from_type in self.INT_TYPES and val == "0":
            # Integer 0 to struct type — zeroinitializer
            return "zeroinitializer", to_type
        elif to_type.startswith("%") and from_type in self.INT_TYPES:
            # Integer to struct — can't cast, return zero
            return "zeroinitializer", to_type
        else:
            self.emit(f"  {result} = bitcast {from_type} {val} to {to_type}")

        return result, to_type


# UL builtin → C library function name
BUILTIN_MAP = {
    "absval":  "abs",
    "memtake": "malloc",
    "memgive": "free",
    "memset":  "memset",
    "memcopy": "memcpy",
    "memmove": "memmove",
}

# External builtin signatures: name → (ret_type, [param_types])
BUILTIN_SIGS = {
    "abs":     ("i32",  ["i32"]),
    "malloc":  ("i8*",  ["i64"]),
    "free":    ("void", ["i8*"]),
    "memset":  ("i8*",  ["i8*", "i32", "i64"]),
    "memcpy":  ("i8*",  ["i8*", "i8*", "i64"]),
    "memmove": ("i8*",  ["i8*", "i8*", "i64"]),
    "exit":    ("void", ["i32"]),
}

# External builtin LLVM declarations
BUILTIN_DECLS = {
    "abs":     "declare i32 @abs(i32)",
    "malloc":  "declare i8* @malloc(i64)",
    "free":    "declare void @free(i8*)",
    "memset":  "declare i8* @memset(i8*, i32, i64)",
    "memcpy":  "declare i8* @memcpy(i8*, i8*, i64)",
    "memmove": "declare i8* @memmove(i8*, i8*, i64)",
    "exit":    "declare void @exit(i32)",
}


def generate(program, target_triple=None, profile=False):
    return LLVMGen(target_triple=target_triple, profile=profile).gen_program(program)
