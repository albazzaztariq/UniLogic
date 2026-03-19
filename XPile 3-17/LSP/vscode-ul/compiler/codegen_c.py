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

    # ── Helpers ───────────────────────────────────────────────────────────────

    def emit(self, text=""):
        self.lines.append(INDENT * self.depth + text)

    def indent(self):   self.depth += 1
    def dedent(self):   self.depth -= 1

    def result(self):
        return "\n".join(self.lines) + "\n"

    def _const_value(self, node):
        """Emit a compile-time constant value."""
        t = type(node)
        if t == IntLiteral:
            return str(node.value)
        if t == FloatLiteral:
            return str(node.value)
        if t == StringLiteral:
            return f'"{node.value}"'
        if t == BoolLiteral:
            return "1" if node.value else "0"
        return "0"

    def c_type(self, type_node):
        if type_node.name == "auto":
            return "__auto_type"
        base = UL_TO_C.get(type_node.name, type_node.name)
        if type_node.pointer:
            return base + "*"
        return base

    def c_param_type(self, type_node):
        """C type for function parameters — arrays decay to pointers."""
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
        for decl in program.decls:
            if isinstance(decl, ForeignImport) and decl.lib not in included:
                self.emit(f"#include <{decl.lib}.h>")
                included.add(decl.lib)
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

        # Forward declarations (skip non-functions)
        for decl in program.decls:
            if isinstance(decl, (ForeignImport, TypeDecl, ObjectDecl, ConstDecl, DrDirective, NormDirective, AsmBlock)):
                continue
            self.emit(self.fn_signature(decl) + ";")

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
            if isinstance(decl, (ForeignImport, TypeDecl, ObjectDecl, ConstDecl, DrDirective, NormDirective, AsmBlock)):
                continue
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
                    self.emit(f"    free(args);")
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
        # Register parameter types so fmt_specifier works on params too
        for p in decl.params:
            self.var_types[p.name] = p.type_
        self.emit(self.fn_signature(decl))
        self.emit("{")
        self.indent()
        if self._profile:
            self.emit(f'__ul_enter("{decl.name}", __FILE__, {decl.line});')
        for stmt in decl.body:
            self.gen_stmt(stmt)
        if self._profile:
            self.emit(f'__ul_exit("{decl.name}", __FILE__, {decl.line});')
        if self._is_main_void(decl):
            if self._profile:
                self.emit("__ul_profile_dump();")
            self.emit("return 0;")
        self.dedent()
        self.emit("}")
        self.emit("")
        self._current_decl = None

    # ── Statements ────────────────────────────────────────────────────────────

    def gen_stmt(self, node):
        t = type(node)

        if t == VarDecl:
            self.var_types[node.name] = node.type_
            ctype = ("const " if node.fixed else "") + self.c_type(node.type_)
            if node.type_.is_array and isinstance(node.init, ArrayLiteral):
                elems = ", ".join(self.gen_expr(e) for e in node.init.elements)
                self.emit(f"{ctype} {node.name}[] = {{{elems}}};")
                self._array_sizes[node.name] = str(len(node.init.elements))
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

        elif t == Assign:
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
            for s in node.body: self.gen_stmt(s)
            self.dedent()
            self.emit("}")

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

        elif t == Return:
            if self._profile and self._current_decl:
                self.emit(f'__ul_exit("{self._current_decl.name}", __FILE__, {self._current_decl.line});')
                if self._current_decl.name == "main":
                    self.emit("__ul_profile_dump();")
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

        elif t == NormDirective:
            self.emit(f"/* @norm {node.level} */")

        else:
            self.emit(f"/* unhandled stmt: {type(node).__name__} */")

    # ── Expressions ───────────────────────────────────────────────────────────

    def gen_expr(self, node):
        t = type(node)

        if t == IntLiteral:
            return str(node.value)

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
            op = self.ul_op_to_c(node.op)
            return f"({self.gen_expr(node.left)} {op} {self.gen_expr(node.right)})"

        if t == UnaryOp:
            if node.op == "not":     return f"(!{self.gen_expr(node.operand)})"
            if node.op == "-":       return f"(-{self.gen_expr(node.operand)})"
            if node.op == "address": return f"(&{self.gen_expr(node.operand)})"
            if node.op == "deref":   return f"(*{self.gen_expr(node.operand)})"

        if t == Cast:
            ctype = self.c_type(node.target_type)
            # change(x)->type is UL cast syntax — the inner expr is the thing being cast,
            # not a real function call. Unwrap one level if it's a change() call.
            inner = node.expr
            if isinstance(inner, Call) and inner.name == "change" and len(inner.args) == 1:
                inner = inner.args[0]
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
            # remap UL builtins to C equivalents
            name = BUILTIN_MAP.get(node.name, node.name)
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
                        self.emit(f"{struct_name}* {args_var} = ({struct_name}*)malloc(sizeof({struct_name}));")
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
            target_expr = self.gen_expr(node.target)
            index_expr = self.gen_expr(node.index)
            if self._safety_checked and isinstance(node.target, Identifier):
                size = self._array_sizes.get(node.target.name)
                if size:
                    return f"__ul_checked_access({target_expr}, {index_expr}, {size}, \"{node.target.name}\", {node.line})"
            return f"{target_expr}[{index_expr}]"

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
            # String method sugar: s.len() → str_len(s)
            if isinstance(node.target, Identifier):
                vtype = self.var_types.get(node.target.name)
                if vtype and vtype.name == "string":
                    target = self.gen_expr(node.target)
                    args = ", ".join(self.gen_expr(a) for a in node.args)
                    all_args = f"{target}, {args}" if args else target
                    return f"str_{node.method}({all_args})"
                # Array method sugar: nums.sort() → array_sort(nums, size)
                if vtype and vtype.is_array:
                    target = self.gen_expr(node.target)
                    size = self._array_sizes.get(node.target.name,
                           f"(int)(sizeof({target})/sizeof({target}[0]))")
                    extra_args = ", ".join(self.gen_expr(a) for a in node.args)
                    all_args = f"{target}, {size}, {extra_args}" if extra_args else f"{target}, {size}"
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

    def ul_op_to_c(self, op):
        return {
            "and": "&&",
            "or":  "||",
            "not": "!",
        }.get(op, op)   # most operators are identical in C


# UL builtin → C function name
BUILTIN_MAP = {
    "absval":  "abs",
    "size":    "sizeof",
    "memmove": "memmove",
    "memcopy": "memcpy",
    "memset":  "memset",
    "memtake": "malloc",
    "memgive": "free",
}


def generate(program, profile=False):
    return CGen(profile=profile).gen_program(program)
