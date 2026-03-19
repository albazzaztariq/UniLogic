# ulc/codegen_c.py — UniLogic → C code generator
# Walks the AST produced by parser.py and emits valid C source.
# Output targets C99. Includes stdio.h and stdint.h automatically.

from ast_nodes import *
from semcheck import UL_TO_C

INDENT = "    "   # 4 spaces per level


class CGen:
    def __init__(self, profile=False):
        self.lines      = []
        self.depth      = 0
        self.var_types  = {}  # name -> TypeName, populated as we emit VarDecls
        self.type_decls = {}  # type name -> TypeDecl
        self._generators = {}  # name -> TypeName (yield type)
        self._gen_callbacks = []  # lines for generated callback functions
        self._cb_counter = 0
        self._result_funcs = {}  # name -> TypeName (ok payload type) for result-returning functions
        self._result_types_emitted = set()  # track which _Result_T types we've emitted
        self._current_decl = None  # current FunctionDecl being generated
        self._tmp_counter = 0
        self._needs_stdlib = False
        self._profile = profile
        self._array_sizes = {}  # var name -> size expression (for bounds checking)
        self._safety_checked = False
        self._current_object = None  # ObjectDecl when generating method body
        self._object_fields = set()  # field names for current object method
        self._func_decls = {}  # name -> FunctionDecl for default arg injection
        self._needs_stdarg = False
        self._needs_pthread = False
        self._thread_wrappers = set()  # function names that need wrapper structs
        self._thread_wrapper_emitted = set()
        self._mutex_vars = set()  # mutex variable names
        self._dynamic_arrays = set()  # variable names of dynamic (empty-init) arrays
        self._dyn_arr_types = set()   # element type names that need dynamic array helpers
        self._needs_setjmp = False    # true if try/catch is used
        self._try_counter = 0         # unique counter for try block temps
        self._list_types = set()      # element type names for list<T> structs
        self._list_vars = set()       # variable names that are lists
        self._needs_time = False      # true if time/clock/sleep used
        self._needs_prompt = False    # true if prompt() used
        self._needs_in_helper = False # true if 'in' operator used on arrays
        self._nested_name_map = {}   # local name -> mangled name for nested functions
        self._refcount_mode = False
        self._rc_scope_stack = []    # stack of lists: each entry is string locals for that scope level

    # ── Helpers ───────────────────────────────────────────────────────────────

    def emit(self, text=""):
        self.lines.append(INDENT * self.depth + text)

    def indent(self):   self.depth += 1
    def dedent(self):   self.depth -= 1

    def result(self):
        return "\n".join(self.lines) + "\n"

    def _const_value(self, node):
        """Emit a compile-time constant value (supports expressions)."""
        t = type(node)
        if t == IntLiteral:
            return str(node.value)
        if t == FloatLiteral:
            return str(node.value)
        if t == StringLiteral:
            return f'"{node.value}"'
        if t == BoolLiteral:
            return "1" if node.value else "0"
        if t == BinaryOp:
            # String concatenation: use C adjacent literal concatenation
            if node.op == "+" and self._is_const_string(node):
                return self._concat_const_strings(node)
            left = self._const_value(node.left)
            right = self._const_value(node.right)
            op = self.ul_op_to_c(node.op)
            return f"({left} {op} {right})"
        if t == UnaryOp:
            operand = self._const_value(node.operand)
            op = self.ul_op_to_c(node.op)
            return f"({op}{operand})"
        if t == Identifier:
            return node.name
        return "0"

    def _is_const_string(self, node):
        """Check if a node is a compile-time string expression."""
        if isinstance(node, StringLiteral):
            return True
        if isinstance(node, BinaryOp) and node.op == "+":
            return self._is_const_string(node.left) and self._is_const_string(node.right)
        return False

    def _concat_const_strings(self, node):
        """Flatten a string concat tree into C adjacent string literals."""
        parts = []
        def _collect(n):
            if isinstance(n, StringLiteral):
                parts.append(f'"{n.value}"')
            elif isinstance(n, BinaryOp) and n.op == "+":
                _collect(n.left)
                _collect(n.right)
        _collect(node)
        return " ".join(parts)

    def c_type(self, type_node):
        if type_node.is_list:
            tag = type_node.name
            return f"__ul_list_{tag}"
        if type_node.name == "auto":
            return "__auto_type"
        base = UL_TO_C.get(type_node.name, type_node.name)
        if type_node.pointer:
            return base + "*"
        return base

    def c_param_type(self, type_node):
        """C type for function parameters — arrays decay to pointers."""
        if type_node.is_list:
            tag = type_node.name
            return f"__ul_list_{tag}"
        if type_node.is_array:
            base = UL_TO_C.get(type_node.name, type_node.name)
            return base + "*"
        return self.c_type(type_node)

    def all_fields(self, type_name):
        """Return all fields for a type, including inherited fields (parent first)."""
        decl = self.type_decls.get(type_name)
        if not decl:
            return []
        if decl.parent:
            return self.all_fields(decl.parent) + decl.fields
        return list(decl.fields)

    def _result_type_name(self, payload_type):
        """Return the C typedef name for a result type, e.g. _Result_int."""
        ctype = self.c_type(payload_type)
        safe = ctype.replace("*", "_ptr").replace(" ", "_")
        return f"_Result_{safe}"

    def _emit_result_type(self, payload_type):
        """Emit a tagged-union result struct typedef if not already emitted."""
        name = self._result_type_name(payload_type)
        if name in self._result_types_emitted:
            return
        self._result_types_emitted.add(name)
        ctype = self.c_type(payload_type)
        self.emit(f"typedef struct {{")
        self.indent()
        self.emit(f"int _ok;")
        self.emit(f"{ctype} _value;")
        self.emit(f"char* _error;")
        self.dedent()
        self.emit(f"}} {name};")
        self.emit("")

    def _field_is_inherited(self, type_name, field_name):
        """Check if a field is inherited (belongs to a parent, not declared directly)."""
        decl = self.type_decls.get(type_name)
        if not decl:
            return False
        for f in decl.fields:
            if f.name == field_name:
                return False
        if decl.parent:
            for f in self.all_fields(decl.parent):
                if f.name == field_name:
                    return True
        return False

    def _base_chain(self, type_name, field_name):
        """Return the chain of _base prefixes needed to reach an inherited field."""
        decl = self.type_decls.get(type_name)
        if not decl:
            return ""
        # Check own fields first
        for f in decl.fields:
            if f.name == field_name:
                return ""
        # Must be inherited — go through _base
        if decl.parent:
            deeper = self._base_chain(decl.parent, field_name)
            return "_base." + deeper
        return ""

    def _resolve_field_type(self, node):
        """Resolve the UL TypeName for a FieldAccess expression."""
        if isinstance(node.target, Identifier):
            vtype = self.var_types.get(node.target.name)
            if vtype and vtype.name in self.type_decls:
                for f in self.all_fields(vtype.name):
                    if f.name == node.field:
                        return f.type_
        elif isinstance(node.target, FieldAccess):
            parent = self._resolve_field_type(node.target)
            if parent and parent.name in self.type_decls:
                for f in self.all_fields(parent.name):
                    if f.name == node.field:
                        return f.type_
        return None

    def fmt_specifier(self, node):
        t = type(node)
        if t == StringLiteral:                          return "%s"
        if t == FloatLiteral:                           return "%f"
        if t == BoolLiteral:                            return "%d"
        if t == Cast:
            if node.target_type.name in ("float", "double"): return "%f"
            if node.target_type.name == "string":            return "%s"
            return "%d"
        if t == Identifier:
            vtype = self.var_types.get(node.name)
            if vtype and vtype.name in ("float", "double"): return "%f"
            if vtype and vtype.name == "string":            return "%s"
            return "%d"
        if t == Call:
            fn_decl = self._func_decls.get(node.name)
            if fn_decl and fn_decl.return_type:
                if fn_decl.return_type.name in ("float", "double"): return "%f"
                if fn_decl.return_type.name == "string": return "%s"
            return "%d"
        if t == BinaryOp:
            if self._is_string_expr(node):  return "%s"
            # if either side is float, use %f
            if self.fmt_specifier(node.left) == "%f":  return "%f"
            if self.fmt_specifier(node.right) == "%f": return "%f"
            return "%d"
        if t == FieldAccess:
            ftype = self._resolve_field_type(node)
            if ftype:
                if ftype.name in ("float", "double"): return "%f"
                if ftype.name == "string":             return "%s"
            return "%d"
        if t == Index:
            # Array element: check array's element type
            if isinstance(node.target, Identifier):
                vtype = self.var_types.get(node.target.name)
                if vtype:
                    if hasattr(vtype, 'name') and vtype.name == 'string': return "%s"
                    if hasattr(vtype, 'element_type'):
                        ename = vtype.element_type.name if hasattr(vtype.element_type, 'name') else str(vtype.element_type)
                        if ename == 'string': return "%s"
                        if ename in ('float', 'double'): return "%f"
            return "%d"
        if t == SliceExpr:
            # String slice → %s, array slice → depends
            if isinstance(node.target, Identifier):
                vtype = self.var_types.get(node.target.name)
                if vtype and vtype.name == "string":
                    return "%s"
            return "%s"
        if t == MethodCall:
            # Look up method return type from object definition
            target_type = self.var_types.get(node.target.name) if isinstance(node.target, Identifier) else None
            if target_type and target_type.name in self.type_decls:
                obj_decl = self.type_decls[target_type.name]
                if isinstance(obj_decl, ObjectDecl):
                    for m in obj_decl.methods:
                        if m.name == node.method and m.return_type:
                            if m.return_type.name in ("float", "double"): return "%f"
                            if m.return_type.name == "string":            return "%s"
            return "%d"
        return "%d"

    # ── Program ───────────────────────────────────────────────────────────────

    def gen_program(self, program):
        # Read DR directives for codegen behavior
        dr_directives = [d for d in program.decls if isinstance(d, DrDirective)]
        self._dr = {d.key: d.value for d in dr_directives}
        self._safety_checked = self._dr.get("safety") == "checked"
        self._arena_mode = self._dr.get("memory") == "arena"
        self._gc_mode = self._dr.get("memory") == "gc"
        self._refcount_mode = self._dr.get("memory") == "refcount"
        if dr_directives:
            for d in dr_directives:
                self.emit(f"// @dr {d.key} = {d.value}")
            self.emit("")

        # Emit const declarations as #define and register types
        for decl in program.decls:
            if isinstance(decl, ConstDecl):
                val = self._const_value(decl.value)
                self.emit(f"#define {decl.name} {val}")
                self.var_types[decl.name] = decl.type_

        self.emit("#include <stdio.h>")
        self.emit("#include <stdint.h>")
        self.emit("#include <string.h>")

        # Emit #include for each foreign import library
        included = {"stdio", "stdint", "string"}  # already included above
        _string_helpers_needed = set()

        # Map UL stdlib names to their _impl.c files
        _UL_STDLIB_IMPLS = {
            "uljson": "json_impl.c",
            "ulmap":  "map_impl.c",
            "ulhttp": "http_impl.c",
            "ulfile": "file_impl.c",
        }

        # Find the Stdlib directory relative to Main.py
        import os as _os
        _stdlib_dir = _os.path.join(
            _os.path.dirname(_os.path.dirname(_os.path.abspath(__file__))),
            "Stdlib")

        for decl in program.decls:
            if isinstance(decl, ForeignImport):
                if decl.lib in _UL_STDLIB_IMPLS:
                    impl_file = _UL_STDLIB_IMPLS[decl.lib]
                    impl_path = _os.path.join(_stdlib_dir, impl_file)
                    if decl.lib not in included:
                        if _os.path.exists(impl_path):
                            self.emit(f'#include "{impl_path}"')
                        else:
                            self.emit(f"// WARNING: {impl_file} not found at {impl_path}")
                        included.add(decl.lib)
                elif decl.lib not in included:
                    self.emit(f"#include <{decl.lib}.h>")
                    included.add(decl.lib)
                if decl.name in ("str_char_at", "str_substr", "str_index_of"):
                    _string_helpers_needed.add(decl.name)
        # Ensure stdlib.h is included (needed for malloc in arena init, default mode, etc.)
        if "stdlib" not in included:
            self.emit("#include <stdlib.h>")
            included.add("stdlib")

        # Memory mode: arena, gc, or default (malloc/free)
        # Must be emitted BEFORE string helpers so __ul_malloc is defined
        if self._arena_mode:
            self.emit("")
            self.emit("// @dr memory = arena — bump allocator, bulk free on exit")
            self.emit("typedef struct { char* base; size_t offset; size_t capacity; } __ul_arena_t;")
            self.emit("static __ul_arena_t __ul_arena;")
            self.emit("void __ul_arena_init(size_t capacity) {")
            self.emit("    __ul_arena.base = (char*)malloc(capacity); __ul_arena.offset = 0; __ul_arena.capacity = capacity;")
            self.emit("}")
            self.emit("void* __ul_arena_alloc(size_t size) {")
            self.emit("    size = (size + 7) & ~7;")
            self.emit("    if (__ul_arena.offset + size > __ul_arena.capacity) { fprintf(stderr, \"arena out of memory\\n\"); exit(1); }")
            self.emit("    void* ptr = __ul_arena.base + __ul_arena.offset; __ul_arena.offset += size; return ptr;")
            self.emit("}")
            self.emit("void __ul_arena_reset() { __ul_arena.offset = 0; }")
            self.emit("void __ul_arena_free() { free(__ul_arena.base); __ul_arena.base = NULL; }")
            self.emit("#define __ul_malloc(size) __ul_arena_alloc(size)")
            self.emit("#define __ul_free(ptr)    /* arena — no individual frees */")
            self.emit("")
        elif self._gc_mode:
            self.emit("")
            self.emit("// @dr memory = gc — Boehm GC (falls back to malloc if gc.h unavailable)")
            self.emit("#if __has_include(<gc.h>)")
            self.emit("#include <gc.h>")
            self.emit("#define __ul_malloc(size) GC_malloc(size)")
            self.emit("#define __ul_free(ptr)    /* GC handles it */")
            self.emit("#define __UL_GC_AVAILABLE 1")
            self.emit("#else")
            self.emit("// WARNING: Boehm GC not found — using malloc, GC behavior not available")
            self.emit("#define __ul_malloc(size) malloc(size)")
            self.emit("#define __ul_free(ptr)    free(ptr)")
            self.emit("#define __UL_GC_AVAILABLE 0")
            self.emit("#endif")
            self.emit("")
        elif self._refcount_mode:
            self.emit("")
            self.emit("// @dr memory = refcount -- reference counting")
            self.emit("typedef struct __ul_rc_header {")
            self.emit("    int refcount;")
            self.emit("    void (*destructor)(void*);")
            self.emit("} __ul_rc_header;")
            self.emit("")
            self.emit("static void* __ul_rc_alloc(size_t size) {")
            self.emit("    __ul_rc_header* hdr = (__ul_rc_header*)malloc(sizeof(__ul_rc_header) + size);")
            self.emit("    hdr->refcount = 1;")
            self.emit("    hdr->destructor = NULL;")
            self.emit("    return (void*)(hdr + 1);")
            self.emit("}")
            self.emit("static void __ul_retain(void* ptr) {")
            self.emit("    if (!ptr) return;")
            self.emit("    __ul_rc_header* hdr = ((__ul_rc_header*)ptr) - 1;")
            self.emit("    hdr->refcount++;")
            self.emit("}")
            self.emit("static void __ul_release(void* ptr) {")
            self.emit("    if (!ptr) return;")
            self.emit("    __ul_rc_header* hdr = ((__ul_rc_header*)ptr) - 1;")
            self.emit("    if (--hdr->refcount <= 0) {")
            self.emit("        if (hdr->destructor) hdr->destructor(ptr);")
            self.emit("        free(hdr);")
            self.emit("    }")
            self.emit("}")
            self.emit("#define __ul_malloc(size) __ul_rc_alloc(size)")
            self.emit("#define __ul_free(ptr)    __ul_release(ptr)")
            self.emit("")
        else:
            self.emit("#define __ul_malloc(size) malloc(size)")
            self.emit("#define __ul_free(ptr)    free(ptr)")

        # Emit inline string helper functions (after __ul_malloc is defined)
        if "str_char_at" in _string_helpers_needed:
            self.emit("static char* str_char_at(const char* s, int i) {")
            self.emit("    char* r = (char*)__ul_malloc(2); if (i >= 0 && i < (int)strlen(s)) { r[0] = s[i]; r[1] = 0; } else { r[0] = 0; } return r; }")
        if "str_substr" in _string_helpers_needed:
            self.emit("static char* str_substr(const char* s, int start, int len) {")
            self.emit("    int slen = (int)strlen(s); if (start < 0) start = 0; if (start >= slen) { static char e[1] = {0}; return e; }")
            self.emit("    if (len > slen - start) len = slen - start; char* r = (char*)__ul_malloc(len + 1); memcpy(r, s + start, len); r[len] = 0; return r; }")
        if "str_index_of" in _string_helpers_needed:
            self.emit("static int str_index_of(const char* s, const char* sub) { const char* p = strstr(s, sub); return p ? (int)(p - s) : -1; }")
        # String concat helper — always emit since string.h is always included
        self.emit("static char* __ul_strcat(const char* a, const char* b) {")
        self.emit("    int la = (int)strlen(a), lb = (int)strlen(b);")
        self.emit("    char* r = (char*)__ul_malloc(la + lb + 1); memcpy(r, a, la); memcpy(r + la, b, lb); r[la+lb] = 0; return r; }")
        self._needs_strcat = False
        self._needs_sort = False
        # Sort comparators for qsort — emitted here so they're available
        self.emit("static int __ul_cmp_int(const void* a, const void* b) { return (*(int*)a - *(int*)b); }")
        self.emit("static int __ul_cmp_float(const void* a, const void* b) { float fa=*(float*)a, fb=*(float*)b; return (fa>fb)-(fa<fb); }")
        self.emit("static int __ul_cmp_double(const void* a, const void* b) { double da=*(double*)a, db=*(double*)b; return (da>db)-(da<db); }")
        self.emit("static int __ul_cmp_string(const void* a, const void* b) { return strcmp(*(char**)a, *(char**)b); }")
        if self._profile:
            self.emit('#include "__ul_profile.h"')
        if self._safety_checked:
            self.emit("#include <stdlib.h>")
            self.emit("")
            self.emit("#define __ul_checked_access(arr, idx, size, name, line) \\")
            self.emit('    ((unsigned)(idx) < (unsigned)(size) ? (arr)[(idx)] : \\')
            self.emit('    (fprintf(stderr, "bounds check error: %s[%d] out of range (size %d) at line %d\\n", \\')
            self.emit('        (name), (int)(idx), (int)(size), (line)), exit(1), (arr)[0]))')
        # Record where to insert stdlib.h if needed later
        self._stdlib_insert_idx = len(self.lines)
        self.emit("")

        # Register generators, result functions, and all function decls
        for decl in program.decls:
            if isinstance(decl, FunctionDecl):
                self._func_decls[decl.name] = decl
                if decl.is_generator:
                    self._generators[decl.name] = decl.return_type
                if decl.returns_result:
                    payload_type = decl.return_type if decl.return_type else TypeName("int")
                    self._result_funcs[decl.name] = payload_type
                # Register nested functions with mangled names
                for nf in getattr(decl, 'nested_functions', []):
                    mangled = f"{decl.name}__{nf.name}"
                    self._func_decls[mangled] = nf
                    self._func_decls[nf.name] = nf

        # Result type struct definitions
        for name, payload_type in self._result_funcs.items():
            self._emit_result_type(payload_type)

        # Tuple return struct typedefs
        self._tuple_types_emitted = set()
        for decl in program.decls:
            if isinstance(decl, FunctionDecl) and isinstance(decl.return_type, TupleReturn):
                tname = "_Tuple_" + "_".join(t.name for t in decl.return_type.types)
                if tname not in self._tuple_types_emitted:
                    self._tuple_types_emitted.add(tname)
                    fields = "; ".join(f"{self.c_type(t)} _f{i}" for i, t in enumerate(decl.return_type.types))
                    self.emit(f"typedef struct {{ {fields}; }} {tname};")

        # Struct type definitions
        for decl in program.decls:
            if isinstance(decl, TypeDecl):
                self.type_decls[decl.name] = decl
                self.gen_type_decl(decl)

        # Object type definitions (vtable pattern)
        for decl in program.decls:
            if isinstance(decl, ObjectDecl):
                self.type_decls[decl.name] = decl
                self.gen_object_decl(decl)

        # Enum declarations
        self._enums = {}  # name -> dict of member -> value
        for decl in program.decls:
            if isinstance(decl, EnumDecl):
                self._enums[decl.name] = {name: val for name, val in decl.members}
                members = ", ".join(f"{decl.name}_{name} = {self._const_value(val)}" for name, val in decl.members)
                self.emit(f"enum {decl.name} {{ {members} }};")
                self.emit("")

        # Scan for list types and emit list infrastructure
        self._scan_list_types(program)
        if self._list_types:
            self._needs_stdlib = True
            for elem in sorted(self._list_types):
                self._emit_list_type(elem)

        # Module-level variable declarations (global C variables)
        for decl in program.decls:
            if isinstance(decl, VarDecl):
                self.var_types[decl.name] = decl.type_
                ctype = self.c_type(decl.type_)
                if decl.type_.is_array and isinstance(decl.init, ArrayLiteral):
                    elems = ", ".join(self.gen_expr(e) for e in decl.init.elements)
                    self.emit(f"{ctype} {decl.name}[] = {{{elems}}};")
                elif decl.type_.is_list:
                    tag = decl.type_.name
                    self._list_vars.add(decl.name)
                    self.emit(f"__ul_list_{tag} {decl.name} = {{NULL, 0, 0}};")
                elif decl.init is not None:
                    self.emit(f"{ctype} {decl.name} = {self.gen_expr(decl.init)};")
                else:
                    self.emit(f"{ctype} {decl.name};")

        # Forward declarations (skip non-functions)
        for decl in program.decls:
            if isinstance(decl, (ForeignImport, TypeDecl, ObjectDecl, ConstDecl, DrDirective, NormDirective, AsmBlock, EnumDecl, VarDecl)):
                continue
            self.emit(self.fn_signature(decl) + ";")
            # Forward declarations for nested functions (hoisted with mangled names)
            for nf in getattr(decl, 'nested_functions', []):
                saved = nf.name
                nf.name = f"{decl.name}__{nf.name}"
                self.emit(self.fn_signature(nf) + ";")
                nf.name = saved

        # Object method forward declarations
        for decl in program.decls:
            if isinstance(decl, ObjectDecl):
                for method in decl.methods:
                    params = ", ".join(f"{self.c_type(p.type_)} {p.name}" for p in method.params)
                    self_param = f"{decl.name}* self"
                    all_params = f"{self_param}, {params}" if params else self_param
                    ret = self.c_type(method.return_type) if method.return_type else "void"
                    self.emit(f"{ret} {decl.name}_{method.name}({all_params});")
        # Forward declare vtable instances
        for decl in program.decls:
            if isinstance(decl, ObjectDecl):
                self.emit(f"static {decl.name}_VTable _{decl.name}_vtable;")
        self.emit("")

        # Full definitions — generate into buffer so callbacks can be inserted
        func_lines = []
        for decl in program.decls:
            if isinstance(decl, (ForeignImport, TypeDecl, ObjectDecl, ConstDecl, DrDirective, NormDirective, AsmBlock, EnumDecl, VarDecl)):
                continue
            # Emit hoisted nested functions before the enclosing function
            for nf in getattr(decl, 'nested_functions', []):
                saved_name = nf.name
                nf.name = f"{decl.name}__{saved_name}"
                saved = self.lines
                self.lines = []
                self.gen_function(nf)
                func_lines.extend(self.lines)
                self.lines = saved
                nf.name = saved_name
            saved = self.lines
            self.lines = []
            self.gen_function(decl)
            func_lines.extend(self.lines)
            self.lines = saved

        # Emit thread wrapper functions if needed
        if self._needs_pthread:
            self._needs_stdlib = True
            self.emit("// Thread infrastructure")
            self.emit("#define __UL_MAX_THREADS 64")
            self.emit("static pthread_t __ul_threads[__UL_MAX_THREADS];")
            self.emit("static int __ul_thread_count = 0;")
            for mvar in self._mutex_vars:
                self.emit(f"static pthread_mutex_t {mvar} = PTHREAD_MUTEX_INITIALIZER;")
            for fname in self._thread_wrappers:
                fn_decl = self._func_decls.get(fname)
                if fn_decl:
                    struct_name = f"__ul_{fname}_args"
                    fields = "; ".join(f"{self.c_type(p.type_)} {p.name}" for p in fn_decl.params)
                    self.emit(f"typedef struct {{ {fields}; }} {struct_name};")
                    call_args = ", ".join(f"a->{p.name}" for p in fn_decl.params)
                    self.emit(f"void* __ul_{fname}_wrapper(void* args) {{")
                    self.emit(f"    {struct_name}* a = ({struct_name}*)args;")
                    self.emit(f"    {fname}({call_args});")
                    self.emit(f"    __ul_free(args);")
                    self.emit(f"    return NULL;")
                    self.emit(f"}}")
            self.emit("")

        # Emit generator callbacks, then function definitions
        self.lines.extend(self._gen_callbacks)
        self.lines.extend(func_lines)

        # Object method implementations
        for decl in program.decls:
            if isinstance(decl, ObjectDecl):
                # Collect all fields including inherited
                all_fields = set()
                if decl.parent and decl.parent in self.type_decls:
                    parent = self.type_decls[decl.parent]
                    if hasattr(parent, 'fields'):
                        for f in parent.fields:
                            all_fields.add(f.name)
                for f in decl.fields:
                    all_fields.add(f.name)

                for method in decl.methods:
                    params = ", ".join(f"{self.c_param_type(p.type_)} {p.name}" for p in method.params)
                    self_param = f"{decl.name}* self"
                    all_params = f"{self_param}, {params}" if params else self_param
                    ret = self.c_type(method.return_type) if method.return_type else "void"
                    self.emit(f"{ret} {decl.name}_{method.name}({all_params})")
                    self.emit("{")
                    self.indent()
                    self._current_object = decl
                    self._object_fields = all_fields
                    for stmt in method.body:
                        self.gen_stmt(stmt)
                    self._current_object = None
                    self._object_fields = set()
                    self.dedent()
                    self.emit("}")
                    self.emit("")

        # Emit static vtable instances for each object type
        for decl in program.decls:
            if isinstance(decl, ObjectDecl):
                all_methods = []
                if decl.parent and decl.parent in self.type_decls:
                    parent = self.type_decls[decl.parent]
                    if isinstance(parent, ObjectDecl):
                        all_methods.extend(parent.methods)
                method_names = {m.name for m in all_methods}
                for m in decl.methods:
                    if m.name in method_names:
                        all_methods = [am if am.name != m.name else m for am in all_methods]
                    else:
                        all_methods.append(m)
                ptrs = ", ".join(f"({decl.name}_{m.name})" for m in all_methods
                                if any(dm.name == m.name for dm in decl.methods))
                # Build full pointer list, using parent impl for inherited non-overridden methods
                ptr_list = []
                for m in all_methods:
                    if any(dm.name == m.name for dm in decl.methods):
                        ptr_list.append(f"(void*){decl.name}_{m.name}")
                    elif decl.parent:
                        ptr_list.append(f"(void*){decl.parent}_{m.name}")
                self.emit(f"static {decl.name}_VTable _{decl.name}_vtable = {{{', '.join(ptr_list)}}};")

        self.emit("")

        # Insert stdlib.h if exit() was used
        if self._needs_stdlib:
            self.lines.insert(self._stdlib_insert_idx, "#include <stdlib.h>")
        if self._needs_stdarg:
            self.lines.insert(self._stdlib_insert_idx, "#include <stdarg.h>")
        if self._needs_pthread:
            self.lines.insert(self._stdlib_insert_idx, "#include <pthread.h>")
        if self._needs_setjmp:
            # Insert setjmp include and exception infrastructure
            idx = self._stdlib_insert_idx
            infra = [
                "#include <setjmp.h>",
                "typedef struct { const char* type; char message[256]; } __ul_exception_t;",
                "static __ul_exception_t __ul_current_exception;",
                "static jmp_buf* __ul_jmp_stack[64];",
                "static int __ul_jmp_top = -1;",
                "static void __ul_throw(const char* type, const char* msg) {",
                "    __ul_current_exception.type = type;",
                "    strncpy(__ul_current_exception.message, msg, 255);",
                "    __ul_current_exception.message[255] = 0;",
                "    if (__ul_jmp_top >= 0) { longjmp(*__ul_jmp_stack[__ul_jmp_top], 1); }",
                '    else { fprintf(stderr, "unhandled exception: %s: %s\\n", type, msg); exit(1); }',
                "}",
            ]
            for i, line in enumerate(infra):
                self.lines.insert(idx + i, line)

        if self._needs_time:
            idx = self._stdlib_insert_idx
            time_infra = [
                "#include <time.h>",
                "#ifdef _WIN32",
                "#include <windows.h>",
                "static void __ul_sleep_ms(int ms) { Sleep(ms); }",
                "#else",
                "#include <unistd.h>",
                "static void __ul_sleep_ms(int ms) { usleep(ms * 1000); }",
                "#endif",
                "static long long __ul_clock_ns(void) {",
                "    struct timespec ts;",
                "    timespec_get(&ts, TIME_UTC);",
                "    return (long long)ts.tv_sec * 1000000000LL + ts.tv_nsec;",
                "}",
            ]
            for i, line in enumerate(time_infra):
                self.lines.insert(idx + i, line)

        if self._needs_prompt:
            idx = self._stdlib_insert_idx
            prompt_infra = [
                "static char __ul_prompt_buf[4096];",
                "static char* __ul_prompt(const char* msg) {",
                '    printf("%s", msg);',
                "    fflush(stdout);",
                "    if (fgets(__ul_prompt_buf, sizeof(__ul_prompt_buf), stdin)) {",
                "        size_t len = strlen(__ul_prompt_buf);",
                "        if (len > 0 && __ul_prompt_buf[len-1] == '\\n') __ul_prompt_buf[len-1] = 0;",
                "        return __ul_prompt_buf;",
                "    }",
                '    return "";',
                "}",
            ]
            for i, line in enumerate(prompt_infra):
                self.lines.insert(idx + i, line)

        if self._needs_in_helper:
            idx = self._stdlib_insert_idx
            in_infra = [
                "static int __ul_in_int(int val, int* arr, int n) { for (int i = 0; i < n; i++) if (arr[i] == val) return 1; return 0; }",
                "static int __ul_in_str(const char* val, const char** arr, int n) { for (int i = 0; i < n; i++) if (strcmp(arr[i], val) == 0) return 1; return 0; }",
            ]
            for i, line in enumerate(in_infra):
                self.lines.insert(idx + i, line)

        return self.result()

    def gen_type_decl(self, decl):
        self.emit(f"typedef struct {{")
        self.indent()
        if decl.parent:
            self.emit(f"{decl.parent} _base;")
        for f in decl.fields:
            self.emit(f"{self.c_type(f.type_)} {f.name};")
        self.dedent()
        self.emit(f"}} {decl.name};")
        self.emit("")

    def gen_object_decl(self, decl):
        """Emit C struct with vtable for an object declaration."""
        # Forward declare the struct
        self.emit(f"typedef struct {decl.name} {decl.name};")

        # Vtable struct — collect all methods including inherited
        all_methods = []
        if decl.parent and decl.parent in self.type_decls:
            parent = self.type_decls[decl.parent]
            if isinstance(parent, ObjectDecl):
                all_methods.extend(parent.methods)
        # Override or add child methods
        method_names = {m.name for m in all_methods}
        for m in decl.methods:
            if m.name in method_names:
                all_methods = [am if am.name != m.name else m for am in all_methods]
            else:
                all_methods.append(m)

        self.emit(f"typedef struct {{")
        self.indent()
        for m in all_methods:
            params = ", ".join(f"{self.c_type(p.type_)} {p.name}" for p in m.params)
            self_param = f"{decl.name}*"
            all_params = f"{self_param}, {params}" if params else self_param
            ret = self.c_type(m.return_type) if m.return_type else "void"
            self.emit(f"{ret} (*{m.name})({all_params});")
        self.dedent()
        self.emit(f"}} {decl.name}_VTable;")
        self.emit("")

        # Data struct
        self.emit(f"struct {decl.name} {{")
        self.indent()
        if decl.parent:
            self.emit(f"{decl.parent} _base;")
        self.emit(f"{decl.name}_VTable* _vtable;")
        self.emit(f"const char* __type;")
        for f in decl.fields:
            self.emit(f"{self.c_type(f.type_)} {f.name};")
        self.dedent()
        self.emit(f"}};")
        self.emit("")

    # ── Function ──────────────────────────────────────────────────────────────

    def fn_signature(self, decl):
        if decl.is_generator:
            yield_ctype = self.c_type(decl.return_type)
            params = ", ".join(
                f"{self.c_param_type(p.type_)} {p.name}" for p in decl.params
            )
            cb_param = f"void (*_yield_cb)({yield_ctype})"
            if params:
                params += ", " + cb_param
            else:
                params = cb_param
            return f"void {decl.name}({params})"
        if decl.returns_result:
            payload_type = decl.return_type if decl.return_type else TypeName("int")
            ret = self._result_type_name(payload_type)
        elif isinstance(decl.return_type, TupleReturn):
            ret = "_Tuple_" + "_".join(t.name for t in decl.return_type.types)
        else:
            ret = self.c_type(decl.return_type) if decl.return_type else "void"
        # C standard requires int main
        if decl.name == "main" and ret == "void":
            ret = "int"
        params = ", ".join(
            f"{self.c_param_type(p.type_)} {p.name}" for p in decl.params
        ) or "void"
        if getattr(decl, 'variadic', False):
            if params == "void":
                params = "..."
            else:
                params += ", ..."
            self._needs_stdarg = True
        return f"{ret} {decl.name}({params})"

    def _is_main_void(self, decl):
        """Check if this is main() declared with returns none."""
        return (decl.name == "main" and not decl.is_generator
                and (decl.return_type is None or decl.return_type.name == "none"))

    def gen_function(self, decl):
        self._current_decl = decl
        # Set up nested function name mapping for call redirection
        saved_nested_map = dict(self._nested_name_map)
        for nf in getattr(decl, 'nested_functions', []):
            self._nested_name_map[nf.name] = f"{decl.name}__{nf.name}"
        # Register parameter types so fmt_specifier works on params too
        for p in decl.params:
            self.var_types[p.name] = p.type_
        # Refcount: push a function-level scope for tracking string/array locals
        if self._refcount_mode:
            self._rc_scope_stack.append([])
        self.emit(self.fn_signature(decl))
        self.emit("{")
        self.indent()
        # Arena/GC init at start of main()
        if decl.name == "main":
            if self._arena_mode:
                self.emit("__ul_arena_init(64 * 1024 * 1024);  /* 64 MB arena */")
            elif self._gc_mode:
                self.emit("#if __UL_GC_AVAILABLE")
                self.emit("    GC_INIT();")
                self.emit("#endif")
        if self._profile:
            self.emit(f'__ul_enter("{decl.name}", __FILE__, {decl.line});')
        for stmt in decl.body:
            self.gen_stmt(stmt)
        # Refcount: release all string/array locals at end of function scope
        if self._refcount_mode and self._rc_scope_stack:
            locals_list = self._rc_scope_stack[-1]
            if locals_list:
                self.emit("// refcount: release locals")
                for varname in locals_list:
                    self.emit(f"__ul_release({varname});")
        if self._profile:
            self.emit(f'__ul_exit("{decl.name}", __FILE__, {decl.line});')
        if self._is_main_void(decl):
            if self._profile:
                self.emit("__ul_profile_dump();")
            if self._arena_mode:
                self.emit("__ul_arena_free();")
            self.emit("return 0;")
        self.dedent()
        self.emit("}")
        self.emit("")
        # Refcount: pop function scope
        if self._refcount_mode and self._rc_scope_stack:
            self._rc_scope_stack.pop()
        self._nested_name_map = saved_nested_map
        self._current_decl = None

    # ── Statements ────────────────────────────────────────────────────────────

    def gen_stmt(self, node):
        t = type(node)

        if t == VarDecl:
            self.var_types[node.name] = node.type_
            ctype = ("const " if node.fixed else "") + self.c_type(node.type_)
            if node.type_.is_array and isinstance(node.init, ArrayLiteral) and len(node.init.elements) == 0:
                # Dynamic array (empty init) → use list infrastructure
                tag = node.type_.name
                self._list_vars.add(node.name)
                self.emit(f"__ul_list_{tag} {node.name} = {{NULL, 0, 0}};")
            elif node.type_.is_array and isinstance(node.init, ArrayLiteral):
                elems = ", ".join(self.gen_expr(e) for e in node.init.elements)
                self.emit(f"{ctype} {node.name}[] = {{{elems}}};")
                self._array_sizes[node.name] = str(len(node.init.elements))
            elif node.type_.is_array and isinstance(node.init, SliceExpr):
                # array T x = arr[i:j] → emit slice, then alias pointer
                slice_name = self.gen_expr(node.init)
                self.emit(f"{ctype}* {node.name} = {slice_name};")
            elif node.type_.is_array and isinstance(node.init, ArrayComprehension):
                comp = node.init
                if isinstance(comp.iterable, Call) and comp.iterable.name == "range":
                    args = comp.iterable.args
                    if len(args) == 1:
                        start, end = "0", self.gen_expr(args[0])
                    elif len(args) >= 2:
                        start, end = self.gen_expr(args[0]), self.gen_expr(args[1])
                    else:
                        start, end = "0", "0"
                    size = f"({end} - {start})"
                    idx = f"_ci{self._tmp_counter}"
                    self._tmp_counter += 1
                    self.emit(f"{ctype} {node.name}[{size}];")
                    self.emit(f"for (int {idx} = 0, {comp.var} = {start}; {comp.var} < {end}; {comp.var}++, {idx}++) {{")
                    self.indent()
                    self.emit(f"{node.name}[{idx}] = {self.gen_expr(comp.expr)};")
                    self.dedent()
                    self.emit("}")
                else:
                    self.emit(f"/* array comprehension with non-range iterable not supported in C */")
                    self.emit(f"{ctype} {node.name}[0];")
            elif node.type_.is_list:
                # list T → __ul_list_T name = {NULL, 0, 0};
                tag = node.type_.name
                self._list_vars.add(node.name)
                if node.init is not None and isinstance(node.init, ArrayLiteral):
                    self.emit(f"__ul_list_{tag} {node.name} = {{NULL, 0, 0}};")
                    for elem in node.init.elements:
                        self.emit(f"__ul_list_{tag}_append(&{node.name}, {self.gen_expr(elem)});")
                elif node.init is not None and isinstance(node.init, Call) and node.init.name in ("map_fn", "filter_fn"):
                    # map_fn/filter_fn returns a temp list variable name
                    tmp = self.gen_expr(node.init)
                    self.emit(f"__ul_list_{tag} {node.name} = {tmp};")
                else:
                    self.emit(f"__ul_list_{tag} {node.name} = {{NULL, 0, 0}};")
            elif node.init is not None:
                self.emit(f"{ctype} {node.name} = {self.gen_expr(node.init)};")
            else:
                self.emit(f"{ctype} {node.name};")
                # Initialize vtable for object types
                if node.type_.name in self.type_decls:
                    td = self.type_decls[node.type_.name]
                    if isinstance(td, ObjectDecl):
                        self.emit(f"{node.name}._vtable = &_{node.type_.name}_vtable;")
                        self.emit(f'{node.name}.__type = "{node.type_.name}";')
            # Refcount: track string locals for release at scope end
            if self._refcount_mode and self._rc_scope_stack and node.type_.name == "string":
                self._rc_scope_stack[-1].append(node.name)

        elif t == Assign:
            if self._refcount_mode and isinstance(node.target, Identifier):
                vtype = self.var_types.get(node.target.name)
                if vtype and vtype.name == "string":
                    # Release old value, assign new, retain new
                    new_val = self.gen_expr(node.value)
                    self.emit(f"__ul_release({node.target.name});")
                    self.emit(f"{node.target.name} {node.op} {new_val};")
                    self.emit(f"__ul_retain({node.target.name});")
                else:
                    self.emit(f"{self.gen_expr(node.target)} {node.op} {self.gen_expr(node.value)};")
            else:
                self.emit(f"{self.gen_expr(node.target)} {node.op} {self.gen_expr(node.value)};")

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
            if self._refcount_mode:
                self._rc_scope_stack.append([])
            for s in node.body: self.gen_stmt(s)
            if self._refcount_mode:
                scope = self._rc_scope_stack.pop()
                for varname in scope:
                    self.emit(f"__ul_release({varname});")
            self.dedent()
            self.emit("}")

        elif t == DoWhile:
            self.emit("do {")
            self.indent()
            if self._refcount_mode:
                self._rc_scope_stack.append([])
            for s in node.body: self.gen_stmt(s)
            if self._refcount_mode:
                scope = self._rc_scope_stack.pop()
                for varname in scope:
                    self.emit(f"__ul_release({varname});")
            self.dedent()
            self.emit(f"}} while ({self.gen_expr(node.condition)});")

        elif t == For:
            if isinstance(node.iterable, Call) and node.iterable.name in self._generators:
                # Generator for-each → callback pattern
                yield_type = self._generators[node.iterable.name]
                cb_name = self._gen_callback(node.var, yield_type, node.body)
                args = ", ".join(self.gen_expr(a) for a in node.iterable.args)
                if args:
                    self.emit(f"{node.iterable.name}({args}, {cb_name});")
                else:
                    self.emit(f"{node.iterable.name}({cb_name});")
            elif isinstance(node.iterable, Call) and node.iterable.name == 'range':
                # range(stop) or range(start, stop) → C for loop
                if getattr(node, 'parallel', False):
                    self.emit("#pragma omp parallel for")
                rargs = node.iterable.args
                if len(rargs) == 1:
                    stop = self.gen_expr(rargs[0])
                    self.emit(f"for (int {node.var} = 0; {node.var} < {stop}; {node.var}++) {{")
                else:
                    start = self.gen_expr(rargs[0])
                    stop = self.gen_expr(rargs[1])
                    self.emit(f"for (int {node.var} = {start}; {node.var} < {stop}; {node.var}++) {{")
                self.indent()
                for s in node.body: self.gen_stmt(s)
                self.dedent()
                self.emit("}")
            elif isinstance(node.iterable, Identifier) and self.var_types.get(node.iterable.name) and self.var_types[node.iterable.name].is_list:
                # Iterate over list: for (int _i = 0; _i < list.len; _i++)
                list_var = node.iterable.name
                vtype = self.var_types[list_var]
                elem_ctype = UL_TO_C.get(vtype.name, vtype.name)
                self.var_types[node.var] = TypeName(vtype.name)
                self.emit(f"for (int _i = 0; _i < {list_var}.len; _i++) {{")
                self.indent()
                self.emit(f"{elem_ctype} {node.var} = {list_var}.data[_i];")
                for s in node.body: self.gen_stmt(s)
                self.dedent()
                self.emit("}")
            else:
                iterable = self.gen_expr(node.iterable)
                # Register loop var type from iterable's element type
                if isinstance(node.iterable, Identifier):
                    arr_type = self.var_types.get(node.iterable.name)
                    if arr_type:
                        self.var_types[node.var] = TypeName(arr_type.name)
                elem_ctype = self.c_type(self.var_types.get(node.var, TypeName("int")))
                self.emit(f"for (int _i = 0; _i < (int)(sizeof({iterable})/sizeof({iterable}[0])); _i++) {{")
                self.indent()
                self.emit(f"{elem_ctype} {node.var} = {iterable}[_i];")
                for s in node.body: self.gen_stmt(s)
                self.dedent()
                self.emit("}")

        elif t == CFor:
            # C-style for: emit init; for (; cond; update) { body }
            # Emit init as a separate statement, then use for loop
            self.emit("{")
            self.indent()
            self.gen_stmt(node.init)
            cond = self.gen_expr(node.condition)
            # Generate update expression
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
            self.emit(f"for (; {cond}; {update_str}) {{")
            self.indent()
            for s in node.body: self.gen_stmt(s)
            self.dedent()
            self.emit("}")
            self.dedent()
            self.emit("}")

        elif t == Return:
            if self._profile and self._current_decl:
                self.emit(f'__ul_exit("{self._current_decl.name}", __FILE__, {self._current_decl.line});')
                if self._current_decl.name == "main":
                    self.emit("__ul_profile_dump();")
            # Arena cleanup before explicit return in main()
            if self._arena_mode and self._current_decl and self._current_decl.name == "main":
                self.emit("__ul_arena_free();")
            # Refcount: release all scope locals before return
            if self._refcount_mode and self._rc_scope_stack:
                for scope in reversed(self._rc_scope_stack):
                    for varname in scope:
                        self.emit(f"__ul_release({varname});")
            if node.value is not None and isinstance(node.value, OkResult):
                payload_type = self._current_decl.return_type if self._current_decl.return_type else TypeName("int")
                rtype = self._result_type_name(payload_type)
                val = self.gen_expr(node.value.value)
                self.emit(f"return ({rtype}){{1, {val}, \"\"}};")
            elif node.value is not None and isinstance(node.value, ErrorResult):
                payload_type = self._current_decl.return_type if self._current_decl.return_type else TypeName("int")
                rtype = self._result_type_name(payload_type)
                val = self.gen_expr(node.value.value)
                self.emit(f"return ({rtype}){{0, 0, {val}}};")
            elif node.value is not None:
                self.emit(f"return {self.gen_expr(node.value)};")
            else:
                self.emit("return;")

        elif t == Print:
            val = self.gen_expr(node.value)
            fmt = self.fmt_specifier(node.value)
            self.emit(f'printf("{fmt}\\n", {val});')

        elif t == ExprStmt:
            self.emit(f"{self.gen_expr(node.expr)};")

        elif t == Escape:
            self.emit("break;")

        elif t == Continue:
            self.emit("continue;")

        elif t == Match:
            subj = self.gen_expr(node.subject)
            first = True
            for case in node.cases:
                if case.value is None:
                    # default
                    if first:
                        self.emit("{")
                    else:
                        self.emit("} else {")
                    self.indent()
                    for s in case.body: self.gen_stmt(s)
                    self.dedent()
                    first = False
                else:
                    cv = self.gen_expr(case.value)
                    if isinstance(case.value, StringLiteral):
                        cond = f"strcmp({subj}, {cv}) == 0"
                    else:
                        cond = f"{subj} == {cv}"
                    kw = "if" if first else "} else if"
                    self.emit(f"{kw} ({cond}) {{")
                    self.indent()
                    for s in case.body: self.gen_stmt(s)
                    self.dedent()
                    first = False
            if not first:
                self.emit("}")

        elif t == Yield:
            self.emit(f"_yield_cb({self.gen_expr(node.value)});")

        elif t == PostIncrement:
            self.emit(f"{self.gen_expr(node.operand)}++;")

        elif t == PostDecrement:
            self.emit(f"{self.gen_expr(node.operand)}--;")

        elif t == TupleDestructure:
            # (type name, type name) = call()
            tmp = f"_td_{self._tmp_counter}"
            self._tmp_counter += 1
            val = self.gen_expr(node.value)
            # Generate tuple struct name from types
            type_names = [self.c_type(t_) for t_, _ in node.targets]
            struct_name = "_Tuple_" + "_".join(t_.name for t_, _ in node.targets)
            self.emit(f"{struct_name} {tmp} = {val};")
            for i, (t_, name) in enumerate(node.targets):
                ctype = self.c_type(t_)
                self.var_types[name] = t_
                self.emit(f"{ctype} {name} = {tmp}._f{i};")

        elif t == KillswitchStmt:
            if self._safety_checked:
                cond = self.gen_expr(node.condition)
                self.emit(f"if (!({cond})) {{")
                self.indent()
                self.emit(f'fprintf(stderr, "killswitch failed at line %d\\n", {node.line});')
                self.emit(f"exit(1);")
                self._needs_stdlib = True
                self.dedent()
                self.emit("}")
            # When safety != checked, emit nothing

        elif t == PortalDecl:
            self.emit(f"{node.name}:;")  # C label (;  = empty statement after label)

        elif t == GotoStmt:
            self.emit(f"goto {node.target};")

        elif t == TryStmt:
            self._needs_setjmp = True
            self._needs_stdlib = True
            n = self._try_counter
            self._try_counter += 1
            self.emit("{")
            self.indent()
            self.emit(f"jmp_buf __ul_try_buf_{n};")
            self.emit(f"__ul_jmp_stack[++__ul_jmp_top] = &__ul_try_buf_{n};")
            self.emit(f"if (setjmp(__ul_try_buf_{n}) == 0) {{")
            self.indent()
            for s in node.body:
                self.gen_stmt(s)
            self.emit("__ul_jmp_top--;")
            self.dedent()
            self.emit("} else {")
            self.indent()
            self.emit("__ul_jmp_top--;")
            if node.handlers:
                first = True
                for handler in node.handlers:
                    if handler.exception_type:
                        kw = "if" if first else "} else if"
                        self.emit(f'{kw} (strcmp(__ul_current_exception.type, "{handler.exception_type}") == 0) {{')
                    else:
                        # catch-all
                        if first:
                            self.emit("{")
                        else:
                            self.emit("} else {")
                    self.indent()
                    if handler.binding_name:
                        self.emit(f"const char* {handler.binding_name} = __ul_current_exception.message;")
                    for s in handler.body:
                        self.gen_stmt(s)
                    self.dedent()
                    first = False
                # If no catch-all handler, re-throw unmatched exceptions
                has_catchall = any(h.exception_type is None for h in node.handlers)
                if not has_catchall:
                    self.emit("} else {")
                    self.indent()
                    self.emit("__ul_throw(__ul_current_exception.type, __ul_current_exception.message);")
                    self.dedent()
                self.emit("}")
            else:
                # No handlers — just caught to run finally
                pass
            self.dedent()
            self.emit("}")
            # Finally body
            for s in node.finally_body:
                self.gen_stmt(s)
            self.dedent()
            self.emit("}")

        elif t == ThrowStmt:
            self._needs_setjmp = True
            self._needs_stdlib = True
            msg = self.gen_expr(node.message)
            self.emit(f'__ul_throw("{node.exception_type}", {msg});')

        elif t == WithStmt:
            expr = self.gen_expr(node.expr)
            self.emit(f"{{ /* with */")
            self.indent()
            # Try to resolve type from the expression
            if isinstance(node.expr, Call):
                fn = self._func_decls.get(node.expr.name)
                if fn and fn.return_type:
                    ctype = self.c_type(fn.return_type)
                    self.emit(f"{ctype} {node.binding} = {expr};")
                else:
                    self.emit(f"__auto_type {node.binding} = {expr};")
            else:
                self.emit(f"__auto_type {node.binding} = {expr};")
            for s in node.body: self.gen_stmt(s)
            self.emit(f"/* end with — cleanup {node.binding} */")
            self.dedent()
            self.emit("}")

        elif t == NormDirective:
            self.emit(f"/* @norm {node.level} */")

        else:
            self.emit(f"/* unhandled stmt: {type(node).__name__} */")

    # ── Expressions ───────────────────────────────────────────────────────────

    def gen_expr(self, node):
        t = type(node)

        if t == IntLiteral:
            v = node.value
            # Large constants exceeding int32 range need suffix
            if v > 2147483647 or v < -2147483648:
                if v > 9223372036854775807:
                    return f"{v}ULL"
                return f"{v}LL"
            return str(v)

        if t == FloatLiteral:
            return repr(node.value)

        if t == StringLiteral:
            escaped = node.value.replace("\\", "\\\\").replace('"', '\\"').replace("\n", "\\n").replace("\t", "\\t")
            return f'"{escaped}"'

        if t == BoolLiteral:
            return "1" if node.value else "0"

        if t == EmptyLiteral:
            return "NULL"

        if t == Identifier:
            if self._current_object and node.name in self._object_fields:
                return f"self->{node.name}"
            return node.name

        if t == BinaryOp:
            if node.op == "|>":
                # Pipe operator: left |> right — call right(left)
                left = self.gen_expr(node.left)
                right = self.gen_expr(node.right)
                return f"{right}({left})"
            if node.op == "+" and self._is_string_expr(node.left):
                self._needs_strcat = True
                return f"__ul_strcat({self.gen_expr(node.left)}, {self.gen_expr(node.right)})"
            if node.op in ("==", "!=", "<", ">", "<=", ">=") and (self._is_string_expr(node.left) or self._is_string_expr(node.right)):
                l = self.gen_expr(node.left)
                r = self.gen_expr(node.right)
                # Don't use strcmp when comparing against NULL (empty literal)
                if isinstance(node.left, EmptyLiteral) or isinstance(node.right, EmptyLiteral):
                    op = self.ul_op_to_c(node.op)
                    return f"({l} {op} {r})"
                cmp_op = {"==": "== 0", "!=": "!= 0", "<": "< 0", ">": "> 0", "<=": "<= 0", ">=": ">= 0"}[node.op]
                return f"(strcmp({l}, {r}) {cmp_op})"
            if node.op == "in":
                left = self.gen_expr(node.left)
                right = self.gen_expr(node.right)
                # String containment: strstr
                if self._is_string_expr(node.right):
                    return f"(strstr({right}, {left}) != NULL)"
                # List containment: use list_T_contains
                if isinstance(node.right, Identifier) and node.right.name in self._list_vars:
                    vtype = self.var_types.get(node.right.name)
                    if vtype:
                        tag = vtype.name  # element type name = list tag
                        return f"__ul_list_{tag}_contains(&{right}, {left})"
                # Array containment: linear search helper
                self._needs_in_helper = True
                return f"__ul_in_int({left}, {right}, sizeof({right}) / sizeof({right}[0]))"
            op = self.ul_op_to_c(node.op)
            return f"({self.gen_expr(node.left)} {op} {self.gen_expr(node.right)})"

        if t == UnaryOp:
            if node.op == "not":     return f"(!{self.gen_expr(node.operand)})"
            if node.op == "-":       return f"(-{self.gen_expr(node.operand)})"
            if node.op == "address": return f"(&{self.gen_expr(node.operand)})"
            if node.op == "deref":   return f"(*{self.gen_expr(node.operand)})"
            if node.op == "bit_not": return f"(~{self.gen_expr(node.operand)})"

        if t == Cast:
            ctype = self.c_type(node.target_type)
            # change(x)->type is UL cast syntax — the inner expr is the thing being cast,
            # not a real function call. Unwrap one level if it's a change() call.
            inner = node.expr
            if isinstance(inner, Call) and inner.name == "change" and len(inner.args) == 1:
                inner = inner.args[0]
            # Numeric-to-string cast needs snprintf
            if node.target_type.name == "string":
                buf = f"_cast_buf_{self._tmp_counter}"
                self._tmp_counter += 1
                inner_expr = self.gen_expr(inner)
                inner_fmt = self.fmt_specifier(inner)
                self.emit(f"static char {buf}[64];")
                self.emit(f'snprintf({buf}, sizeof({buf}), "{inner_fmt}", {inner_expr});')
                return buf
            return f"(({ctype}){self.gen_expr(inner)})"

        if t == Call:
            # Fill in default parameter values for missing args
            call_args = list(node.args)
            fn_decl = self._func_decls.get(node.name)
            if fn_decl and len(call_args) < len(fn_decl.params):
                for i in range(len(call_args), len(fn_decl.params)):
                    if fn_decl.params[i].default is not None:
                        call_args.append(fn_decl.params[i].default)
            args = ", ".join(self.gen_expr(a) for a in call_args)
            # char_code(s, i) → (int)s[i]
            if node.name == "char_code":
                s_expr = self.gen_expr(call_args[0])
                i_expr = self.gen_expr(call_args[1])
                return f"((int)(unsigned char){s_expr}[{i_expr}])"
            # sort(arr, n) → qsort with type-specific comparator
            if node.name == "sort":
                self._needs_sort = True
                arr_expr = self.gen_expr(call_args[0])
                n_expr = self.gen_expr(call_args[1])
                # Determine element type from var_types
                arr_name = call_args[0].name if isinstance(call_args[0], Identifier) else None
                cmp_func = "__ul_cmp_int"
                elem_size = "sizeof(int)"
                if arr_name:
                    vtype = self.var_types.get(arr_name)
                    if vtype and hasattr(vtype, 'name'):
                        if vtype.name in ("float",):
                            cmp_func = "__ul_cmp_float"
                            elem_size = "sizeof(float)"
                        elif vtype.name in ("double",):
                            cmp_func = "__ul_cmp_double"
                            elem_size = "sizeof(double)"
                        elif vtype.name == "string":
                            cmp_func = "__ul_cmp_string"
                            elem_size = "sizeof(char*)"
                return f"qsort({arr_expr}, {n_expr}, {elem_size}, {cmp_func})"
            # System functions (Task 11)
            if node.name == "time":
                self._needs_time = True
                return "((int)time(NULL))"
            if node.name == "clock":
                self._needs_time = True
                return "__ul_clock_ns()"
            if node.name == "sleep":
                self._needs_time = True
                ms_expr = self.gen_expr(call_args[0])
                return f"__ul_sleep_ms({ms_expr})"
            if node.name == "random":
                self._needs_stdlib = True
                return "((double)rand() / (double)RAND_MAX)"
            if node.name == "random_int":
                self._needs_stdlib = True
                lo = self.gen_expr(call_args[0])
                hi = self.gen_expr(call_args[1])
                return f"({lo} + rand() % ({hi} - {lo} + 1))"
            if node.name == "random_seed":
                self._needs_stdlib = True
                return f"srand({self.gen_expr(call_args[0])})"
            if node.name == "vault":
                self._needs_stdlib = True
                return f"getenv({self.gen_expr(call_args[0])})"
            if node.name == "abort":
                self._needs_stdlib = True
                return "abort()"
            if node.name == "prompt":
                self._needs_prompt = True
                return f"__ul_prompt({self.gen_expr(call_args[0])})"
            # Python builtin equivalents (Task 15) — C implementations
            if node.name == "enumerate":
                arr = self.gen_expr(call_args[0])
                return f"/* enumerate({arr}) — use for loop with index */"
            if node.name == "zip":
                a = self.gen_expr(call_args[0])
                b = self.gen_expr(call_args[1])
                return f"/* zip({a}, {b}) — use for loop to combine */"
            if node.name == "map_fn":
                fn_name = call_args[0].name if isinstance(call_args[0], Identifier) else self.gen_expr(call_args[0])
                arr = self.gen_expr(call_args[1])
                arr_name = call_args[1].name if isinstance(call_args[1], Identifier) else None
                # Check if source is a list
                is_list_src = arr_name and arr_name in self._list_vars
                tmp = f"_map_{self._tmp_counter}"
                self._tmp_counter += 1
                if is_list_src:
                    vtype = self.var_types.get(arr_name)
                    tag = vtype.name if vtype else "int"
                    self.emit(f"__ul_list_{tag} {tmp} = {{NULL, 0, 0}};")
                    self.emit(f"for (int _i = 0; _i < {arr}.len; _i++) __ul_list_{tag}_append(&{tmp}, {fn_name}({arr}.data[_i]));")
                else:
                    size_expr = self._array_sizes.get(arr_name, f"sizeof({arr})/sizeof({arr}[0])") if arr_name else f"sizeof({arr})/sizeof({arr}[0])"
                    self.emit(f"int {tmp}[{size_expr}];")
                    self.emit(f"for (int _i = 0; _i < {size_expr}; _i++) {tmp}[_i] = {fn_name}({arr}[_i]);")
                return tmp
            if node.name == "filter_fn":
                fn_name = call_args[0].name if isinstance(call_args[0], Identifier) else self.gen_expr(call_args[0])
                arr = self.gen_expr(call_args[1])
                arr_name = call_args[1].name if isinstance(call_args[1], Identifier) else None
                is_list_src = arr_name and arr_name in self._list_vars
                tmp = f"_filt_{self._tmp_counter}"
                self._tmp_counter += 1
                if is_list_src:
                    vtype = self.var_types.get(arr_name)
                    tag = vtype.name if vtype else "int"
                    self.emit(f"__ul_list_{tag} {tmp} = {{NULL, 0, 0}};")
                    self.emit(f"for (int _i = 0; _i < {arr}.len; _i++) if ({fn_name}({arr}.data[_i])) __ul_list_{tag}_append(&{tmp}, {arr}.data[_i]);")
                else:
                    size_expr = self._array_sizes.get(arr_name, f"sizeof({arr})/sizeof({arr}[0])") if arr_name else f"sizeof({arr})/sizeof({arr}[0])"
                    cnt = f"_fcnt_{self._tmp_counter}"
                    self.emit(f"int {tmp}[{size_expr}]; int {cnt} = 0;")
                    self.emit(f"for (int _i = 0; _i < {size_expr}; _i++) if ({fn_name}({arr}[_i])) {tmp}[{cnt}++] = {arr}[_i];")
                return tmp
            # remap nested function calls to mangled names, then UL builtins to C equivalents
            resolved = self._nested_name_map.get(node.name, node.name)
            name = BUILTIN_MAP.get(resolved, resolved)
            if node.name in ("exit", "memtake", "memgive"):
                self._needs_stdlib = True
                if self._profile:
                    self.emit("__ul_profile_dump();")
            if self._profile and node.name == "memtake":
                return f"__ul_alloc({args})"
            if self._profile and node.name == "memgive":
                return f"__ul_free({args})"
            # Threading: spawn, wait, lock, unlock
            if node.name == "spawn" and self._dr.get("concurrency") == "threaded":
                self._needs_pthread = True
                # spawn func(args) → inner Call node is the arg
                if call_args and isinstance(call_args[0], Call):
                    inner = call_args[0]
                    fname = inner.name
                    inner_args = [self.gen_expr(a) for a in inner.args]
                    tid = f"_tid_{self._tmp_counter}"
                    self._tmp_counter += 1
                    if inner_args:
                        # Emit args struct and wrapper
                        struct_name = f"__ul_{fname}_args"
                        fn_decl = self._func_decls.get(fname)
                        if fn_decl:
                            fields = "; ".join(f"{self.c_type(p.type_)} {p.name}" for p in fn_decl.params)
                            self._thread_wrappers.add(fname)
                            if fname not in self._thread_wrapper_emitted:
                                self._thread_wrapper_emitted.add(fname)
                                # These will be emitted in gen_program
                        args_var = f"_args_{self._tmp_counter}"
                        self._tmp_counter += 1
                        self.emit(f"{struct_name}* {args_var} = ({struct_name}*)__ul_malloc(sizeof({struct_name}));")
                        for i, a in enumerate(inner_args):
                            pname = fn_decl.params[i].name if fn_decl else f"_a{i}"
                            self.emit(f"{args_var}->{pname} = {a};")
                        self.emit(f"pthread_t {tid};")
                        self.emit(f"pthread_create(&{tid}, NULL, __ul_{fname}_wrapper, {args_var});")
                    else:
                        self.emit(f"pthread_t {tid};")
                        self.emit(f"pthread_create(&{tid}, NULL, (void*(*)(void*)){fname}, NULL);")
                    self.emit(f"__ul_threads[__ul_thread_count++] = {tid};")
                    return "0"
                return "0"
            if node.name == "wait" and self._dr.get("concurrency") == "threaded":
                self._needs_pthread = True
                self.emit("for (int _i = 0; _i < __ul_thread_count; _i++) pthread_join(__ul_threads[_i], NULL);")
                self.emit("__ul_thread_count = 0;")
                return "0"
            if node.name == "lock" and self._dr.get("concurrency") == "threaded":
                self._needs_pthread = True
                if call_args:
                    mutex_name = self.gen_expr(call_args[0])
                    self._mutex_vars.add(mutex_name)
                    return f"pthread_mutex_lock(&{mutex_name})"
                return "0"
            if node.name == "unlock" and self._dr.get("concurrency") == "threaded":
                self._needs_pthread = True
                if call_args:
                    mutex_name = self.gen_expr(call_args[0])
                    return f"pthread_mutex_unlock(&{mutex_name})"
                return "0"
            # Stubs when not in threaded mode
            if node.name in ("spawn", "wait", "lock", "unlock"):
                return "0"
            return f"{name}({args})"

        if t == Index:
            # List or dynamic-array indexing: .data[i]
            if isinstance(node.target, Identifier):
                vtype = self.var_types.get(node.target.name)
                if vtype and (vtype.is_list or node.target.name in self._dynamic_arrays):
                    target = node.target.name
                    index_expr = self.gen_expr(node.index)
                    if self._safety_checked:
                        return f"__ul_checked_access({target}.data, {index_expr}, {target}.len, \"{target}\", {node.line})"
                    return f"{target}.data[{index_expr}]"
            target_expr = self.gen_expr(node.target)
            index_expr = self.gen_expr(node.index)
            if self._safety_checked and isinstance(node.target, Identifier):
                size = self._array_sizes.get(node.target.name)
                if size:
                    return f"__ul_checked_access({target_expr}, {index_expr}, {size}, \"{node.target.name}\", {node.line})"
            return f"{target_expr}[{index_expr}]"

        if t == SliceExpr:
            target_expr = self.gen_expr(node.target)
            start_expr = self.gen_expr(node.start)
            end_expr = self.gen_expr(node.end)
            # Determine if slicing a string or an array
            is_str = False
            if isinstance(node.target, Identifier):
                vtype = self.var_types.get(node.target.name)
                if vtype and vtype.name == "string":
                    is_str = True
            elif isinstance(node.target, StringLiteral):
                is_str = True
            if is_str:
                buf = f"_slice{self._tmp_counter}"
                self._tmp_counter += 1
                self._needs_stdlib = True
                self.emit(f"int _sl{self._tmp_counter} = ({end_expr}) - ({start_expr});")
                self.emit(f"char* {buf} = (char*)__ul_malloc(_sl{self._tmp_counter} + 1);")
                self.emit(f"memcpy({buf}, {target_expr} + ({start_expr}), _sl{self._tmp_counter});")
                self.emit(f"{buf}[_sl{self._tmp_counter}] = 0;")
                return buf
            else:
                # Array slice — copy into a new stack array
                buf = f"_slice{self._tmp_counter}"
                self._tmp_counter += 1
                vtype = self.var_types.get(node.target.name) if isinstance(node.target, Identifier) else None
                elem_ctype = UL_TO_C.get(vtype.name, vtype.name) if vtype else "int"
                self.emit(f"int _sn{self._tmp_counter} = ({end_expr}) - ({start_expr});")
                self.emit(f"{elem_ctype} {buf}[_sn{self._tmp_counter}];")
                self.emit(f"memcpy({buf}, &{target_expr}[{start_expr}], _sn{self._tmp_counter} * sizeof({elem_ctype}));")
                return buf

        if t == FieldAccess:
            return self._gen_field_access(node)

        if t == PostIncrement:
            return f"{self.gen_expr(node.operand)}++"

        if t == PostDecrement:
            return f"{self.gen_expr(node.operand)}--"

        if t == TupleLiteral:
            elems = ", ".join(self.gen_expr(e) for e in node.elements)
            # If inside a function with TupleReturn, use the struct type
            if self._current_decl and isinstance(self._current_decl.return_type, TupleReturn):
                tname = "_Tuple_" + "_".join(t_.name for t_ in self._current_decl.return_type.types)
                return f"({tname}){{{elems}}}"
            return f"{{{elems}}}"

        if t == StructLiteral:
            fields = ", ".join(f".{fname} = {self.gen_expr(fval)}" for fname, fval in node.fields)
            return f"({node.type_name}){{{fields}}}"

        if t == MethodCall:
            # String method sugar: s.len() → strlen(s), s.method() → str_method(s)
            if isinstance(node.target, Identifier):
                vtype = self.var_types.get(node.target.name)
                if vtype and vtype.name == "string":
                    target = self.gen_expr(node.target)
                    # .len() maps directly to strlen (always available via string.h)
                    if node.method == "len":
                        return f"(int)strlen({target})"
                    args = ", ".join(self.gen_expr(a) for a in node.args)
                    all_args = f"{target}, {args}" if args else target
                    return f"str_{node.method}({all_args})"
                # List method sugar: scores.append(x) → __ul_list_int_append(&scores, x)
                if vtype and vtype.is_list:
                    target = node.target.name
                    tag = vtype.name
                    if node.method in ("append", "push"):
                        val = self.gen_expr(node.args[0])
                        return f"__ul_list_{tag}_append(&{target}, {val})"
                    if node.method == "pop":
                        return f"__ul_list_{tag}_pop(&{target})"
                    if node.method == "drop":
                        if node.args:
                            idx = self.gen_expr(node.args[0])
                            return f"__ul_list_{tag}_drop(&{target}, {idx})"
                        return f"__ul_list_{tag}_pop(&{target})"
                    if node.method == "insert":
                        idx = self.gen_expr(node.args[0])
                        val = self.gen_expr(node.args[1])
                        return f"__ul_list_{tag}_insert(&{target}, {idx}, {val})"
                    if node.method == "remove":
                        val = self.gen_expr(node.args[0])
                        return f"__ul_list_{tag}_remove(&{target}, {val})"
                    if node.method == "clear":
                        return f"__ul_list_{tag}_clear(&{target})"
                    if node.method == "sort":
                        return f"__ul_list_{tag}_sort(&{target})"
                    if node.method == "reverse":
                        return f"__ul_list_{tag}_reverse(&{target})"
                    if node.method == "contains":
                        val = self.gen_expr(node.args[0])
                        return f"__ul_list_{tag}_contains(&{target}, {val})"
                    if node.method == "len":
                        return f"{target}.len"
                # Array method sugar: nums.sort() → array_sort(nums, size)
                if vtype and vtype.is_array:
                    target = node.target.name
                    # Dynamic arrays use list infrastructure
                    if target in self._dynamic_arrays:
                        tag = vtype.name
                        if node.method in ("append", "push"):
                            val = self.gen_expr(node.args[0])
                            return f"__ul_list_{tag}_append(&{target}, {val})"
                        if node.method == "pop":
                            return f"__ul_list_{tag}_pop(&{target})"
                        if node.method == "drop":
                            if node.args:
                                idx = self.gen_expr(node.args[0])
                                return f"__ul_list_{tag}_drop(&{target}, {idx})"
                            return f"__ul_list_{tag}_pop(&{target})"
                        if node.method == "len":
                            return f"{target}.len"
                        if node.method == "sort":
                            return f"__ul_list_{tag}_sort(&{target})"
                        if node.method == "reverse":
                            return f"__ul_list_{tag}_reverse(&{target})"
                        if node.method == "contains":
                            val = self.gen_expr(node.args[0])
                            return f"__ul_list_{tag}_contains(&{target}, {val})"
                    target_expr = self.gen_expr(node.target)
                    size = self._array_sizes.get(node.target.name,
                           f"(int)(sizeof({target_expr})/sizeof({target_expr}[0]))")
                    extra_args = ", ".join(self.gen_expr(a) for a in node.args)
                    all_args = f"{target_expr}, {size}, {extra_args}" if extra_args else f"{target_expr}, {size}"
                    return f"array_{node.method}({all_args})"
            target = self.gen_expr(node.target)
            args = ", ".join(self.gen_expr(a) for a in node.args)
            self_arg = f"&{target}" if not target.startswith("(") else target
            all_args = f"{self_arg}, {args}" if args else self_arg
            return f"{target}._vtable->{node.method}({all_args})"

        if t == OkResult:
            payload_type = self._current_decl.return_type if self._current_decl.return_type else TypeName("int")
            rtype = self._result_type_name(payload_type)
            val = self.gen_expr(node.value)
            return f"({rtype}){{1, {val}, \"\"}}"

        if t == ErrorResult:
            payload_type = self._current_decl.return_type if self._current_decl.return_type else TypeName("int")
            rtype = self._result_type_name(payload_type)
            val = self.gen_expr(node.value)
            return f"({rtype}){{0, 0, {val}}}"

        if t == ResultPropagation:
            inner = node.expr
            call_expr = self.gen_expr(inner)
            call_name = inner.name if isinstance(inner, Call) else None
            payload_type = self._result_funcs.get(call_name, TypeName("int"))
            rtype = self._result_type_name(payload_type)
            tmp = f"_r{self._tmp_counter}"
            self._tmp_counter += 1
            self.emit(f"{rtype} {tmp} = {call_expr};")
            self.emit(f"if (!{tmp}._ok) {{")
            self.indent()
            if self._current_decl and self._current_decl.returns_result:
                # Propagate: return error result to caller
                caller_payload = self._current_decl.return_type if self._current_decl.return_type else TypeName("int")
                caller_rtype = self._result_type_name(caller_payload)
                self.emit(f"return ({caller_rtype}){{0, 0, {tmp}._error}};")
            else:
                # Boundary: print error to stderr and exit
                self.emit(f'fprintf(stderr, "error: %s\\n", {tmp}._error);')
                self.emit(f"exit(1);")
                self._needs_stdlib = True
            self.dedent()
            self.emit("}")
            return f"{tmp}._value"

        if t == ArrayComprehension:
            return self._gen_array_comprehension(node)

        return f"/* unknown expr {t.__name__} */"

    def _resolve_var_type_name(self, node):
        """Resolve the UL type name for an expression's result."""
        if isinstance(node, Identifier):
            vtype = self.var_types.get(node.name)
            return vtype.name if vtype else None
        if isinstance(node, FieldAccess):
            ftype = self._resolve_field_type(node)
            return ftype.name if ftype else None
        return None

    def _gen_field_access(self, node):
        """Generate C field access, inserting _base for inherited fields."""
        # Enum member: Priority.high → Priority_high
        if isinstance(node.target, Identifier) and node.target.name in self._enums:
            return f"{node.target.name}_{node.field}"
        # self.field inside method → self->field (self is a pointer)
        if (isinstance(node.target, Identifier) and node.target.name == "self"
                and self._current_object):
            base_chain = self._base_chain(self._current_object.name, node.field)
            return f"self->{base_chain}{node.field}"
        # List .length → .len
        if isinstance(node.target, Identifier):
            vtype = self.var_types.get(node.target.name)
            if vtype and vtype.is_list and node.field == "length":
                return f"{node.target.name}.len"
        target_type_name = self._resolve_var_type_name(node.target)
        if target_type_name and target_type_name in self.type_decls:
            base_chain = self._base_chain(target_type_name, node.field)
            return f"{self.gen_expr(node.target)}.{base_chain}{node.field}"
        return f"{self.gen_expr(node.target)}.{node.field}"

    def _gen_callback(self, loop_var, yield_type, body_stmts):
        """Generate a file-scope static callback function for a generator for-each."""
        cb_name = f"_gen_body_{self._cb_counter}"
        self._cb_counter += 1
        ctype = self.c_type(yield_type)

        saved_lines = self.lines
        saved_depth = self.depth
        saved_var_types = dict(self.var_types)

        self.lines = []
        self.depth = 0
        self.emit(f"static void {cb_name}({ctype} {loop_var}) {{")
        self.indent()
        self.var_types[loop_var] = yield_type
        for s in body_stmts:
            self.gen_stmt(s)
        self.dedent()
        self.emit("}")
        self.emit("")

        self._gen_callbacks.extend(self.lines)

        self.lines = saved_lines
        self.depth = saved_depth
        self.var_types = saved_var_types
        return cb_name

    def _gen_array_comprehension(self, node):
        """Generate C code for [expr for var in range(start, end)].
        Emits a for loop filling a static array, returns the array name."""
        arr = f"_comp{self._tmp_counter}"
        idx = f"_ci{self._tmp_counter}"
        self._tmp_counter += 1
        # Determine range bounds from the iterable
        if isinstance(node.iterable, Call) and node.iterable.name == "range":
            args = node.iterable.args
            if len(args) == 1:
                start, end = "0", self.gen_expr(args[0])
            elif len(args) >= 2:
                start, end = self.gen_expr(args[0]), self.gen_expr(args[1])
            else:
                start, end = "0", "0"
            size = f"({end} - {start})"
            self.emit(f"int {arr}[{size}];")
            self.emit(f"for (int {idx} = 0, {node.var} = {start}; {node.var} < {end}; {node.var}++, {idx}++) {{")
            self.indent()
            expr_val = self.gen_expr(node.expr)
            self.emit(f"{arr}[{idx}] = {expr_val};")
            self.dedent()
            self.emit("}")
        else:
            # Generic iterable — not supported in C, emit error comment
            self.emit(f"/* array comprehension with non-range iterable not supported in C */")
            self.emit(f"int {arr}[0];")
        return arr

    def _is_string_expr(self, node):
        """Check if an expression is likely a string type."""
        if isinstance(node, StringLiteral):
            return True
        if isinstance(node, Identifier):
            vt = self.var_types.get(node.name)
            if vt and hasattr(vt, 'name') and vt.name == 'string':
                return True
        if isinstance(node, BinaryOp) and node.op == '+':
            return self._is_string_expr(node.left) or self._is_string_expr(node.right)
        if isinstance(node, Call) and node.name in ('str_char_at', 'str_substr', 'reverse_string', 'caesar_cipher'):
            return True
        return False

    # ── List type infrastructure ────────────────────────────────────────────

    def _scan_list_types(self, program):
        """Walk all declarations finding list types to determine which __ul_list_<T> structs to emit."""
        def _scan_stmts(stmts):
            for s in stmts:
                if isinstance(s, VarDecl) and s.type_.is_list:
                    self._list_types.add(s.type_.name)
                # Dynamic arrays (array T x = []) also need list helpers
                if (isinstance(s, VarDecl) and s.type_.is_array
                        and isinstance(s.init, ArrayLiteral)
                        and len(s.init.elements) == 0):
                    self._list_types.add(s.type_.name)
                    self._dynamic_arrays.add(s.name)
                if isinstance(s, If):
                    _scan_stmts(s.then_body)
                    _scan_stmts(s.else_body)
                if isinstance(s, While):
                    _scan_stmts(s.body)
                if isinstance(s, For):
                    _scan_stmts(s.body)
                if isinstance(s, TryStmt):
                    _scan_stmts(s.body)
                    for h in s.handlers:
                        _scan_stmts(h.body)
                    _scan_stmts(s.finally_body)
                if isinstance(s, Match):
                    for c in s.cases:
                        _scan_stmts(c.body)
        for decl in program.decls:
            if isinstance(decl, FunctionDecl):
                _scan_stmts(decl.body)
                for p in decl.params:
                    if p.type_.is_list:
                        self._list_types.add(p.type_.name)
            elif isinstance(decl, ObjectDecl):
                for f in decl.fields:
                    if f.type_.is_list:
                        self._list_types.add(f.type_.name)
                for m in decl.methods:
                    _scan_stmts(m.body)

    def _emit_list_type(self, elem_type_name):
        """Emit C struct typedef and helper functions for __ul_list_<elem>."""
        c_elem = UL_TO_C.get(elem_type_name, elem_type_name)
        tag = elem_type_name
        # Struct definition
        self.emit(f"typedef struct {{ {c_elem}* data; int len; int cap; }} __ul_list_{tag};")
        # append
        self.emit(f"static void __ul_list_{tag}_append(__ul_list_{tag}* l, {c_elem} val) {{")
        self.emit(f"    if (l->len >= l->cap) {{ l->cap = l->cap ? l->cap * 2 : 8; l->data = ({c_elem}*)realloc(l->data, l->cap * sizeof({c_elem})); }}")
        self.emit(f"    l->data[l->len++] = val;")
        self.emit(f"}}")
        # pop (remove last, return it)
        self.emit(f"static {c_elem} __ul_list_{tag}_pop(__ul_list_{tag}* l) {{ return l->data[--l->len]; }}")
        # drop (remove at index, return it)
        self.emit(f"static {c_elem} __ul_list_{tag}_drop(__ul_list_{tag}* l, int i) {{")
        self.emit(f"    {c_elem} val = l->data[i];")
        self.emit(f"    memmove(&l->data[i], &l->data[i+1], (l->len - i - 1) * sizeof({c_elem}));")
        self.emit(f"    l->len--; return val;")
        self.emit(f"}}")
        # insert
        self.emit(f"static void __ul_list_{tag}_insert(__ul_list_{tag}* l, int i, {c_elem} val) {{")
        self.emit(f"    if (l->len >= l->cap) {{ l->cap = l->cap ? l->cap * 2 : 8; l->data = ({c_elem}*)realloc(l->data, l->cap * sizeof({c_elem})); }}")
        self.emit(f"    memmove(&l->data[i+1], &l->data[i], (l->len - i) * sizeof({c_elem}));")
        self.emit(f"    l->data[i] = val; l->len++;")
        self.emit(f"}}")
        # remove (by value)
        if elem_type_name == "string":
            cmp = "strcmp(l->data[i], val) == 0"
        else:
            cmp = "l->data[i] == val"
        self.emit(f"static void __ul_list_{tag}_remove(__ul_list_{tag}* l, {c_elem} val) {{")
        self.emit(f"    for (int i = 0; i < l->len; i++) {{ if ({cmp}) {{")
        self.emit(f"        memmove(&l->data[i], &l->data[i+1], (l->len - i - 1) * sizeof({c_elem})); l->len--; return; }} }}")
        self.emit(f"}}")
        # clear
        self.emit(f"static void __ul_list_{tag}_clear(__ul_list_{tag}* l) {{ l->len = 0; }}")
        # sort
        cmp_func = {"int": "__ul_cmp_int", "float": "__ul_cmp_float", "double": "__ul_cmp_double", "string": "__ul_cmp_string"}.get(elem_type_name, "__ul_cmp_int")
        self.emit(f"static void __ul_list_{tag}_sort(__ul_list_{tag}* l) {{ qsort(l->data, l->len, sizeof({c_elem}), {cmp_func}); }}")
        # reverse
        self.emit(f"static void __ul_list_{tag}_reverse(__ul_list_{tag}* l) {{")
        self.emit(f"    for (int i = 0, j = l->len - 1; i < j; i++, j--) {{ {c_elem} tmp = l->data[i]; l->data[i] = l->data[j]; l->data[j] = tmp; }}")
        self.emit(f"}}")
        # contains
        if elem_type_name == "string":
            contains_cmp = "strcmp(l->data[i], val) == 0"
        else:
            contains_cmp = "l->data[i] == val"
        self.emit(f"static int __ul_list_{tag}_contains(__ul_list_{tag}* l, {c_elem} val) {{")
        self.emit(f"    for (int i = 0; i < l->len; i++) if ({contains_cmp}) return 1; return 0;")
        self.emit(f"}}")
        self.emit("")

    def ul_op_to_c(self, op):
        return {
            "and": "&&",
            "or":  "||",
            "not": "!",
            "bit_and":   "&",
            "bit_or":    "|",
            "bit_xor":   "^",
            "bit_not":   "~",
            "bit_left":  "<<",
            "bit_right": ">>",
        }.get(op, op)   # most operators are identical in C


# UL builtin → C function name
BUILTIN_MAP = {
    "absval":  "abs",
    "size":    "sizeof",
    "memmove": "memmove",
    "memcopy": "memcpy",
    "memset":  "memset",
    "memtake": "__ul_malloc",
    "memgive": "__ul_free",
}


def generate(program, profile=False):
    return CGen(profile=profile).gen_program(program)
