#!/usr/bin/env python3
# REPL/repl.py — UniLogic interactive interpreter
# Run: python repl.py   or   ul
# Lexes, parses, and evaluates UL code interactively.

import sys
import os

# Ensure UTF-8 output on Windows
if sys.stdout.encoding != 'utf-8':
    sys.stdout.reconfigure(encoding='utf-8', errors='replace')
if sys.stderr.encoding != 'utf-8':
    sys.stderr.reconfigure(encoding='utf-8', errors='replace')

_base = os.path.dirname(os.path.abspath(__file__))
_root = os.path.dirname(_base)
for sub in ("Lexer", "Parser", "AST", "Semantic", "Codegen"):
    sys.path.insert(0, os.path.join(_root, sub))

from lexer import tokenize
from parser import Parser
from ast_nodes import *


# ── Colors ───────────────────────────────────────────────────────────────

NAVY  = "\033[38;2;26;74;107m"
RED   = "\033[31m"
RESET = "\033[0m"

BANNER = f"""{NAVY}
     \u221e  UniLogic
     One Language for Everything
     type 'help' for commands
{RESET}"""

HELP_TEXT = f"""{NAVY}Commands:{RESET}
  help          \u2014 show this message
  exit / quit   \u2014 leave the REPL
  functions     \u2014 list declared functions
  vars          \u2014 list declared variables
  clear         \u2014 clear all state

{NAVY}Usage:{RESET}
  Expressions are evaluated and printed:   3 + 4  \u2192  7
  Statements execute in the global scope:  int x = 10
  Function declarations are registered:    function add(...) ... end function
  Multi-line input continues until all blocks are closed."""


# ── Control-flow signals ─────────────────────────────────────────────────

class ReturnValue(Exception):
    def __init__(self, value):
        self.value = value

class BreakSignal(Exception):
    pass

class ContinueSignal(Exception):
    pass

class GotoSignal(Exception):
    """Raised by goto statements — caught by the body executor to jump to a portal."""
    def __init__(self, target):
        self.target = target

class ULException(Exception):
    """Raised by throw statements — caught by try/catch blocks."""
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)

class RuntimeErr(Exception):
    pass

class ResultPropagation_Signal(Exception):
    """Raised by ? operator when the result is an error — propagates up the call stack."""
    def __init__(self, error_value):
        self.error_value = error_value

class YieldSignal(Exception):
    """Raised by yield statements inside generator functions."""
    def __init__(self, value):
        self.value = value


# ── Multi-line detection ─────────────────────────────────────────────────

def block_depth(text):
    """Count unclosed blocks. Positive means input is incomplete."""
    depth = 0
    in_string = False
    skip_after_end = False   # skip the keyword following 'end' (e.g. end if)
    i = 0
    n = len(text)
    while i < n:
        ch = text[i]
        # Toggle string mode on unescaped quote
        if ch == '"' and (i == 0 or text[i - 1] != '\\'):
            in_string = not in_string
            i += 1
            continue
        if in_string:
            i += 1
            continue
        # Skip line comments
        if ch == '/' and i + 1 < n and text[i + 1] == '/':
            while i < n and text[i] != '\n':
                i += 1
            continue
        # Word scan
        if ch.isalpha() or ch == '_':
            j = i
            while j < n and (text[j].isalnum() or text[j] == '_'):
                j += 1
            word = text[i:j]
            if skip_after_end:
                # This is the companion keyword after 'end' — don't count it
                skip_after_end = False
            elif word in ('function', 'if', 'while', 'for', 'match', 'try', 'do', 'with'):
                depth += 1
            elif word == 'end':
                depth -= 1
                skip_after_end = True
            i = j
        else:
            i += 1
    return depth


# ── Tree-walking interpreter ─────────────────────────────────────────────

class Interpreter:
    def __init__(self):
        self.globals   = {}   # variable name -> value
        self.functions = {}   # function name -> FunctionDecl
        self.types     = {}   # type name -> TypeDecl
        self.ffi_funcs = {}   # foreign function name -> ForeignImport

    def _assign_to(self, target_node, value, env):
        """Write a value back to the location described by an AST node."""
        if isinstance(target_node, Identifier):
            name = target_node.name
            if name in env:
                env[name] = value
            elif name in self.globals:
                self.globals[name] = value
            else:
                env[name] = value
        elif isinstance(target_node, FieldAccess):
            obj = self.eval_expr(target_node.target, env)
            obj[target_node.field] = value
        elif isinstance(target_node, Index):
            obj = self.eval_expr(target_node.target, env)
            idx = self.eval_expr(target_node.index, env)
            # bytearray[i] = char → convert to int
            if isinstance(obj, bytearray) and isinstance(value, str) and len(value) == 1:
                value = ord(value)
            obj[idx] = value
        else:
            raise RuntimeErr(f"cannot assign to {type(target_node).__name__}")

    def _make_default(self, type_name):
        """Create a default value for a given UL type name."""
        if type_name in ('int', 'int8', 'int16', 'int32', 'int64',
                         'uint8', 'uint16', 'uint32', 'uint64'):
            return 0
        if type_name in ('float', 'double'):
            return 0.0
        if type_name == 'string':
            return ""
        if type_name == 'bool':
            return False
        if type_name in self.types:
            return self._make_struct(type_name)
        return None

    def _make_struct(self, type_name):
        """Instantiate a struct/object type as a dict with default field values."""
        decl = self.types.get(type_name)
        if not decl:
            return {}
        obj = {"__type__": type_name}
        # Inherited fields first
        if decl.parent and decl.parent in self.types:
            parent = self._make_struct(decl.parent)
            parent.pop("__type__", None)
            obj.update(parent)
        for f in decl.fields:
            obj[f.name] = self._make_default(f.type_.name)
        return obj

    # ── Expressions ──────────────────────────────────────────────────────

    def eval_expr(self, node, env):
        t = type(node)

        if t == IntLiteral:    return node.value
        if t == FloatLiteral:  return node.value
        if t == StringLiteral: return node.value
        if t == BoolLiteral:   return node.value
        if t == EmptyLiteral:  return None
        if t == TupleLiteral:  return tuple(self.eval_expr(e, env) for e in node.elements)
        if t == ArrayLiteral:  return [self.eval_expr(e, env) for e in node.elements]

        if t == ArrayComprehension:
            iterable = self.eval_expr(node.iterable, env)
            result = []
            for item in iterable:
                local = dict(env)
                local[node.var] = item
                result.append(self.eval_expr(node.expr, local))
            return result

        if t == Identifier:
            if node.name in env:
                return env[node.name]
            if node.name in self.globals:
                return self.globals[node.name]
            constants = getattr(self, 'constants', {})
            if node.name in constants:
                return constants[node.name]
            raise RuntimeErr(f"undefined variable '{node.name}' at {node.line}:{node.col}")

        if t == BinaryOp:
            return self.eval_binop(node, env)

        if t == UnaryOp:
            val = self.eval_expr(node.operand, env)
            if node.op == '-':       return -val
            if node.op == 'not':     return not val
            if node.op == 'bit_not': return ~int(val)
            raise RuntimeErr(f"unknown unary op '{node.op}'")

        if t == Cast:
            inner = node.expr
            if isinstance(inner, Call) and inner.name == 'change' and len(inner.args) == 1:
                inner = inner.args[0]
            val = self.eval_expr(inner, env)
            target = node.target_type.name
            if target in ('int', 'int8', 'int16', 'int32', 'int64',
                          'uint8', 'uint16', 'uint32', 'uint64'):
                return int(val)
            if target in ('float', 'double'):
                return float(val)
            if target == 'string':
                return str(val)
            if target == 'bool':
                return bool(val)
            return val

        if t == Call:
            return self.eval_call(node, env)

        if t == Index:
            target = self.eval_expr(node.target, env)
            index = self.eval_expr(node.index, env)
            return target[index]

        if t == SliceExpr:
            target = self.eval_expr(node.target, env)
            start = self.eval_expr(node.start, env)
            end = self.eval_expr(node.end, env)
            return target[start:end]

        if t == FieldAccess:
            # Enum member access: EnumName.member
            enums = getattr(self, 'enums', {})
            if isinstance(node.target, Identifier) and node.target.name in enums:
                members = enums[node.target.name]
                if node.field not in members:
                    raise RuntimeErr(f"enum '{node.target.name}' has no member '{node.field}'")
                return members[node.field]
            target = self.eval_expr(node.target, env)
            if isinstance(target, dict):
                if node.field not in target:
                    raise RuntimeErr(f"no field '{node.field}' on object")
                return target[node.field]
            raise RuntimeErr(f"field access on non-struct value")

        if t == MethodCall:
            target = self.eval_expr(node.target, env)
            args = [self.eval_expr(a, env) for a in node.args]
            # String methods
            if isinstance(target, str):
                m = node.method
                if m == 'len':          return len(target)
                if m == 'upper':        return target.upper()
                if m == 'lower':        return target.lower()
                if m == 'contains':     return args[0] in target if args else False
                if m == 'starts_with':  return target.startswith(args[0]) if args else False
                if m == 'ends_with':    return target.endswith(args[0]) if args else False
                if m == 'trim':         return target.strip()
                if m == 'split':        return target.split(args[0]) if args else target.split()
                if m == 'replace':      return target.replace(args[0], args[1]) if len(args) >= 2 else target
                if m == 'concat':       return target + (args[0] if args else "")
                if m == 'char_at':      return target[int(args[0])] if args and 0 <= int(args[0]) < len(target) else ""
                if m == 'substr':
                    start = int(args[0]) if args else 0
                    length = int(args[1]) if len(args) >= 2 else len(target) - start
                    return target[start:start+length]
                if m == 'index_of':     return target.find(args[0]) if args else -1
                raise RuntimeErr(f"unknown string method '{m}'")
            # Array methods
            if isinstance(target, list):
                m = node.method
                if m == 'len':      return len(target)
                if m == 'sort':     target.sort(); return None
                if m == 'reverse':  target.reverse(); return None
                if m == 'contains': return args[0] in target if args else False
                if m == 'sum':      return sum(target)
                if m == 'min':      return min(target) if target else 0
                if m == 'max':      return max(target) if target else 0
                if m == 'get':      return target[args[0]] if args and args[0] < len(target) else None
                if m == 'set':      target[args[0]] = args[1]; return None
                if m == 'push':     target.append(args[0]); return None
                if m == 'append':   target.append(args[0]); return None
                if m in ('pop', 'drop'):
                    if args:
                        return target.pop(int(args[0]))
                    return target.pop() if target else None
                if m == 'remove':
                    if args and args[0] in target:
                        target.remove(args[0])
                    return None
                if m == 'insert':
                    target.insert(int(args[0]), args[1]); return None
                if m == 'clear':    target.clear(); return None
                raise RuntimeErr(f"unknown array/list method '{m}'")
            # Object methods
            if isinstance(target, dict) and "__type__" in target:
                type_name = target["__type__"]
                method_key = f"{type_name}.{node.method}"
                if method_key not in self.functions:
                    # Check parent chain
                    found = False
                    obj_decl = self.types.get(type_name)
                    while obj_decl and hasattr(obj_decl, 'parent') and obj_decl.parent:
                        parent_key = f"{obj_decl.parent}.{node.method}"
                        if parent_key in self.functions:
                            method_key = parent_key
                            found = True
                            break
                        obj_decl = self.types.get(obj_decl.parent)
                    if not found:
                        raise RuntimeErr(f"no method '{node.method}' on type '{type_name}'")
                method = self.functions[method_key]
                args = [self.eval_expr(a, env) for a in node.args]
                local = {"self": target}
                # Inject object fields into method scope
                for k, v in target.items():
                    if k != "__type__":
                        local[k] = v
                for i, p in enumerate(method.params):
                    local[p.name] = args[i] if i < len(args) else self._make_default(p.type_.name)
                try:
                    self.exec_body(method.body, local)
                except ReturnValue as rv:
                    return rv.value
                return None
            raise RuntimeErr(f"method call on non-object value")

        if t == StructLiteral:
            obj = self._make_struct(node.type_name)
            for fname, fval in node.fields:
                obj[fname] = self.eval_expr(fval, env)
            return obj

        if t == PostIncrement:
            val = self.eval_expr(node.operand, env)
            self._assign_to(node.operand, val + 1, env)
            return val

        if t == PostDecrement:
            val = self.eval_expr(node.operand, env)
            self._assign_to(node.operand, val - 1, env)
            return val

        if t == OkResult:
            return (True, self.eval_expr(node.value, env))

        if t == ErrorResult:
            return (False, self.eval_expr(node.value, env))

        if t == ResultPropagation:
            result = self.eval_expr(node.expr, env)
            if not isinstance(result, tuple) or len(result) != 2:
                raise RuntimeErr(f"? operator on non-result value")
            ok, val = result
            if not ok:
                raise ResultPropagation_Signal(val)
            return val

        raise RuntimeErr(f"unknown expression {t.__name__}")

    def eval_binop(self, node, env):
        # Pipe operator: left |> right — call right(left)
        if node.op == '|>':
            left = self.eval_expr(node.left, env)
            right = node.right
            # If right is an identifier (function name), call it with left as arg
            if isinstance(right, Identifier):
                return self.eval_call(Call(right.name, [node.left], node.line, node.col), env)
            # If left is a list/generator and right is a generator function, chain
            if isinstance(left, (list, tuple)):
                if isinstance(right, Identifier) and right.name in self.functions:
                    fn = self.functions[right.name]
                    if getattr(fn, 'is_generator', False):
                        # Chain: for each item in left, run generator, collect
                        results = []
                        for item in left:
                            gen = self._run_generator(fn, [item])
                            results.extend(gen)
                        return results
            return self.eval_expr(right, env)

        # Short-circuit logical operators
        if node.op == 'and':
            return self.eval_expr(node.left, env) and self.eval_expr(node.right, env)
        if node.op == 'or':
            return self.eval_expr(node.left, env) or self.eval_expr(node.right, env)

        left  = self.eval_expr(node.left, env)
        right = self.eval_expr(node.right, env)

        op = node.op
        if op == '+':  return left + right
        if op == '-':  return left - right
        if op == '*':  return left * right
        if op == '/':
            if isinstance(left, int) and isinstance(right, int):
                # C-style truncation toward zero
                r = left // right
                if (left ^ right) < 0 and r != 0 and left % right != 0:
                    r += 1
                return r
            return left / right
        if op == '%':  return left % right
        if op == '==': return left == right
        if op == '!=': return left != right
        if op == '<':  return left < right
        if op == '>':  return left > right
        if op == '<=': return left <= right
        if op == '>=': return left >= right
        # Bitwise operators
        if op == 'bit_and':   return int(left) & int(right)
        if op == 'bit_or':    return int(left) | int(right)
        if op == 'bit_xor':   return int(left) ^ int(right)
        if op == 'xor':       return int(left) ^ int(right)
        if op == 'bit_left':  return int(left) << int(right)
        if op == 'bit_right': return int(left) >> int(right)
        if op == 'in':        return left in right
        raise RuntimeErr(f"unknown operator '{op}'")

    def eval_call(self, node, env):
        import math as _math
        name = node.name
        # For map_fn/filter_fn, don't evaluate the function name argument
        if name in ('map_fn', 'filter_fn') and len(node.args) >= 2:
            fn_ref = node.args[0]  # keep as AST node
            remaining = [self.eval_expr(a, env) for a in node.args[1:]]
            args = [fn_ref] + remaining
        else:
            args = [self.eval_expr(a, env) for a in node.args]

        # Built-in functions
        if name == 'absval':    return abs(args[0])
        if name == 'size':      return len(args[0])
        if name == 'change':    return args[0]
        if name == 'cast':      return args[0]
        if name == 'char_code':
            s, i = args[0], int(args[1])
            return ord(s[i]) if 0 <= i < len(s) else 0
        if name == 'sort':
            arr, n = args[0], int(args[1])
            sub = sorted(arr[:n])
            for i in range(n):
                arr[i] = sub[i]
            return None
        if name == 'exit':    raise SystemExit(args[0] if args else 0)
        if name == 'typeof':
            val = args[0] if args else None
            if isinstance(val, dict) and "__type__" in val:
                return val["__type__"]
            if isinstance(val, int):    return "int"
            if isinstance(val, float):  return "float"
            if isinstance(val, str):    return "string"
            if isinstance(val, bool):   return "bool"
            if isinstance(val, list):   return "array"
            return "none"
        if name == 'range':
            if len(args) == 1:   return list(range(args[0]))
            if len(args) == 2:   return list(range(args[0], args[1]))
            if len(args) == 3:   return list(range(args[0], args[1], args[2]))
            return []
        if name == 'memtake':
            return bytearray(args[0] if args else 0)
        if name == 'memgive':
            return None
        # System functions (Task 11)
        if name == 'time':
            import time as _time
            return int(_time.time())
        if name == 'clock':
            import time as _time
            return _time.time_ns()
        if name == 'sleep':
            import time as _time
            _time.sleep(args[0] / 1000)
            return None
        if name == 'random':
            import random as _rand
            return _rand.random()
        if name == 'random_int':
            import random as _rand
            return _rand.randint(args[0], args[1])
        if name == 'random_seed':
            import random as _rand
            _rand.seed(args[0])
            return None
        if name == 'userinput':
            return sys.argv
        if name == 'vault':
            return os.environ.get(args[0], '')
        if name == 'abort':
            raise SystemExit(1)
        if name == 'prompt':
            return input(args[0])
        # Python builtin equivalents (Task 15)
        if name == 'zip':
            return list(zip(args[0], args[1]))
        if name == 'enumerate':
            return list(enumerate(args[0]))
        if name == 'map_fn':
            fn_ref = args[0]  # AST Identifier node
            arr = args[1]
            fn_name = fn_ref.name if isinstance(fn_ref, Identifier) else str(fn_ref)
            if fn_name in self.functions:
                fn_decl = self.functions[fn_name]
                result = []
                for item in arr:
                    local = {fn_decl.params[0].name: item}
                    try:
                        self.exec_body(fn_decl.body, local)
                        result.append(None)
                    except ReturnValue as rv:
                        result.append(rv.value)
                return result
            raise RuntimeErr(f"map_fn: unknown function '{fn_name}'")
        if name == 'filter_fn':
            fn_ref = args[0]  # AST Identifier node
            arr = args[1]
            fn_name = fn_ref.name if isinstance(fn_ref, Identifier) else str(fn_ref)
            if fn_name in self.functions:
                fn_decl = self.functions[fn_name]
                result = []
                for item in arr:
                    local = {fn_decl.params[0].name: item}
                    try:
                        self.exec_body(fn_decl.body, local)
                        result.append(item)  # no return = include by default
                    except ReturnValue as rv:
                        if rv.value:
                            result.append(item)
                return result
            raise RuntimeErr(f"filter_fn: unknown function '{fn_name}'")
        # TODO: threading stubs
        if name == 'spawn':  return None  # TODO: threading.Thread(target=...).start()
        if name == 'wait':   return None  # TODO: thread.join()
        if name == 'lock':   return None  # TODO: threading.Lock().acquire()
        if name == 'unlock': return None  # TODO: threading.Lock().release()

        # FFI functions
        if name in self.ffi_funcs:
            fi = self.ffi_funcs[name]
            if fi.lib == 'math':
                fn = getattr(_math, name, None)
                if fn:
                    return fn(*args)
            if fi.lib == 'stdio' and name == 'puts':
                print(args[0])
                return 0
            if fi.lib == 'string':
                if name == 'strlen':
                    return len(args[0])
                if name == 'str_char_at':
                    s, i = args[0], int(args[1])
                    return s[i] if 0 <= i < len(s) else ""
                if name == 'str_substr':
                    s, start, length = args[0], int(args[1]), int(args[2])
                    return s[start:start+length]
                if name == 'str_index_of':
                    return args[0].find(args[1])
                if name == 'strcmp':
                    a, b = args[0], args[1]
                    return (a > b) - (a < b)
                if name == 'strncmp':
                    a, b, n = args[0], args[1], args[2]
                    sa, sb = a[:n], b[:n]
                    return (sa > sb) - (sa < sb)
                if name == 'strstr':
                    s, sub = args[0], args[1]
                    idx = s.find(sub)
                    return s[idx:] if idx >= 0 else None
                if name == 'strcpy':
                    dest, src = args[0], args[1]
                    if isinstance(dest, bytearray):
                        enc = src.encode() if isinstance(src, str) else src
                        dest[:len(enc)] = enc
                        dest[len(enc)] = 0
                        return dest
                    return src
                if name == 'strcat':
                    dest, src = args[0], args[1]
                    if isinstance(dest, bytearray):
                        enc = src.encode() if isinstance(src, str) else src
                        try: end = dest.index(0)
                        except ValueError: end = len(dest)
                        dest[end:end+len(enc)] = enc
                        dest[end+len(enc)] = 0
                        return dest
                    return dest + src
            raise RuntimeErr(f"FFI function '{fi.lib}.{name}' not available in interpreter")

        # User-defined functions
        if name not in self.functions:
            raise RuntimeErr(f"undefined function '{name}' at {node.line}:{node.col}")

        decl = self.functions[name]
        # Fill in default parameter values if fewer args provided
        if len(args) < len(decl.params):
            for i in range(len(args), len(decl.params)):
                p = decl.params[i]
                if p.default is not None:
                    args.append(self.eval_expr(p.default, env))
                else:
                    raise RuntimeErr(
                        f"'{name}' expects {len(decl.params)} args, got {len(args)}"
                    )
        elif len(args) > len(decl.params):
            raise RuntimeErr(
                f"'{name}' expects {len(decl.params)} args, got {len(args)}"
            )

        # Create local scope with params bound to args
        local = {}
        for param, arg in zip(decl.params, args):
            local[param.name] = arg

        # Register nested functions so they're callable within this scope
        for nf in getattr(decl, 'nested_functions', []):
            self.functions[nf.name] = nf

        try:
            self.exec_body(decl.body, local)
        except ReturnValue as rv:
            val = rv.value
            # Convert bytearray to string for string-returning functions
            if (isinstance(val, bytearray)
                    and decl.return_type
                    and decl.return_type.name == "string"):
                idx = val.find(0)
                val = val[:idx].decode() if idx >= 0 else val.decode()
            return val
        except ResultPropagation_Signal:
            # If the called function propagates an error and itself returns result,
            # re-raise so the caller's ? can catch it
            raise

        return None  # void function

    # ── Statements ───────────────────────────────────────────────────────

    def exec_stmt(self, node, env):
        t = type(node)

        if t == VarDecl:
            if node.type_.is_list:
                # list T → always a Python list
                if node.init is not None:
                    val = self.eval_expr(node.init, env)
                    if not isinstance(val, list):
                        val = [val]
                else:
                    val = []
            elif node.init is not None:
                val = self.eval_expr(node.init, env)
                # int c = string[i] → ord() conversion
                if (node.type_.name in ("int","int8","int16","int32","int64",
                                        "uint8","uint16","uint32","uint64")
                        and isinstance(val, str) and len(val) == 1):
                    val = ord(val)
            else:
                val = self._make_default(node.type_.name)
            env[node.name] = val

        elif t == Assign:
            val = self.eval_expr(node.value, env)
            if isinstance(node.target, Identifier):
                name = node.target.name
                # Find the scope that owns the variable
                if name in env:
                    target_env = env
                elif name in self.globals:
                    target_env = self.globals
                else:
                    target_env = env

                if node.op == '=':
                    target_env[name] = val
                else:
                    cur = target_env.get(name, 0)
                    OPS = {
                        '+=': lambda a, b: a + b,
                        '-=': lambda a, b: a - b,
                        '*=': lambda a, b: a * b,
                        '/=': lambda a, b: a // b if isinstance(a, int) and isinstance(b, int) else a / b,
                        '%=': lambda a, b: a % b,
                    }
                    target_env[name] = OPS[node.op](cur, val)

            elif isinstance(node.target, Index):
                target = self.eval_expr(node.target.target, env)
                index  = self.eval_expr(node.target.index, env)
                # bytearray[i] = char → convert to int
                if isinstance(target, bytearray) and isinstance(val, str) and len(val) == 1:
                    val = ord(val)
                target[index] = val

            elif isinstance(node.target, FieldAccess):
                self._field_assign(node.target, val, node.op, env)

        elif t == If:
            cond = self.eval_expr(node.condition, env)
            if cond:
                self.exec_body(node.then_body, env)
            elif node.else_body:
                self.exec_body(node.else_body, env)

        elif t == While:
            while self.eval_expr(node.condition, env):
                try:
                    self.exec_body(node.body, env)
                except BreakSignal:
                    break
                except ContinueSignal:
                    continue

        elif t == DoWhile:
            while True:
                try:
                    self.exec_body(node.body, env)
                except BreakSignal:
                    break
                except ContinueSignal:
                    pass
                if not self.eval_expr(node.condition, env):
                    break

        elif t == For:
            # Generator function call — run body, yield items
            if isinstance(node.iterable, Call) and node.iterable.name in self.functions:
                decl = self.functions[node.iterable.name]
                if decl.is_generator:
                    args = [self.eval_expr(a, env) for a in node.iterable.args]
                    iterable = self._run_generator(decl, args)
                else:
                    iterable = self.eval_expr(node.iterable, env)
            else:
                iterable = self.eval_expr(node.iterable, env)
            for item in iterable:
                env[node.var] = item
                try:
                    self.exec_body(node.body, env)
                except BreakSignal:
                    break
                except ContinueSignal:
                    continue

        elif t == CFor:
            self.exec_stmt(node.init, env)
            while self.eval_expr(node.condition, env):
                try:
                    self.exec_body(node.body, env)
                except BreakSignal:
                    break
                except ContinueSignal:
                    pass
                if node.update:
                    self.exec_stmt(node.update, env)

        elif t == Return:
            val = self.eval_expr(node.value, env) if node.value else None
            raise ReturnValue(val)

        elif t == Print:
            val = self.eval_expr(node.value, env)
            # Match compiled output: bools as 0/1, floats as %f
            if isinstance(val, bool):
                print(1 if val else 0)
            elif isinstance(val, float):
                print(f"{val:f}")
            else:
                print(val)

        elif t == ExprStmt:
            return self.eval_expr(node.expr, env)

        elif t == Escape:
            raise BreakSignal()

        elif t == Continue:
            raise ContinueSignal()

        elif t == Match:
            subj = self.eval_expr(node.subject, env)
            matched = False
            for case in node.cases:
                if case.value is None:
                    # default case
                    self.exec_body(case.body, env)
                    matched = True
                    break
                case_val = self.eval_expr(case.value, env)
                if subj == case_val:
                    self.exec_body(case.body, env)
                    matched = True
                    break

        elif t == PostIncrement:
            val = self.eval_expr(node.operand, env)
            self._assign_to(node.operand, val + 1, env)

        elif t == PostDecrement:
            val = self.eval_expr(node.operand, env)
            self._assign_to(node.operand, val - 1, env)

        elif t == Yield:
            raise YieldSignal(self.eval_expr(node.value, env))

        elif t == TupleDestructure:
            val = self.eval_expr(node.value, env)
            for i, (_, name) in enumerate(node.targets):
                env[name] = val[i]

        elif t == KillswitchStmt:
            # killswitch is active when @dr safety = checked (always active in interpreter)
            cond = self.eval_expr(node.condition, env)
            if not cond:
                raise RuntimeErr(f"killswitch failed at line {node.line}")

        elif t == PortalDecl:
            pass  # portal labels are markers — no effect when reached normally

        elif t == GotoStmt:
            raise GotoSignal(node.target)

        elif t == TryStmt:
            try:
                try:
                    self.exec_body(node.body, env)
                except ULException as ex:
                    caught = False
                    for handler in node.handlers:
                        if handler.exception_type is None or handler.exception_type == ex.type_name:
                            if handler.binding_name:
                                env[handler.binding_name] = ex.message
                            self.exec_body(handler.body, env)
                            caught = True
                            break
                    if not caught:
                        raise
            finally:
                if node.finally_body:
                    self.exec_body(node.finally_body, env)

        elif t == ThrowStmt:
            msg = self.eval_expr(node.message, env)
            raise ULException(node.exception_type, str(msg))

        elif t == WithStmt:
            resource = self.eval_expr(node.expr, env)
            env[node.binding] = resource
            try:
                self.exec_body(node.body, env)
            finally:
                # Cleanup: call close() if available
                if hasattr(resource, 'close'):
                    resource.close()

        elif t == NormDirective:
            pass  # inline @norm — no effect in interpreter

        return None

    def _field_assign(self, target, val, op, env):
        """Assign to a FieldAccess target, handling nested fields like a.b.c = val."""
        obj = self.eval_expr(target.target, env)
        if op == '=':
            obj[target.field] = val
        else:
            cur = obj.get(target.field, 0)
            OPS = {
                '+=': lambda a, b: a + b,
                '-=': lambda a, b: a - b,
                '*=': lambda a, b: a * b,
                '/=': lambda a, b: a // b if isinstance(a, int) and isinstance(b, int) else a / b,
                '%=': lambda a, b: a % b,
            }
            obj[target.field] = OPS[op](cur, val)

    def exec_body(self, stmts, env):
        result = None
        i = 0
        while i < len(stmts):
            try:
                result = self.exec_stmt(stmts[i], env)
                i += 1
            except GotoSignal as gs:
                # Scan for the portal with the given name in THIS body
                found = False
                for j, s in enumerate(stmts):
                    if isinstance(s, PortalDecl) and s.name == gs.target:
                        i = j  # jump to the portal label
                        found = True
                        break
                if not found:
                    raise  # propagate to parent body
        return result

    def _run_generator(self, decl, args):
        """Execute a generator function body, yielding values via YieldSignal."""
        local = {}
        for param, arg in zip(decl.params, args):
            local[param.name] = arg
        # Walk statements one at a time; catch YieldSignal to produce items
        # We use a coroutine-like approach with a continuation stack
        return self._gen_exec_body(decl.body, local)

    def _gen_exec_body(self, stmts, env):
        """Execute statements, yielding on YieldSignal. Returns a generator."""
        for stmt in stmts:
            yield from self._gen_exec_stmt(stmt, env)

    def _gen_exec_stmt(self, node, env):
        """Like exec_stmt but yields values instead of raising YieldSignal."""
        t = type(node)

        if t == Yield:
            yield self.eval_expr(node.value, env)
            return

        if t == While:
            while self.eval_expr(node.condition, env):
                try:
                    for v in self._gen_exec_body(node.body, env):
                        yield v
                except BreakSignal:
                    break
                except ContinueSignal:
                    continue
            return

        if t == For:
            iterable = self.eval_expr(node.iterable, env)
            for item in iterable:
                env[node.var] = item
                try:
                    for v in self._gen_exec_body(node.body, env):
                        yield v
                except BreakSignal:
                    break
                except ContinueSignal:
                    continue
            return

        if t == If:
            cond = self.eval_expr(node.condition, env)
            if cond:
                yield from self._gen_exec_body(node.then_body, env)
            elif node.else_body:
                yield from self._gen_exec_body(node.else_body, env)
            return

        # For all non-yielding statements, fall through to normal exec
        self.exec_stmt(node, env)


# ── run_program — execute a full parsed AST ──────────────────────────────

def run_program(program):
    """Execute a parsed Program AST. Calls main() if defined, otherwise
    runs top-level statements in order. Returns exit code."""
    interp = Interpreter()

    # First pass: register types, functions, FFI, objects, enums
    interp.enums = {}
    for decl in program.decls:
        if isinstance(decl, TypeDecl):
            interp.types[decl.name] = decl
        elif isinstance(decl, ObjectDecl):
            interp.types[decl.name] = decl
            for method in decl.methods:
                interp.functions[f"{decl.name}.{method.name}"] = method
        elif isinstance(decl, ConstDecl):
            interp.constants = getattr(interp, 'constants', {})
            interp.constants[decl.name] = interp.eval_expr(decl.value, {})
        elif isinstance(decl, EnumDecl):
            members = {}
            for mname, mval in decl.members:
                members[mname] = interp.eval_expr(mval, {})
            interp.enums[decl.name] = members
        elif isinstance(decl, VarDecl):
            # Module-level variable — evaluate init and store as global
            if decl.init is not None:
                interp.globals[decl.name] = interp.eval_expr(decl.init, {})
            else:
                defaults = {"int": 0, "float": 0.0, "string": "", "bool": False}
                interp.globals[decl.name] = defaults.get(decl.type_.name, None)
        elif isinstance(decl, FunctionDecl):
            interp.functions[decl.name] = decl
            # Register nested functions so they're callable
            for nf in getattr(decl, 'nested_functions', []):
                interp.functions[nf.name] = nf
        elif isinstance(decl, ForeignImport):
            interp.ffi_funcs[decl.name] = decl
        # DrDirective, NormDirective, AsmBlock — skip silently

    # Execute: call main() if it exists
    if 'main' in interp.functions:
        decl = interp.functions['main']
        local = {}
        try:
            interp.exec_body(decl.body, local)
        except ReturnValue as rv:
            if isinstance(rv.value, int):
                return rv.value
            return 0
        except ResultPropagation_Signal as rp:
            print(f"error: {rp.error_value}", file=sys.stderr)
            return 1
        except SystemExit as se:
            return se.code
        return 0
    else:
        # No main — execute top-level function bodies aren't typical,
        # but we can run any top-level expressions if the user wants
        return 0


# ── Input reader ─────────────────────────────────────────────────────────

def read_input():
    """Read possibly multi-line input. Returns None on EOF."""
    try:
        line = input(f"{NAVY}ul> {RESET}")
    except EOFError:
        return None

    buf = line
    while block_depth(buf) > 0:
        try:
            cont = input(f"{NAVY}... {RESET}")
        except EOFError:
            break
        buf += "\n" + cont

    return buf.strip()


# ── Main REPL loop ──────────────────────────────────────────────────────

def main():
    print(BANNER)
    interp = Interpreter()

    while True:
        try:
            text = read_input()
        except KeyboardInterrupt:
            print()
            break

        if text is None:
            break
        if text == "":
            continue

        # ── Built-in REPL commands ───────────────────────────────────────
        if text == "help":
            print(HELP_TEXT)
            continue

        if text in ("exit", "quit"):
            break

        if text == "functions":
            if interp.functions:
                for name, decl in interp.functions.items():
                    params = ", ".join(
                        f"{p.type_.name} {p.name}" for p in decl.params
                    )
                    ret = decl.return_type.name if decl.return_type else "none"
                    print(f"  {name}({params}) returns {ret}")
            else:
                print("  (no functions declared)")
            continue

        if text == "vars":
            if interp.globals:
                for name, val in interp.globals.items():
                    print(f"  {name} = {val}")
            else:
                print("  (no variables)")
            continue

        if text == "clear":
            interp = Interpreter()
            print("  state cleared")
            continue

        # ── Lex -> Parse -> Execute ──────────────────────────────────────
        try:
            tokens = tokenize(text, "<repl>")
            parser = Parser(tokens, "<repl>")

            if parser.check("KW", "function"):
                # Function declarations
                while not parser.at_end():
                    decl = parser.parse_function()
                    interp.functions[decl.name] = decl
                    params = ", ".join(
                        f"{p.type_.name} {p.name}" for p in decl.params
                    )
                    ret = decl.return_type.name if decl.return_type else "none"
                    print(f"{NAVY}  defined {decl.name}({params}) returns {ret}{RESET}")
            else:
                # Statements / expressions
                while not parser.at_end():
                    stmt = parser.parse_statement()
                    result = interp.exec_stmt(stmt, interp.globals)
                    # For bare expression statements, print the result
                    if isinstance(stmt, ExprStmt) and result is not None:
                        if isinstance(result, bool):
                            print(1 if result else 0)
                        elif isinstance(result, float):
                            print(f"{result:f}")
                        else:
                            print(result)

        except SyntaxError as e:
            print(f"{RED}syntax error: {e}{RESET}")
        except RuntimeErr as e:
            print(f"{RED}runtime error: {e}{RESET}")
        except ReturnValue:
            print(f"{RED}return outside function{RESET}")
        except Exception as e:
            print(f"{RED}error: {type(e).__name__}: {e}{RESET}")


if __name__ == "__main__":
    main()
