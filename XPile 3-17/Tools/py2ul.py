#!/usr/bin/env python3
"""py2ul.py — Python to UniLogic transpiler.

Translates Python source with type annotations into UniLogic (.ul) source.
Usage: python py2ul.py input.py [-o output.ul]
"""

import ast
import sys
import os
import textwrap


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

    def emit(self, text=""):
        self.lines.append("  " * self.depth + text)
        self.translated_lines += 1

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
            return str(node.value)
        if isinstance(node, ast.Name):
            mapped = {
                'int': 'int', 'float': 'float', 'str': 'string',
                'bool': 'bool', 'None': 'none', 'NoneType': 'none',
                'bytes': 'string', 'list': 'array string', 'List': 'array string',
                'dict': 'map', 'Dict': 'map', 'set': 'array int', 'Set': 'array int',
                'tuple': 'array int', 'Tuple': 'array int',
            }.get(node.id)
            if mapped:
                return mapped
            return node.id
        if isinstance(node, ast.Attribute):
            return self.map_type(node.attr) if isinstance(node.attr, str) else "int"
        if isinstance(node, ast.Subscript):
            base = self._name(node.value)
            if base in ('list', 'List'):
                inner = self.map_type(node.slice)
                return f"array {inner}"
            if base in ('Optional',):
                return self.map_type(node.slice)
            if base in ('tuple', 'Tuple'):
                return "array int"  # flatten
            if base in ('dict', 'Dict'):
                return "map"
            if base in ('set', 'Set'):
                return "array int"
            return self.map_type(node.slice)
        if isinstance(node, ast.BinOp) and isinstance(node.op, ast.BitOr):
            # X | None → X
            left = self.map_type(node.left)
            right = self.map_type(node.right)
            if right == 'none':
                return left
            return left
        if isinstance(node, str):
            return {'int': 'int', 'float': 'float', 'str': 'string',
                    'bool': 'bool'}.get(node, node)
        return "int"

    def _name(self, node):
        if isinstance(node, ast.Name):
            return node.id
        if isinstance(node, ast.Attribute):
            return node.attr
        return ""

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
                return f'"{v}"'
            if isinstance(node.value, bool):
                return "true" if node.value else "false"
            if node.value is None:
                return "empty"
            return str(node.value)

        if isinstance(node, ast.Name):
            if node.id == 'True': return 'true'
            if node.id == 'False': return 'false'
            if node.id == 'None': return 'empty'
            return node.id

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
                return f"bit_not({operand})"
            return operand

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
            parts = [left]
            for op, comp in zip(node.ops, node.comparators):
                parts.append(self._cmpop(op))
                parts.append(self.expr(comp))
            return " ".join(parts)

        if isinstance(node, ast.Call):
            func = self.expr(node.func)
            args = [self.expr(a) for a in node.args]
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
            idx = self.expr(node.slice)
            # Dict subscript → map_get if we can detect it's a dict
            # For now just emit array-style access; dict assignment is handled in visit_Assign
            return f"{target}[{idx}]"

        if isinstance(node, ast.List):
            elems = ", ".join(self.expr(e) for e in node.elts)
            return f"[{elems}]"

        if isinstance(node, ast.Tuple):
            elems = ", ".join(self.expr(e) for e in node.elts)
            return f"[{elems}]"

        if isinstance(node, ast.IfExp):
            # ternary: a if cond else b → no direct UL equivalent
            cond = self.expr(node.test)
            body = self.expr(node.body)
            orelse = self.expr(node.orelse)
            return f"/* {body} if {cond} else {orelse} */"

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
            # Expand to a for loop — return a placeholder and emit the loop
            # We need a temp variable name
            tmp = f"_lc{getattr(self, '_lc_counter', 0)}"
            self._lc_counter = getattr(self, '_lc_counter', 0) + 1
            elem_type = self._infer_type(node.elt)
            self.emit(f"array {elem_type} {tmp}")
            for gen in node.generators:
                target = self.expr(gen.target)
                iter_expr = self.expr(gen.iter)
                self.emit(f"for each {target} in {iter_expr}")
                self.depth += 1
                for cond in gen.ifs:
                    self.emit(f"if {self.expr(cond)}")
                    self.depth += 1
            self.emit(f"{tmp}.push({self.expr(node.elt)})")
            for gen in node.generators:
                for _ in gen.ifs:
                    self.depth -= 1
                    self.emit("end if")
                self.depth -= 1
                self.emit("end for")
            return tmp

        if isinstance(node, ast.Dict):
            # Dict → emit map_new() + map_set() calls inline is hard,
            # so return a placeholder and emit setup in the statement handler
            tmp = f"_dict{getattr(self, '_dict_counter', 0)}"
            self._dict_counter = getattr(self, '_dict_counter', 0) + 1
            self.emit(f"// TRANSPILER: dict values cast to string — may lose type information")
            self.emit(f"map {tmp} = map_new()")
            for k, v in zip(node.keys, node.values):
                if k is not None:
                    self.emit(f"map_set({tmp}, {self.expr(k)}, cast({self.expr(v)}, string))")
            return tmp

        if isinstance(node, ast.Set):
            # Set → array (no dedup at compile time)
            self.emit(f"// TRANSPILER: set translated to array — no deduplication")
            elems = ", ".join(self.expr(e) for e in node.elts)
            return f"[{elems}]"

        if isinstance(node, ast.Lambda):
            self.warn(node, "lambda function — define as named function")
            return "/* lambda */"

        if isinstance(node, ast.Starred):
            return self.expr(node.value)

        return f"/* unknown expr: {type(node).__name__} */"

    def _binop(self, op):
        ops = {
            ast.Add: '+', ast.Sub: '-', ast.Mult: '*', ast.Div: '/',
            ast.Mod: '%', ast.FloorDiv: '/', ast.Pow: '**',
            ast.BitAnd: 'bit_and', ast.BitOr: 'bit_or', ast.BitXor: 'bit_xor',
            ast.LShift: 'bit_left', ast.RShift: 'bit_right',
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
        for stmt in node.body:
            self.visit(stmt)

    def visit_FunctionDef(self, node):
        name = node.name
        # Skip self parameter for methods
        params = []
        for arg in node.args.args:
            if arg.arg == 'self':
                continue
            ptype = self.map_type(arg.annotation) if arg.annotation else "int"
            params.append(f"{ptype} {arg.arg}")

        # Handle *args → variadic
        has_varargs = False
        vararg_name = "args"
        if node.args.vararg:
            has_varargs = True
            vararg_name = node.args.vararg.arg
            params.append("...")

        # Default parameter values
        defaults = node.args.defaults
        num_non_default = len(node.args.args) - len(defaults)
        # Adjust for self in methods
        if self.in_class:
            num_non_default = max(0, num_non_default - 1)

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
            saved_lines = self.lines
            saved_depth = self.depth
            self.lines = []
            self.depth = 0
            param_str = ", ".join(params)
            self.emit(f"function {mangled}({param_str}){ret_clause}")
            self.depth += 1
            saved_func = self._current_func
            self._current_func = mangled
            for stmt in node.body:
                self.visit(stmt)
            self._current_func = saved_func
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
            self._current_func = name
            for stmt in node.body:
                self.visit(stmt)
            self._current_func = saved_func
            self.depth -= 1
            self.emit("end function")
            self.emit("")

    visit_AsyncFunctionDef = visit_FunctionDef

    def visit_ClassDef(self, node):
        name = node.name
        parent = None
        if node.bases:
            if len(node.bases) > 1:
                self.warn(node, "multiple inheritance — only first base used")
            base = node.bases[0]
            parent_name = self._name(base)
            if parent_name and parent_name != 'object':
                parent = parent_name

        inherits = f" inherits {parent}" if parent else ""
        self.emit(f"object {name}{inherits}")
        self.depth += 1
        self.in_class = name

        # Emit fields and methods
        for stmt in node.body:
            if isinstance(stmt, ast.AnnAssign) and isinstance(stmt.target, ast.Name):
                ftype = self.map_type(stmt.annotation)
                self.emit(f"{ftype} {stmt.target.id}")
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
            return "int"
        if isinstance(node, ast.Name):
            if node.id in ('True', 'False'): return "bool"
            if node.id == 'None': return "none"
        return "int"

    def visit_Assign(self, node):
        if len(node.targets) == 1:
            target_node = node.targets[0]
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
                if is_literal:
                    value = self.expr(node.value)
                    self.emit(f"const {inferred} {target} = {value}")
                elif is_simple_list:
                    value = self.expr(node.value)
                    self.emit(f"// TRANSPILER: inferred type for {target} — verify this is correct")
                    self.emit(f"const {inferred} {target} = {value}")
                else:
                    self.emit(f"// TRANSPILER: module-level variable {target} — verify type is correct")
                    self.emit(f"// Original: {target} = ...")
                return

            value = self.expr(node.value)
            self.emit(f"{target} = {value}")
        else:
            value = self.expr(node.value)
            for t in node.targets:
                self.emit(f"{self.expr(t)} = {value}")

    def visit_AnnAssign(self, node):
        target = self.expr(node.target)
        ul_type = self.map_type(node.annotation)
        if node.value:
            value = self.expr(node.value)
            if self.depth == 0 and not self.in_class:
                self.emit(f"const {ul_type} {target} = {value}")
            else:
                self.emit(f"{ul_type} {target} = {value}")
        else:
            if self.depth == 0 and not self.in_class:
                self.emit(f"// {ul_type} {target}  // module-level declaration")
            else:
                self.emit(f"{ul_type} {target}")

    def visit_AugAssign(self, node):
        target = self.expr(node.target)
        value = self.expr(node.value)
        op = self._binop(node.op)
        self.emit(f"{target} {op}= {value}")

    def visit_If(self, node):
        cond = self.expr(node.test)
        self.emit(f"if {cond}")
        self.depth += 1
        for stmt in node.body:
            self.visit(stmt)
        self.depth -= 1
        if node.orelse:
            if len(node.orelse) == 1 and isinstance(node.orelse[0], ast.If):
                # elif → else + nested if
                self.emit("else")
                self.depth += 1
                self.visit_If(node.orelse[0])
                self.depth -= 1
            else:
                self.emit("else")
                self.depth += 1
                for stmt in node.orelse:
                    self.visit(stmt)
                self.depth -= 1
        self.emit("end if")

    def visit_While(self, node):
        cond = self.expr(node.test)
        self.emit(f"while {cond}")
        self.depth += 1
        for stmt in node.body:
            self.visit(stmt)
        self.depth -= 1
        self.emit("end while")

    def visit_For(self, node):
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
                    self.emit(f"print {self.expr(args[0])}")
                else:
                    self.emit('print ""')
                return
        self.emit(self.expr(node.value))

    def visit_Pass(self, node):
        self.emit("// pass")

    def visit_Break(self, node):
        self.emit("escape")

    def visit_Continue(self, node):
        self.emit("continue")

    def visit_Try(self, node):
        self.emit("try")
        self.depth += 1
        for stmt in node.body:
            self.visit(stmt)
        self.depth -= 1
        for handler in node.handlers:
            exc_type = self._name(handler.type) if handler.type else ""
            if handler.name:
                self.emit(f"catch {exc_type} as {handler.name}")
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

    def visit_Raise(self, node):
        if node.exc:
            self.emit(f"throw {self.expr(node.exc)}")
        else:
            self.emit("throw")

    def visit_Import(self, node):
        for alias in node.names:
            self.warn(node, f"import {alias.name} — convert to UL import syntax")

    def visit_ImportFrom(self, node):
        self.warn(node, f"from {node.module} import ... — convert to UL import syntax")

    def visit_With(self, node):
        for item in node.items:
            ctx = self.expr(item.context_expr)
            if item.optional_vars:
                var = self.expr(item.optional_vars)
                self.emit(f"with {ctx} as {var}")
            else:
                self.emit(f"with {ctx}")
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
    # Insert hoisted nested functions at the top of the output
    if converter._hoisted_funcs:
        hoisted_lines = []
        for lines, orig_name, mangled in converter._hoisted_funcs:
            hoisted_lines.extend(lines)
        converter.lines = hoisted_lines + converter.lines
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
