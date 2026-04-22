#!/usr/bin/env python3
"""c2ul.py -- C to UniLogic transpiler.

Translates C source into UniLogic (.ul) source.
Usage: python c2ul.py input.c [-o output.ul]

Uses pycparser to parse C. Requires fake libc headers for #include resolution.
If pycparser is not installed: pip install pycparser
"""

import sys
import os
import re
import subprocess
import tempfile
from pycparser import c_parser, c_ast, parse_file


# UL reserved keywords that can clash with C identifiers.
# When c2ul emits a name, it checks against this set and adds a '_c' suffix if needed.
_UL_KEYWORDS = {
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
    "array", "list", "map", "arena", "file",
    "ok", "error", "some",
    "object", "const", "cast", "exit", "typeof", "spawn", "wait",
    "lock", "unlock", "try_lock",
    "random", "random_int", "random_seed", "userinput",
    "integer", "teleport", "rand", "rand_int", "rand_seed", "args",
}


def _safe_name(name):
    """Rename C identifiers that clash with UL keywords by adding _c suffix."""
    if name in _UL_KEYWORDS:
        return name + "_c"
    return name


class C2UL(c_ast.NodeVisitor):
    def __init__(self):
        self.lines = []
        self.depth = 0
        self.warnings = []
        self.translated_lines = 0
        self._in_main = False
        # Maps C type names (typedef names or struct tags) that are UL user-defined types.
        # key = C name (e.g. "cJSON"), value = UL type name (same after _safe_name).
        self._known_struct_types = {}

    def emit(self, text=""):
        self.lines.append("  " * self.depth + text)
        self.translated_lines += 1

    def warn(self, node, reason):
        coord = getattr(node, 'coord', None)
        lineno = coord.line if coord else 0
        self.warnings.append((lineno, reason))
        self.emit(f"// C2UL WARNING line {lineno}: {reason}")

    # -- Type mapping ---------------------------------------------------------

    def map_type(self, node):
        """Convert a pycparser type declaration node to UL type string.
        Returns just the type name (no pointer syntax). Use map_type_ptr()
        to get (type_str, is_pointer) when you need to know if it's a pointer.
        """
        t, _ = self.map_type_ptr(node)
        return t

    def map_type_ptr(self, node):
        """Convert a pycparser type declaration node to (UL type string, is_pointer).
        Pointer syntax in UL is on the variable name: int <p>, not <int> p.
        """
        if isinstance(node, c_ast.Typename):
            return self.map_type_ptr(node.type)
        if isinstance(node, c_ast.TypeDecl):
            return self._map_type_specifier(node.type), False
        if isinstance(node, c_ast.ArrayDecl):
            inner, _ = self.map_type_ptr(node.type)
            return f"array {inner}", False
        if isinstance(node, c_ast.PtrDecl):
            # char* and unsigned char* → char pointer (explicit, not hidden as string)
            inner_node = node.type
            if isinstance(inner_node, c_ast.TypeDecl):
                spec = inner_node.type
                if isinstance(spec, c_ast.IdentifierType) and 'char' in spec.names:
                    if 'unsigned' in spec.names:
                        return "uint8", True  # unsigned char* → uint8 pointer
                    elif 'signed' not in spec.names:
                        return "char", True   # char* → char pointer (array of char)
            inner, _ = self.map_type_ptr(node.type)
            return inner, True  # pointer to inner type
        if isinstance(node, c_ast.FuncDecl):
            return self.map_type_ptr(node.type)
        coord = getattr(node, 'coord', None)
        loc = f"{coord.line}:{coord.column}" if coord else "unknown"
        raise TypeError(f"c2ul: unmapped type node {type(node).__name__} at {loc}")

    def _map_type_specifier(self, node):
        if isinstance(node, c_ast.IdentifierType):
            names = node.names
            # Handle multi-word types
            type_str = " ".join(names)
            mapping = {
                "int": "int",
                "long": "int",
                "long int": "int",
                "long long": "int64",
                "long long int": "int64",
                "short": "int",
                "short int": "int",
                "unsigned int": "uint32",
                "unsigned": "uint32",
                "unsigned long": "uint64",
                "unsigned long int": "uint64",
                "unsigned long long": "uint64",
                "unsigned long long int": "uint64",
                "unsigned short": "uint16",
                "unsigned char": "uint8",
                "signed": "int",
                "signed int": "int",
                "signed char": "int",
                "signed short": "int",
                "signed long": "int",
                "float": "float",
                "double": "double",
                "long double": "double",
                "char": "char",
                "void": "none",
                "_Bool": "bool",
                "bool": "bool",
                # stdint types
                "uint8_t": "uint8",
                "uint16_t": "uint16",
                "uint32_t": "uint32",
                "uint64_t": "uint64",
                "int8_t": "int8",
                "int16_t": "int16",
                "int32_t": "int32",
                "int64_t": "int64",
                # shorthand unsigned
                "uint": "uint32",
                # size/pointer types — 64-bit on modern systems
                "uintptr_t": "uint64",
                "intptr_t": "int64",
                "ptrdiff_t": "int64",
                "size_t": "uint64",
                "ssize_t": "int64",
                "off_t": "int64",
                "mode_t": "uint32",
                "FILE": "file",
                "va_list": "auto",
            }
            result = mapping.get(type_str, None)
            if result is not None:
                return result
            # Check if this is a known struct typedef with an actual body.
            # Only return the UL type name if the struct was collected into _struct_defs
            # (i.e., it has a real body). Forward-declared / opaque structs fall through to int.
            if type_str in self._known_struct_types:
                ul_name = self._known_struct_types[type_str]
                struct_defs = getattr(self, '_struct_defs', {})
                if ul_name in struct_defs:
                    return ul_name
            # Unknown typedef or opaque struct — error, not silent fallback
            self.warnings.append((0, f"unmapped type '{type_str}' — cannot translate"))
            return f"__unknown_{type_str}"
        if isinstance(node, c_ast.Struct):
            # Named struct — return its UL type name only if it has an actual body
            tag = node.name
            if tag and tag in self._known_struct_types:
                ul_name = self._known_struct_types[tag]
                struct_defs = getattr(self, '_struct_defs', {})
                if ul_name in struct_defs:
                    return ul_name
            self.warnings.append((0, f"unmapped struct '{tag}' — cannot translate"))
            return f"__unknown_struct_{tag}"
        if isinstance(node, c_ast.Enum):
            return "int"  # C enums are integers
        raise TypeError(f"c2ul: unmapped type specifier {type(node).__name__}: {node}")

    def get_return_type(self, func_decl):
        """Extract return type from a FuncDecl node."""
        return self.map_type(func_decl.type)

    def get_return_type_ptr(self, func_decl):
        """Extract (type, is_pointer) from a FuncDecl node."""
        return self.map_type_ptr(func_decl.type)

    def get_decl_type(self, decl_node):
        """Extract type from a Decl node."""
        return self.map_type(decl_node.type)

    def get_decl_type_ptr(self, decl_node):
        """Extract (type, is_pointer) from a Decl node."""
        return self.map_type_ptr(decl_node.type)

    # -- Expressions ----------------------------------------------------------

    def expr(self, node):
        """Convert a C expression AST node to UL source string."""
        if node is None:
            return ""

        if isinstance(node, c_ast.Constant):
            if node.type == "string":
                return node.value  # already has quotes
            if node.type == "char":
                # 'x' -> numeric value — UL expects int for char
                inner = node.value[1:-1]  # strip single quotes
                # All common C escape sequences -> their ASCII integer values
                esc_map = {
                    r'\n': '10', r'\t': '9', r'\r': '13', r'\0': '0',
                    r'\\': '92', r'\"': '34', r"\'": '39', r'\a': '7',
                    r'\b': '8', r'\f': '12', r'\v': '11', r'\?': '63',
                }
                if inner in esc_map:
                    return esc_map[inner]
                if inner.startswith(r'\x'):
                    # Hex escape
                    try:
                        return str(int(inner[2:], 16))
                    except ValueError:
                        pass
                if inner.startswith('\\') and inner[1:].isdigit():
                    # Octal escape
                    try:
                        return str(int(inner[1:], 8))
                    except ValueError:
                        pass
                if len(inner) == 1:
                    return str(ord(inner))
                return '0'  # fallback for unrecognized escapes
            # Strip numeric suffixes: L, f, F, u, U, ll, LL, etc.
            # IMPORTANT: must not strip hex digits (A-F) from hex literals
            raw = node.value
            if raw.startswith('0x') or raw.startswith('0X'):
                # Hex literal — only strip trailing non-hex chars
                val = raw.rstrip('uUlL')  # no 'f', 'F' — those are hex digits
            else:
                val = raw.rstrip('uUlLfF')
            # Handle hex literals
            if val.startswith('0x') or val.startswith('0X'):
                hex_digits = val[2:]
                if hex_digits:
                    try:
                        return str(int(hex_digits, 16))
                    except ValueError:
                        pass
                return val
            # Handle octal literals
            if len(val) > 1 and val.startswith('0') and val[1:].isdigit():
                try:
                    return str(int(val, 8))
                except ValueError:
                    return val
            return val

        if isinstance(node, c_ast.ID):
            name = node.name
            if name == "true" or name == "TRUE":
                return "true"
            if name == "false" or name == "FALSE":
                return "false"
            if name == "NULL":
                return "empty"
            # C stdlib function names used as pointer values (e.g. passed as callback)
            # Emit address-of so downstream knows this is a function reference
            _c_func_names = {
                "malloc", "calloc", "realloc", "free",
                "memcpy", "memmove", "memset",
                "printf", "fprintf", "sprintf", "snprintf",
                "strlen", "strcpy", "strncpy", "strcmp", "strncmp",
                "strcat", "strncat", "strdup",
                "fopen", "fclose", "fread", "fwrite",
                "exit", "abort", "qsort",
            }
            if name in _c_func_names:
                return f"address({_safe_name(name)})"
            return _safe_name(name)

        if isinstance(node, c_ast.BinaryOp):
            left = self.expr(node.left)
            right = self.expr(node.right)
            op = node.op
            # Map C operators to UL
            op_map = {
                "+": "+", "-": "-", "*": "*", "/": "/", "%": "%",
                "<": "<", ">": ">", "<=": "<=", ">=": ">=",
                "==": "==", "!=": "!=",
                "&&": "and", "||": "or",
                "&": "both1", "|": "either1", "^": "diff",
                "<<": "left", ">>": "right",
            }
            ul_op = op_map.get(op, op)
            # Bitwise ops are INFIX in UL: left bit_and right
            return f"({left} {ul_op} {right})"

        if isinstance(node, c_ast.UnaryOp):
            operand = self.expr(node.expr)
            if node.op == "-":
                return f"-{operand}"
            if node.op == "!":
                return f"not {operand}"
            if node.op == "~":
                return f"bitflip {operand}"
            if node.op == "p++":  # postfix ++ in expression — INCREMENT IS LOST
                self.warnings.append((0, f"postfix ++ on '{operand}' used in expression — increment dropped"))
                return operand
            if node.op == "p--":
                self.warnings.append((0, f"postfix -- on '{operand}' used in expression — decrement dropped"))
                return operand
            if node.op == "++":  # prefix ++ in expression — INCREMENT IS LOST
                self.warnings.append((0, f"prefix ++ on '{operand}' used in expression — increment dropped"))
                return operand
            if node.op == "--":
                self.warnings.append((0, f"prefix -- on '{operand}' used in expression — decrement dropped"))
                return operand
            if node.op == "&":  # address-of → address(x)
                return f"address({operand})"
            if node.op == "*":  # dereference → deref(<name>) or deref(expr)
                # Only wrap in <> if operand is a simple identifier
                if operand.isidentifier():
                    return f"deref(<{operand}>)"
                return f"deref({operand})"
            if node.op == "sizeof":
                return f"size({operand})"
            self.warnings.append((0, f"unhandled unary operator '{node.op}' on '{operand}'"))
            return f"/* ERROR: unhandled unary {node.op} */ {operand}"

        if isinstance(node, c_ast.TernaryOp):
            cond = self.expr(node.cond)
            true_val = self.expr(node.iftrue)
            false_val = self.expr(node.iffalse)
            # Hoist to temp variable with if/else block
            if not hasattr(self, '_ternary_counter'):
                self._ternary_counter = 0
            self._ternary_counter += 1
            tmp = f"_tern_{self._ternary_counter}"
            self.emit(f"auto {tmp} = {false_val}")
            self.emit(f"if {cond}")
            self.depth += 1
            self.emit(f"{tmp} = {true_val}")
            self.depth -= 1
            self.emit("end if")
            return tmp

        if isinstance(node, c_ast.FuncCall):
            return self._func_call_expr(node)

        if isinstance(node, c_ast.ArrayRef):
            arr = self.expr(node.name)
            idx = self.expr(node.subscript)
            return f"{arr}[{idx}]"

        if isinstance(node, c_ast.StructRef):
            obj = self.expr(node.name)
            field = _safe_name(node.field.name)
            if node.type == "->":
                return f"{obj}.{field}"
            return f"{obj}.{field}"

        if isinstance(node, c_ast.Assignment):
            lhs = self.expr(node.lvalue)
            rhs = self.expr(node.rvalue)
            if node.op == "=":
                return f"{lhs} = {rhs}"
            # +=, -=, etc.
            base_op = node.op[:-1]  # strip the =
            bitwise_map = {
                "&": "both1", "|": "either1", "^": "diff",
                "<<": "left", ">>": "right",
            }
            if base_op in bitwise_map:
                ul_op = bitwise_map[base_op]
                return f"{lhs} = ({lhs} {ul_op} {rhs})"
            return f"{lhs} = {lhs} {base_op} {rhs}"

        if isinstance(node, c_ast.Cast):
            target_type = self.map_type(node.to_type)
            val = self.expr(node.expr)
            return f"cast({val}, {target_type})"

        if isinstance(node, c_ast.ExprList):
            return ", ".join(self.expr(e) for e in node.exprs)

        if isinstance(node, c_ast.Compound):
            # Shouldn't appear in expressions normally
            return "/* compound */"

        if isinstance(node, c_ast.InitList):
            items = [self.expr(e) for e in node.exprs]
            return f"[{', '.join(items)}]"

        if isinstance(node, c_ast.NamedInitializer):
            return self.expr(node.expr)

        if isinstance(node, c_ast.Typename):
            # sizeof(Typename) or cast — just return the mapped type or 0
            return self.map_type(node.type)

        self.warn(node, f"Unhandled expression type: {type(node).__name__}")
        return f"/* ERROR: unhandled {type(node).__name__} */"

    def _func_call_expr(self, node):
        """Handle function call expression."""
        # Get function name: use raw C name first (to match explicit stdlib handlers),
        # then apply _safe_name only for names not already handled below.
        if isinstance(node.name, c_ast.ID):
            raw_name = node.name.name
            func_name = raw_name  # will be renamed later if needed
        else:
            func_name = self.expr(node.name)
        args = []
        if node.args:
            for a in node.args.exprs:
                args.append(self.expr(a))

        # Map common C functions to UL equivalents
        if func_name == "printf":
            return self._convert_printf(args)
        if func_name == "fprintf":
            # Skip file arg, convert rest like printf
            if len(args) > 1:
                return self._convert_printf(args[1:])
            self.warnings.append((0, "fprintf() called with no format args"))
            return "/* ERROR: fprintf no format */"
        if func_name in ("sprintf", "snprintf", "vsprintf", "vsnprintf"):
            self.warnings.append((0, f"'{func_name}' has no UL equivalent — call dropped"))
            return f"/* ERROR: {func_name} not translatable */"
        if func_name == "strlen":
            if not args:
                self.warnings.append((0, "strlen() called with no arguments"))
                return "/* ERROR: strlen no args */"
            return f"size({args[0]})"
        if func_name == "strcmp":
            # C strcmp returns 0 for equal. Map to: (a == b) ? 0 : 1
            # so strcmp(a,b)==0 → (a==b ? 0 : 1)==0 works, but simpler:
            # emit a helper expression. Since callers check ==0 or !=0,
            # return "not (a == b)" which is 1 when different, 0 when equal — matches C semantics.
            if len(args) >= 2:
                if not hasattr(self, '_ternary_counter'):
                    self._ternary_counter = 0
                self._ternary_counter += 1
                tmp = f"_tern_{self._ternary_counter}"
                self.emit(f"int {tmp} = 1")
                self.emit(f"if {args[0]} == {args[1]}")
                self.depth += 1
                self.emit(f"{tmp} = 0")
                self.depth -= 1
                self.emit("end if")
                return tmp
            self.warnings.append((0, "strcmp() called with insufficient arguments"))
            return "/* ERROR: strcmp needs 2 args */"
        if func_name == "rand":
            return "random_int(0, 32767)"
        if func_name == "srand":
            return f"random_seed({args[0]})" if args else "random_seed(0)"
        if func_name == "atoi":
            return f"cast({args[0]}, int)" if args else "0"
        if func_name == "atof":
            return f"cast({args[0]}, float)" if args else "0"
        if func_name == "abs":
            return f"absval({args[0]})" if args else "0"
        if func_name in ("malloc", "s_malloc"):
            return f"memtake({args[0]})" if args else "memtake(0)"
        if func_name == "calloc":
            # calloc(n, size) → memtake(n * size)
            if len(args) >= 2:
                return f"memtake({args[0]} * {args[1]})"
            return f"memtake({args[0]})" if args else "memtake(0)"
        if func_name in ("realloc", "s_realloc", "s_calloc"):
            # realloc(ptr, size) → memtake(size) — UL has no realloc equivalent
            if len(args) >= 2:
                return f"memtake({args[1]})"
            return f"memtake({args[0]})" if args else "memtake(0)"
        if func_name in ("free", "s_free"):
            return f"memgive({args[0]})" if args else "memgive(0)"
        if func_name == "exit":
            return f"exit({args[0]})" if args else "exit(0)"
        if func_name in ("assert", "_assert", "__assert", "__assert_fail",
                         "__assert_func", "__assert_rtn"):
            # assert → killswitch (UL's assert/panic)
            if args:
                return f"killswitch({args[0]})"
            return "killswitch(0)"
        # C stdlib functions that have direct UL equivalents — map them
        _c_to_ul_map = {
            # memory ops → UL builtins
            "memcpy": "memcopy", "bcopy": "memcopy",
            "memmove": "memmove", "memset": "memset",
            # string comparison → strcmp already handled above
            "strncmp": "strncmp",  # pass through, codegen handles
            # type conversions
            "strtod": "cast",  "strtol": "cast", "strtoul": "cast",
            "strtoll": "cast", "strtoull": "cast",
            # char classification → pass through (codegen_c maps back)
            "toupper": "toupper", "tolower": "tolower",
            "isdigit": "isdigit", "isalpha": "isalpha", "isspace": "isspace",
            "isprint": "isprint", "isupper": "isupper", "islower": "islower",
            "isalnum": "isalnum",
            # math → pass through (codegen_c emits same names, needs math.h)
            "floor": "floor", "ceil": "ceil", "round": "round",
            "pow": "pow", "sqrt": "sqrt", "log": "log", "exp": "exp",
            "fabs": "fabs", "modf": "modf", "fmod": "fmod",
            "frexp": "frexp", "ldexp": "ldexp",
            # string ops → pass through
            "strcpy": "strcpy", "strncpy": "strncpy",
            "strcat": "strcat", "strncat": "strncat",
            "strdup": "strdup", "strndup": "strndup",
            "strchr": "strchr", "strrchr": "strrchr",
            "strstr": "strstr", "strtok": "strtok",
            "memcmp": "memcmp", "strcasecmp": "strcasecmp",
            "strncasecmp": "strncasecmp",
            # file I/O → pass through
            "fopen": "fopen", "fclose": "fclose",
            "fread": "fread", "fwrite": "fwrite", "fflush": "fflush",
            "fgets": "fgets", "fputs": "fputs",
            "fseek": "fseek", "ftell": "ftell", "rewind": "rewind",
            "feof": "feof", "ferror": "ferror",
            # system
            "getenv": "vault", "system": "system",
            "qsort_r": "qsort_r", "bsearch": "bsearch",
            "setenv": "setenv",
        }
        if func_name in _c_to_ul_map:
            ul_name = _c_to_ul_map[func_name]
            # Special handling for strtol/strtod family: strtol(str, NULL, base) → cast(str, int)
            if func_name in ("strtod", "strtol", "strtoul", "strtoll", "strtoull"):
                target = "int" if "int" in func_name or "long" in func_name or "tol" in func_name else "float"
                return f"cast({args[0]}, {target})" if args else "0"
            if func_name == "getenv":
                return f"vault({args[0]})" if args else "vault(\"\")"
            return f"{ul_name}({', '.join(args)})"
        # Apply _safe_name to avoid UL keyword clashes for non-stdlib functions
        if isinstance(node.name, c_ast.ID):
            func_name = _safe_name(raw_name)
        # Remaining direct maps
        c_to_ul_funcs = {"memcpy": "memcopy", "bcopy": "memcopy"}
        if func_name in c_to_ul_funcs:
            func_name = c_to_ul_funcs[func_name]

        return f"{func_name}({', '.join(args)})"

    def _convert_printf(self, args):
        """Convert printf format string + args to UL print expression.
        Returns a string like: "prefix" + cast(x, string) + " suffix"
        """
        if not args:
            return '""'

        fmt = args[0]
        # Strip quotes
        if fmt.startswith('"') and fmt.endswith('"'):
            fmt = fmt[1:-1]
        else:
            # Not a literal format string — can't convert
            return f'"" // printf with dynamic format - not converted'

        rest = args[1:]
        arg_idx = 0
        parts = []
        current = ""
        i = 0
        while i < len(fmt):
            if fmt[i] == '%' and i + 1 < len(fmt):
                i += 1
                # Skip flags, width, precision
                while i < len(fmt) and fmt[i] in '0123456789.-+ #l':
                    i += 1
                if i < len(fmt):
                    spec = fmt[i]
                    if spec == '%':
                        current += '%'
                    elif spec in ('d', 'i', 'u', 'x', 'X', 'o'):
                        if current:
                            parts.append(f'"{current}"')
                            current = ""
                        if arg_idx < len(rest):
                            parts.append(f"cast({rest[arg_idx]}, string)")
                            arg_idx += 1
                    elif spec in ('f', 'e', 'E', 'g', 'G'):
                        if current:
                            parts.append(f'"{current}"')
                            current = ""
                        if arg_idx < len(rest):
                            parts.append(f"cast({rest[arg_idx]}, string)")
                            arg_idx += 1
                    elif spec == 's':
                        if current:
                            parts.append(f'"{current}"')
                            current = ""
                        if arg_idx < len(rest):
                            parts.append(rest[arg_idx])
                            arg_idx += 1
                    elif spec == 'c':
                        if current:
                            parts.append(f'"{current}"')
                            current = ""
                        if arg_idx < len(rest):
                            parts.append(f"cast({rest[arg_idx]}, string)")
                            arg_idx += 1
                    else:
                        current += f"%{spec}"
                i += 1
            elif fmt[i] == '\\' and i + 1 < len(fmt):
                esc = fmt[i + 1]
                if esc == 'n':
                    current += '\\n'
                elif esc == 't':
                    current += '\\t'
                elif esc == '\\':
                    current += '\\\\'
                elif esc == '"':
                    current += '\\"'
                else:
                    current += fmt[i:i+2]
                i += 2
            else:
                current += fmt[i]
                i += 1

        if current:
            parts.append(f'"{current}"')

        if not parts:
            return '""'

        return " + ".join(parts)

    # -- Statements -----------------------------------------------------------

    def _prepass_register_structs(self, node):
        """Walk the entire FileAST and populate _known_struct_types before translation.
        This ensures all struct typedef names are known when we translate function bodies,
        regardless of the order they appear in the source.
        """
        for ext in node.ext:
            if isinstance(ext, c_ast.Typedef):
                name = ext.name or "unnamed"
                inner = ext.type
                if not isinstance(inner, c_ast.TypeDecl):
                    continue
                type_node = inner.type
                if isinstance(type_node, c_ast.Struct):
                    ul_name = _safe_name(name)
                    self._known_struct_types[name] = ul_name
                    if type_node.name:
                        self._known_struct_types[type_node.name] = ul_name
            elif isinstance(ext, c_ast.Decl):
                # Bare struct definition: struct Foo { ... };
                type_node = ext.type
                if isinstance(type_node, c_ast.TypeDecl):
                    type_node = type_node.type
                if isinstance(type_node, c_ast.Struct) and type_node.name:
                    ul_name = _safe_name(type_node.name)
                    self._known_struct_types[type_node.name] = ul_name

    def visit_FileAST(self, node):
        # Track which struct names we have already emitted as 'type' blocks
        self._emitted_types = set()
        # Pre-pass: register all struct typedef names before translation
        self._prepass_register_structs(node)
        # Pre-pass: collect all struct definitions for topological ordering
        self._struct_defs = {}  # ul_name -> struct c_ast.Struct node
        self._collect_struct_defs(node)
        # Emit all type blocks in dependency order before functions/globals
        self._emit_all_types_topo()
        # Now emit globals and functions
        for ext in node.ext:
            if isinstance(ext, c_ast.FuncDef):
                self.visit_FuncDef(ext)
                self.emit()
            elif isinstance(ext, c_ast.Decl):
                self._visit_global_decl(ext)
                self.emit()
            elif isinstance(ext, c_ast.Typedef):
                pass  # types already emitted above
            else:
                self.generic_visit(ext)
                self.emit()

    def _collect_struct_defs(self, node):
        """Collect all struct definitions into self._struct_defs keyed by UL type name."""
        for ext in node.ext:
            if isinstance(ext, c_ast.Typedef):
                name = ext.name or "unnamed"
                inner = ext.type
                if not isinstance(inner, c_ast.TypeDecl):
                    continue
                type_node = inner.type
                if isinstance(type_node, c_ast.Struct) and type_node.decls:
                    ul_name = _safe_name(name)
                    if ul_name not in self._struct_defs:
                        self._struct_defs[ul_name] = type_node
            elif isinstance(ext, c_ast.Decl):
                type_node = ext.type
                if isinstance(type_node, c_ast.TypeDecl):
                    type_node = type_node.type
                if isinstance(type_node, c_ast.Struct) and type_node.decls and type_node.name:
                    ul_name = _safe_name(type_node.name)
                    if ul_name not in self._struct_defs:
                        self._struct_defs[ul_name] = type_node

    def _emit_all_types_topo(self):
        """Emit all struct type blocks in topological dependency order."""
        for ul_name in list(self._struct_defs.keys()):
            self._emit_type_topo(ul_name)

    def _emit_type_topo(self, ul_name):
        """Recursively emit a type block, emitting non-pointer struct dependencies first.

        Pointer-to-struct fields (e.g. Table *pTable) are always emitted as int because:
        1. UL has no pointer types — pointers are opaque handles.
        2. C circular struct dependencies always go through pointers, so mapping
           pointer-to-struct fields as int eliminates all circular dependency problems.
        Only directly embedded structs (rare in C, usually via non-pointer field) require
        the dependency to be emitted first.
        """
        if ul_name in self._emitted_types:
            return
        if ul_name not in self._struct_defs:
            return
        # Mark early to break any remaining circular references
        self._emitted_types.add(ul_name)
        struct_node = self._struct_defs[ul_name]

        def _is_ptr_field(fd):
            """Return True if this field is a pointer (or array of pointers) type."""
            t = fd.type
            # Unwrap ArrayDecl to its element type
            while isinstance(t, c_ast.ArrayDecl):
                t = t.type
            return isinstance(t, c_ast.PtrDecl)

        # Emit dependencies: only NON-pointer struct fields create real embedding deps
        if struct_node.decls:
            for field_decl in struct_node.decls:
                if not isinstance(field_decl, c_ast.Decl):
                    continue
                if _is_ptr_field(field_decl):
                    continue  # pointer fields don't create embedding deps (no circular issue)
                field_type = self.get_decl_type(field_decl)
                if field_type in self._struct_defs:
                    self._emit_type_topo(field_type)

        # Emit this type block
        self.emit(f"type {ul_name}")
        self.depth += 1
        if struct_node.decls:
            for field_decl in struct_node.decls:
                if not isinstance(field_decl, c_ast.Decl):
                    continue
                field_name = _safe_name(field_decl.name or "_")
                field_type, field_is_ptr = self.get_decl_type_ptr(field_decl)
                field_decl_name = f"<{field_name}>" if field_is_ptr else field_name
                # For array fields in structs, include size so codegen_c can emit
                # a proper fixed-size C array (uint32_t state[4]) rather than a pointer.
                if isinstance(field_decl.type, c_ast.ArrayDecl):
                    arr_size = self._get_array_size(field_decl.type)
                    if arr_size is not None and field_type.startswith("array "):
                        base = field_type[len("array "):]
                        field_type = f"array {base}[{arr_size}]"
                self.emit(f"{field_type} {field_decl_name}")
        self.depth -= 1
        self.emit("end type")
        self.emit()

    def _visit_typedef(self, node):
        """No-op: type blocks are now emitted in topological order by _emit_all_types_topo.
        Kept for legacy call sites; visit_FileAST no longer calls this directly."""
        pass

    def _maybe_emit_bare_struct(self, type_node):
        """No-op: bare struct type blocks are emitted by _emit_all_types_topo.
        _visit_global_decl still calls this for nameless decls — handled by topo pass."""
        pass

    def _visit_global_decl(self, node):
        """Handle global variable or function prototype declaration."""
        if isinstance(node.type, c_ast.FuncDecl):
            if node.init:
                # Function pointer variable with initializer: e.g. parson_malloc = malloc
                self._emit_func_ptr_decl(node)
                return
            # Function prototype — skip (we'll get the definition)
            return
        name = node.name
        if name is None:
            # Nameless decl — may carry a bare struct definition: struct Foo { ... };
            self._maybe_emit_bare_struct(node.type)
            return
        ul_name = _safe_name(name)
        ul_type = self.get_decl_type(node)
        # UL supports module-level variable declarations (int x = 5).
        # Emit simple scalar/string globals as module-level vars.
        # Struct/array initializers (NamedInitializer, InitList) are emitted as
        # zero-init module-level vars (the struct layout matches because we emit type blocks).
        if node.init:
            if isinstance(node.init, c_ast.Constant) and node.init.type == "string":
                ul_type = "string"
                init_val = self.expr(node.init)
                self.emit(f"constant {ul_type} {ul_name} = {init_val}")
            elif isinstance(node.init, c_ast.InitList):
                # Array or struct initializer list.
                # Rules:
                #  - Named initializers (.field = val) → struct init → zero-init
                #  - Type is a known struct type → struct init → zero-init
                #  - Otherwise (flat scalar list, C type is array) → UL array literal
                exprs = node.init.exprs or []
                is_struct_type = ul_type in self._known_struct_types.values()
                has_named_init = exprs and isinstance(exprs[0], c_ast.NamedInitializer)
                # Also treat it as a struct if the underlying C type is a struct/typedef-of-struct
                raw_type = node.type
                if isinstance(raw_type, c_ast.TypeDecl) and isinstance(raw_type.type, c_ast.Struct):
                    is_struct_type = True
                # If the C declaration is an ArrayDecl, it's always a genuine C array.
                is_c_array = isinstance(node.type, c_ast.ArrayDecl)
                # Check underlying type via pycparser
                if is_c_array and not has_named_init and exprs:
                    # Flat scalar array init — emit as UL array literal [v1, v2, ...]
                    items = [self.expr(e) for e in exprs]
                    self.emit(f"{ul_type} {ul_name} = [{', '.join(items)}]")
                else:
                    # Struct init or named initializer — zero-init module-level var
                    self.emit(f"{ul_type} {ul_name} = 0")
            elif isinstance(node.init, (c_ast.NamedInitializer, c_ast.CompoundLiteral)):
                # Struct/compound init — emit as zero-init module-level var
                self.emit(f"{ul_type} {ul_name} = 0")
            else:
                init_val = self.expr(node.init)
                self.emit(f"{ul_type} {ul_name} = {init_val}")
        else:
            # No initializer — emit appropriate zero-init
            if ul_type.startswith("array "):
                arr_size = self._get_array_size(node.type)
                if arr_size is not None:
                    base_type = ul_type[len("array "):].strip()
                    elem_default = {"int": "0", "float": "0.0", "string": '""',
                                    "bool": "false"}.get(base_type, "0")
                    zeros = ", ".join([elem_default] * arr_size)
                    self.emit(f"{ul_type} {ul_name} = [{zeros}]")
                else:
                    self.emit(f"{ul_type} {ul_name} = []")
            else:
                self.emit(f"{ul_type} {ul_name} = 0")

    def _emit_func_ptr_decl(self, node):
        """Emit a function pointer variable declaration.
        C: returntype (*name)(paramtypes) = funcname;
        UL: returntype <name> = funcname(type param, type param)
        """
        func_decl = node.type  # c_ast.FuncDecl
        ret_type, ret_is_ptr = self.get_return_type_ptr(func_decl)
        name = _safe_name(node.name or "unnamed")
        # Build parameter list
        params = []
        if func_decl.args:
            for p in func_decl.args.params:
                if isinstance(p, c_ast.EllipsisParam):
                    continue
                p_type, p_is_ptr = self.get_decl_type_ptr(p)
                if p_type == "none" and p.name is None:
                    continue
                p_name = _safe_name(p.name) if p.name else "_"
                if p_is_ptr:
                    params.append(f"{p_type} <{p_name}>")
                else:
                    params.append(f"{p_type} {p_name}")
        params_str = ", ".join(params)
        # Get the function name being pointed to
        init_name = "0"
        if node.init:
            init_name = self.expr(node.init)
        self.emit(f"{ret_type} <{name}> = {init_name}({params_str})")

    def visit_FuncDef(self, node):
        """Emit a UL function definition."""
        func_name = _safe_name(node.decl.name)
        self._in_main = (node.decl.name == "main")
        func_decl = node.decl.type
        ret_type, ret_is_ptr = self.get_return_type_ptr(func_decl)

        # Parameters
        params = []
        is_variadic = False
        if func_decl.args:
            for p in func_decl.args.params:
                if isinstance(p, c_ast.EllipsisParam):
                    is_variadic = True
                    continue  # handle below
                p_type, p_is_ptr = self.get_decl_type_ptr(p)
                # C 'void' as sole param means no params (e.g. f(void))
                if p_type == "none" and p.name is None:
                    continue
                p_name = _safe_name(p.name) if p.name else "_"
                if p_is_ptr:
                    params.append(f"{p_type} <{p_name}>")
                else:
                    params.append(f"{p_type} {p_name}")

        if is_variadic:
            params.append("...")
        params_str = ", ".join(params)
        if ret_type == "none" or ret_type == "void":
            if ret_is_ptr:
                # void* return → returns <none>
                self.emit(f"function {func_name}({params_str}) returns <none>")
            else:
                self.emit(f"function {func_name}({params_str})")
        else:
            if ret_is_ptr:
                self.emit(f"function {func_name}({params_str}) returns <{ret_type}>")
            else:
                self.emit(f"function {func_name}({params_str}) returns {ret_type}")

        self.depth += 1
        # Track locally declared variable names so that C block-scoped
        # re-declarations of the same name (e.g. `unsigned char dst[32]`
        # in multiple inner {} blocks) are emitted as plain assignments
        # instead of new declarations (UL has a single flat function scope).
        self._local_vars = set()
        # Seed with parameter names so re-declaration of a param also assigns.
        for p in params:
            parts = p.split()
            if parts:
                self._local_vars.add(parts[-1])
        if node.body:
            self._visit_compound(node.body)
        self.depth -= 1
        self._local_vars = set()
        self.emit("end function")

    def _visit_compound(self, node):
        """Visit a compound statement (block).

        When inside a while loop that uses break-flag emulation, guard all
        statements that follow a break-containing statement with 'if not _brk_N'.
        This ensures that after a break occurs, the remaining loop body is skipped.
        """
        if not node.block_items:
            return
        # Check if we're in a break-flag context
        brk_var = (self._break_flags[-1]
                   if hasattr(self, '_break_flags') and self._break_flags
                   else None)
        if brk_var is None:
            # No break flag — simple iteration
            for item in node.block_items:
                self._visit_stmt(item)
            return
        # Break-flag context: after each statement that COULD set the flag,
        # guard subsequent statements with 'if not _brk_var'.
        # Strategy: visit statements one at a time; after any stmt that contains
        # a break (or is a Break), wrap remaining statements in a guard.
        items = node.block_items
        i = 0
        while i < len(items):
            item = items[i]
            self._visit_stmt(item)
            i += 1
            # If this statement could have set the break flag, guard the rest
            if self._has_break(item) or isinstance(item, c_ast.Break):
                if i < len(items):
                    self.emit(f"if not {brk_var}")
                    self.depth += 1
                    while i < len(items):
                        self._visit_stmt(items[i])
                        i += 1
                    self.depth -= 1
                    self.emit("end if")
                break

    def _visit_stmt(self, node):
        """Visit a single statement."""
        if isinstance(node, c_ast.Decl):
            self._visit_local_decl(node)
        elif isinstance(node, c_ast.Assignment):
            self._visit_assignment(node)
        elif isinstance(node, c_ast.If):
            self._visit_if(node)
        elif isinstance(node, c_ast.While):
            self._visit_while(node)
        elif isinstance(node, c_ast.DoWhile):
            self._visit_dowhile(node)
        elif isinstance(node, c_ast.For):
            self._visit_for(node)
        elif isinstance(node, c_ast.Return):
            self._visit_return(node)
        elif isinstance(node, c_ast.FuncCall):
            self._visit_func_call_stmt(node)
        elif isinstance(node, c_ast.Compound):
            self._visit_compound(node)
        elif isinstance(node, c_ast.UnaryOp):
            self._visit_unary_stmt(node)
        elif isinstance(node, c_ast.Switch):
            self._visit_switch(node)
        elif isinstance(node, c_ast.Break):
            # If inside a while loop that uses break-flag emulation, set the flag.
            # Otherwise (inside switch), breaks are implicit in UL — skip.
            if hasattr(self, '_break_flags') and self._break_flags:
                brk_var = self._break_flags[-1]
                self.emit(f"{brk_var} = 1")
            # else: switch break — implicit in UL, skip
        elif isinstance(node, c_ast.Continue):
            self.emit("continue")
        elif isinstance(node, c_ast.EmptyStatement):
            pass
        elif isinstance(node, c_ast.Label):
            self.emit(f"portal {_safe_name(node.name)}")
            if node.stmt:
                self._visit_stmt(node.stmt)
        elif isinstance(node, c_ast.Goto):
            self.emit(f"goto {_safe_name(node.name)}")
        elif isinstance(node, c_ast.Cast):
            # (void)(expr) used as statement (e.g. assert expansion) — skip void casts
            target = self.map_type(node.to_type)
            if target == "none":
                pass  # void cast as statement — no-op
            else:
                e = self.expr(node)
                if e:
                    self.emit(e)
        else:
            # Try expression statement
            e = self.expr(node)
            if e:
                self.emit(e)

    def _get_array_size(self, type_node):
        """Return the integer size of a C ArrayDecl node, or None if unknown."""
        if isinstance(type_node, c_ast.ArrayDecl):
            dim = type_node.dim
            if isinstance(dim, c_ast.Constant) and dim.type in ('int', 'unsigned int'):
                try:
                    return int(dim.value)
                except ValueError:
                    pass
        return None

    def _visit_local_decl(self, node):
        """Handle local variable declaration.

        UL has a flat function scope — there is no block-level shadowing.
        When the same variable name is declared more than once (e.g. in
        multiple C inner {} blocks), emit a plain assignment for the
        subsequent declarations instead of a new typed declaration.
        """
        if isinstance(node.type, c_ast.FuncDecl):
            # Function pointer variable: returntype <name> = funcname(type param, ...)
            self._emit_func_ptr_decl(node)
            if hasattr(self, '_local_vars'):
                self._local_vars.add(_safe_name(node.name or "unnamed"))
            return
        ul_type, is_ptr = self.get_decl_type_ptr(node)
        name = _safe_name(node.name or "unnamed")
        # Pointer variables: type <name> syntax
        decl_name = f"<{name}>" if is_ptr else name

        # Check if already declared in this function scope
        already_declared = hasattr(self, '_local_vars') and name in self._local_vars
        if hasattr(self, '_local_vars'):
            self._local_vars.add(name)

        if node.init:
            if isinstance(node.init, c_ast.InitList):
                items = [self.expr(e) for e in node.init.exprs]
                if already_declared:
                    self.emit(f"{name} = [{', '.join(items)}]")
                else:
                    self.emit(f"{ul_type} {decl_name} = [{', '.join(items)}]")
            else:
                init_val = self.expr(node.init)
                if already_declared:
                    self.emit(f"{name} = {init_val}")
                else:
                    self.emit(f"{ul_type} {decl_name} = {init_val}")
        else:
            # For array types without an initializer, emit a zero-filled array literal
            # of the correct size (derived from the C ArrayDecl dimension).
            if ul_type.startswith("array "):
                arr_size = self._get_array_size(node.type)
                if arr_size is not None:
                    # Determine the base element default
                    base_type = ul_type[len("array "):].strip()
                    elem_default = {"int": "0", "float": "0.0", "string": '""',
                                    "bool": "false"}.get(base_type, "0")
                    zeros = ", ".join([elem_default] * arr_size)
                    default = f"[{zeros}]"
                else:
                    default = "[]"  # dynamic/unknown size — empty list
                if already_declared:
                    self.emit(f"{name} = {default}")
                else:
                    self.emit(f"{ul_type} {decl_name} = {default}")
                return
            defaults = {"int": "0", "float": "0.0", "string": '""', "bool": "false"}
            default = defaults.get(ul_type, "0")
            if already_declared:
                self.emit(f"{name} = {default}")
            else:
                self.emit(f"{ul_type} {decl_name} = {default}")

    def _visit_assignment(self, node):
        """Handle assignment statement.
        Handles chained assignments (a = b = c) by emitting them in order:
            b = c
            a = b
        """
        lhs = self.expr(node.lvalue)
        # Chained assignment: rhs is itself an Assignment (e.g. a = b = c)
        if node.op == "=" and isinstance(node.rvalue, c_ast.Assignment):
            self._emit_chained_assign(node)
            return
        rhs = self.expr(node.rvalue)
        if node.op == "=":
            self.emit(f"{lhs} = {rhs}")
        else:
            base_op = node.op[:-1]
            # Map bitwise compound assignment operators to UL infix form
            bitwise_map = {
                "&": "both1", "|": "either1", "^": "diff",
                "<<": "left", ">>": "right",
            }
            if base_op in bitwise_map:
                ul_op = bitwise_map[base_op]
                self.emit(f"{lhs} = ({lhs} {ul_op} {rhs})")
            else:
                self.emit(f"{lhs} = {lhs} {base_op} {rhs}")

    def _emit_chained_assign(self, node):
        """Recursively emit chained assignments right-to-left.
        C: a = b = c  →  UL: b = c; a = b
        C: a = (b += c)  →  UL: b = b + c; a = b
        """
        # Collect all lhs values from the simple-assignment chain
        chain = []
        current = node
        while isinstance(current, c_ast.Assignment) and current.op == "=":
            chain.append(self.expr(current.lvalue))
            current = current.rvalue
        # 'current' is the rightmost value — may be a compound assignment (+=, etc.)
        if isinstance(current, c_ast.Assignment) and current.op != "=":
            # Compound assignment as rvalue (e.g. b += c) — emit it as a statement first
            self._visit_assignment(current)
            rhs_val = self.expr(current.lvalue)  # the lhs of the inner assign is the new rhs
        else:
            rhs_val = self.expr(current)
        # Emit from right to left
        for lhs_val in reversed(chain):
            self.emit(f"{lhs_val} = {rhs_val}")
            rhs_val = lhs_val  # each next lhs uses previous lhs as rhs

    def _hoist_assignments_from_cond(self, cond_node):
        """Extract assignment sub-expressions from a condition, emitting them as statements.

        C allows assignments in conditions: if ((x = f()) != 0) {...}
        UL does not. This method walks the condition AST, emits any assignment
        nodes as statements, and returns a condition string with the assignment
        replaced by its LHS.
        """
        def _hoist(node):
            if node is None:
                return node
            if isinstance(node, c_ast.Assignment) and node.op == "=":
                self._visit_assignment(node)
                return node.lvalue
            if isinstance(node, c_ast.BinaryOp):
                node.left = _hoist(node.left)
                node.right = _hoist(node.right)
            if isinstance(node, c_ast.UnaryOp):
                node.expr = _hoist(node.expr)
            return node
        cleaned = _hoist(cond_node)
        return self.expr(cleaned)

    def _visit_if(self, node):
        """Handle if/else if/else statement."""
        cond = self._hoist_assignments_from_cond(node.cond)
        self.emit(f"if {cond}")
        self.depth += 1
        if node.iftrue:
            if isinstance(node.iftrue, c_ast.Compound):
                self._visit_compound(node.iftrue)
            else:
                self._visit_stmt(node.iftrue)
        self.depth -= 1
        if node.iffalse:
            if isinstance(node.iffalse, c_ast.If):
                # Direct else if — handled via _visit_else chain
                self._visit_else(node.iffalse)
            else:
                # else block — check if it only contains a single if statement
                # to avoid nested 'end if' inside the else body (UL parser bug workaround)
                else_body = node.iffalse
                else_stmts = []
                if isinstance(else_body, c_ast.Compound) and else_body.block_items:
                    else_stmts = else_body.block_items
                elif not isinstance(else_body, c_ast.Compound):
                    else_stmts = [else_body]

                # If the else body is a single If statement, convert to else if
                # (UL parser bug workaround: avoids nested 'end if' inside else)
                if len(else_stmts) == 1 and isinstance(else_stmts[0], c_ast.If):
                    self._visit_else(else_stmts[0])
                else:
                    self.emit("else")
                    self.depth += 1
                    for s in else_stmts:
                        self._visit_stmt(s)
                    self.depth -= 1
        self.emit("end if")

    def _visit_else(self, node):
        """Handle remaining else/else-if chains."""
        if isinstance(node, c_ast.If):
            cond = self._hoist_assignments_from_cond(node.cond)
            self.emit(f"else if {cond}")
            self.depth += 1
            if node.iftrue:
                if isinstance(node.iftrue, c_ast.Compound):
                    self._visit_compound(node.iftrue)
                else:
                    self._visit_stmt(node.iftrue)
            self.depth -= 1
            if node.iffalse:
                self._visit_else(node.iffalse)
        else:
            self.emit("else")
            self.depth += 1
            if isinstance(node, c_ast.Compound):
                self._visit_compound(node)
            else:
                self._visit_stmt(node)
            self.depth -= 1

    def _has_break(self, node):
        """Recursively check if a statement subtree contains a Break node.
        Does NOT descend into nested while/for/switch (their breaks are scoped to them).
        """
        if node is None:
            return False
        if isinstance(node, c_ast.Break):
            return True
        if isinstance(node, (c_ast.While, c_ast.DoWhile, c_ast.For, c_ast.Switch)):
            return False  # breaks inside nested loops are scoped to them
        if isinstance(node, c_ast.If):
            return (self._has_break(node.iftrue) or self._has_break(node.iffalse))
        if isinstance(node, c_ast.Compound):
            return any(self._has_break(s) for s in (node.block_items or []))
        if isinstance(node, c_ast.Label):
            return self._has_break(node.stmt)
        return False

    def _visit_while(self, node):
        """Handle while loop.

        If the loop body contains break statements, emulate them with a
        boolean flag variable (_brk_N) that guards both the loop condition
        and each subsequent statement after the break.
        """
        body_has_break = self._has_break(node.stmt)
        if body_has_break:
            # Generate a unique break-flag name
            brk_var = f"_brk_{getattr(self, '_brk_counter', 0)}"
            self._brk_counter = getattr(self, '_brk_counter', 0) + 1
            if hasattr(self, '_local_vars'):
                self._local_vars.add(brk_var)
            self.emit(f"int {brk_var} = 0")
            cond = self._hoist_assignments_from_cond(node.cond)
            self.emit(f"while ({cond} and not {brk_var})")
            self.depth += 1
            # Push the break flag so nested _visit_stmt knows what to emit for Break
            if not hasattr(self, '_break_flags'):
                self._break_flags = []
            self._break_flags.append(brk_var)
            if node.stmt:
                if isinstance(node.stmt, c_ast.Compound):
                    self._visit_compound(node.stmt)
                else:
                    self._visit_stmt(node.stmt)
            self._break_flags.pop()
            self.depth -= 1
            self.emit("end while")
        else:
            cond = self._hoist_assignments_from_cond(node.cond)
            self.emit(f"while {cond}")
            self.depth += 1
            if node.stmt:
                if isinstance(node.stmt, c_ast.Compound):
                    self._visit_compound(node.stmt)
                else:
                    self._visit_stmt(node.stmt)
            self.depth -= 1
            self.emit("end while")

    def _visit_dowhile(self, node):
        """Handle do-while loop — UL has do...while...end do.
        Special case: do { ... } while(0) is a C macro idiom meaning
        'execute this block once'. Emit just the body, no loop.
        """
        # Detect while(0) — just emit body, no loop wrapper
        if isinstance(node.cond, c_ast.Constant) and node.cond.value == '0':
            if node.stmt:
                if isinstance(node.stmt, c_ast.Compound):
                    self._visit_compound(node.stmt)
                else:
                    self._visit_stmt(node.stmt)
            return
        cond = self.expr(node.cond)
        self.emit("do")
        self.depth += 1
        if node.stmt:
            if isinstance(node.stmt, c_ast.Compound):
                self._visit_compound(node.stmt)
            else:
                self._visit_stmt(node.stmt)
        self.depth -= 1
        self.emit(f"while {cond} end do")

    def _visit_for(self, node):
        """Convert C for loop to UL while loop."""
        body_has_break = self._has_break(node.stmt)
        brk_var = None
        if body_has_break:
            brk_var = f"_brk_{getattr(self, '_brk_counter', 0)}"
            self._brk_counter = getattr(self, '_brk_counter', 0) + 1
            if hasattr(self, '_local_vars'):
                self._local_vars.add(brk_var)
            self.emit(f"int {brk_var} = 0")

        # Emit init
        if node.init:
            if isinstance(node.init, c_ast.DeclList):
                for d in node.init.decls:
                    self._visit_local_decl(d)
            elif isinstance(node.init, c_ast.Assignment):
                self._visit_assignment(node.init)
            elif isinstance(node.init, c_ast.ExprList):
                # Comma-separated init expressions: for (i=0, p=dst; ...)
                # Emit each as a separate statement.
                for sub in node.init.exprs:
                    if isinstance(sub, c_ast.Assignment):
                        self._visit_assignment(sub)
                    elif isinstance(sub, c_ast.UnaryOp):
                        self._visit_unary_stmt(sub)
                    else:
                        e = self.expr(sub)
                        if e:
                            self.emit(e)
            else:
                e = self.expr(node.init)
                if e:
                    self.emit(e)

        # while (cond)
        cond = self._hoist_assignments_from_cond(node.cond) if node.cond else "true"
        if brk_var:
            self.emit(f"while ({cond} and not {brk_var})")
        else:
            self.emit(f"while {cond}")
        self.depth += 1

        # Body
        if brk_var:
            if not hasattr(self, '_break_flags'):
                self._break_flags = []
            self._break_flags.append(brk_var)
        if node.stmt:
            if isinstance(node.stmt, c_ast.Compound):
                self._visit_compound(node.stmt)
            else:
                self._visit_stmt(node.stmt)
        if brk_var:
            self._break_flags.pop()

        # Next (increment) — emit only if not breaking
        if node.next:
            if brk_var:
                self.emit(f"if not {brk_var}")
                self.depth += 1
            if isinstance(node.next, c_ast.UnaryOp):
                self._visit_unary_stmt(node.next)
            elif isinstance(node.next, c_ast.Assignment):
                self._visit_assignment(node.next)
            elif isinstance(node.next, c_ast.ExprList):
                for e in node.next.exprs:
                    if isinstance(e, c_ast.UnaryOp):
                        self._visit_unary_stmt(e)
                    elif isinstance(e, c_ast.Assignment):
                        self._visit_assignment(e)
                    else:
                        self.emit(self.expr(e))
            else:
                e = self.expr(node.next)
                if e:
                    self.emit(e)
            if brk_var:
                self.depth -= 1
                self.emit("end if")

        self.depth -= 1
        self.emit("end while")

    def _visit_return(self, node):
        """Handle return statement."""
        if node.expr:
            # In C, return can contain an assignment expression: return (x = val)
            # UL doesn't support assignment-as-expression, so split it
            if isinstance(node.expr, c_ast.Assignment):
                self._visit_assignment(node.expr)
                lhs = self.expr(node.expr.lvalue)
                self.emit(f"return {lhs}")
            else:
                val = self.expr(node.expr)
                self.emit(f"return {val}")
        else:
            self.emit("return")

    def _visit_func_call_stmt(self, node):
        """Handle function call as a statement."""
        # Get raw function name first so explicit stdlib handlers can match it.
        # _safe_name is applied only at the fallthrough emit.
        if isinstance(node.name, c_ast.ID):
            func_name = node.name.name
        else:
            func_name = self.expr(node.name)
        args = []
        if node.args:
            for a in node.args.exprs:
                args.append(self.expr(a))

        if func_name == "printf":
            print_expr = self._convert_printf(args)
            # Check if it ends with \n — if so, use print (adds newline)
            # If not, need to handle differently
            if print_expr.endswith('"\\n"'):
                # Strip trailing + "\\n" and use print
                print_expr = print_expr[:-len(' + "\\n"')]
                if not print_expr:
                    print_expr = '""'
                self.emit(f"print {print_expr}")
            elif print_expr.endswith('\\n"'):
                # The \n is embedded in the last string part
                # Strip it from the end of the last string
                if print_expr.endswith('\\n"'):
                    # Remove \n from end of string literal
                    print_expr = print_expr[:-3] + '"'
                    if print_expr == '""':
                        self.emit('print ""')
                    else:
                        self.emit(f"print {print_expr}")
                else:
                    self.emit(f"print {print_expr}")
            else:
                # No newline — use print anyway (best effort)
                self.emit(f"print {print_expr}")
        elif func_name == "puts":
            if args:
                self.emit(f"print {args[0]}")
            else:
                self.emit('print ""')
        elif func_name in ("free", "s_free", "malloc", "calloc", "realloc",
                           "s_malloc", "s_realloc", "s_calloc"):
            self.emit(f"// {func_name}({', '.join(args)}) - UL handles memory")
        elif func_name in ("assert", "_assert", "__assert", "__assert_fail",
                           "__assert_func", "__assert_rtn"):
            self.emit(f"// assert({', '.join(args)}) - skipped")
        elif func_name == "srand":
            self.emit(f"random_seed({args[0]})" if args else "random_seed(0)")
        elif func_name == "rand":
            self.emit(f"random_int(0, 32767)")
        else:
            # Map C stdlib names to UL equivalents
            c_to_ul_funcs = {"memcpy": "memcopy", "bcopy": "memcopy"}
            if func_name in c_to_ul_funcs:
                func_name = c_to_ul_funcs[func_name]
            else:
                func_name = _safe_name(func_name)
            self.emit(f"{func_name}({', '.join(args)})")

    def _visit_unary_stmt(self, node):
        """Handle i++, i--, ++i, --i as statements."""
        operand = self.expr(node.expr)
        if node.op in ("p++", "++"):
            self.emit(f"{operand} = {operand} + 1")
        elif node.op in ("p--", "--"):
            self.emit(f"{operand} = {operand} - 1")
        else:
            e = self.expr(node)
            if e:
                self.emit(e)

    def _visit_switch(self, node):
        """Convert switch/case to if/else if chain.
        Fall-through cases (empty case bodies) are combined with OR into the next case.
        """
        switch_var = self.expr(node.cond)
        if not (node.stmt and isinstance(node.stmt, c_ast.Compound) and node.stmt.block_items):
            return

        # Pre-process: group consecutive cases that fall-through together
        # Each group is a list of (case_values, stmts) where case_values is a list
        items = node.stmt.block_items
        groups = []  # list of (is_default, case_vals_list, stmts_list)
        current_vals = []
        current_stmts = []
        is_default = False

        for item in items:
            if isinstance(item, c_ast.Case):
                all_stmts = item.stmts or []
                real_stmts = [s for s in all_stmts if not isinstance(s, c_ast.Break)]
                has_explicit_break = any(isinstance(s, c_ast.Break) for s in all_stmts)
                # True fall-through: no statements at all (not even break)
                true_fallthrough = (len(all_stmts) == 0) or (
                    not real_stmts and not has_explicit_break
                )
                if real_stmts:
                    current_vals.append(self.expr(item.expr))
                    current_stmts = real_stmts
                    groups.append((False, list(current_vals), current_stmts))
                    current_vals = []
                    current_stmts = []
                    is_default = False
                elif true_fallthrough:
                    # True fall-through (no stmts, no break): combine with next case
                    current_vals.append(self.expr(item.expr))
                else:
                    # case N: break; — explicit break with no body.
                    # Emit as an empty group so it doesn't fall-through to next case.
                    val = self.expr(item.expr)
                    groups.append((False, list(current_vals) + [val], []))
                    current_vals = []
                    current_stmts = []
                    is_default = False
            elif isinstance(item, c_ast.Default):
                real_stmts = [s for s in (item.stmts or []) if not isinstance(s, c_ast.Break)]
                groups.append((True, [], real_stmts))
                current_vals = []
                current_stmts = []
                is_default = False

        # Handle any remaining fall-through with no body
        if current_vals:
            # Fall-through with no matching default — just skip
            pass

        if not groups:
            return

        first_case = True
        for (is_def, case_vals, stmts) in groups:
            if is_def:
                if first_case:
                    # No if/else-if emitted yet — default body runs unconditionally
                    for s in stmts:
                        self._visit_stmt(s)
                    continue
                # Check if default body is a single if statement — use else if to avoid
                # nested 'end if' inside else body (UL parser limitation)
                if len(stmts) == 1 and isinstance(stmts[0], c_ast.If):
                    self._visit_else(stmts[0])
                    continue
                self.emit("else")
            else:
                # Build condition: switch_var == v1 or switch_var == v2 or ...
                conds = [f"{switch_var} == {v}" for v in case_vals]
                cond_str = " or ".join(conds)
                if first_case:
                    self.emit(f"if {cond_str}")
                    first_case = False
                else:
                    self.emit(f"else if {cond_str}")
            self.depth += 1
            if stmts:
                for s in stmts:
                    self._visit_stmt(s)
            # else: empty body (case N: break;) — no statements to emit; UL if block can be empty
            self.depth -= 1
        if not first_case:
            self.emit("end if")

    # -- Entry point ----------------------------------------------------------

    def translate(self, ast):
        self.visit_FileAST(ast)
        return "\n".join(self.lines)


def strip_block_comments(source):
    """Remove /* ... */ block comments from C source."""
    result = []
    i = 0
    while i < len(source):
        if source[i:i+2] == '/*':
            # Find closing */
            end = source.find('*/', i + 2)
            if end == -1:
                break  # unterminated comment
            # Preserve newlines inside block comment so line numbers stay
            comment_text = source[i:end+2]
            result.append('\n' * comment_text.count('\n'))
            i = end + 2
        elif source[i:i+2] == '//':
            # Line comment — skip to end of line
            end = source.find('\n', i)
            if end == -1:
                break
            result.append('\n')
            i = end
        else:
            result.append(source[i])
            i += 1
    return ''.join(result)


def _gcc_preprocess(source_path, extra_defines=None):
    """Run gcc -E and filter output to only lines from the original source files.

    Uses line-marker directives to track which file each line comes from, then
    only keeps lines from the source .c file and any local .h files (not system headers).
    Strips GCC-specific extensions that pycparser cannot handle.
    Returns cleaned preprocessed source, or None on failure.
    """
    gcc_candidates = ["gcc", "/c/Users/azt12/OneDrive/Documents/Computing/All Projects/LANG/UniLogic/Web-Content/videos/bin/gcc", "cc"]
    gcc = None
    for g in gcc_candidates:
        try:
            r = subprocess.run([g, "--version"], capture_output=True, timeout=5)
            if r.returncode == 0:
                gcc = g
                break
        except (FileNotFoundError, subprocess.TimeoutExpired):
            continue
    if gcc is None:
        return None

    src_dir = os.path.dirname(os.path.abspath(source_path))
    src_base = os.path.basename(source_path)
    # Derive the base name without extension for detecting related local headers
    src_stem = os.path.splitext(src_base)[0]

    cmd = [gcc, "-E",   # no -P so we get # line "file" markers
           "-w",         # suppress warnings
           "-x", "c",
           f"-I{src_dir}",
           source_path]

    if extra_defines:
        for d in extra_defines:
            cmd += ["-D", d]

    try:
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=60)
    except (subprocess.TimeoutExpired, OSError):
        return None

    if not result.stdout.strip():
        return None

    # Filter: only keep lines that belong to the .c file or any local .h/.c file
    # from the SAME SOURCE DIRECTORY (not system headers with matching filenames).
    # We match by full normalised path prefix so system headers like malloc.h
    # (from MinGW) are excluded even when a project file shares the same name.
    src_dir_norm = src_dir.replace('\\', '/').rstrip('/').lower()

    def _is_local_path(fpath_raw):
        """Return True if the gcc line-marker path belongs to our source dir."""
        fp = fpath_raw.replace('\\', '/').replace('//', '/').lower()
        # The path must be under (or equal to) our source directory.
        # Primary check: absolute path prefix match.
        if fp.startswith(src_dir_norm + '/') or fp.startswith(src_dir_norm.rstrip('/') + '/'):
            return True
        # Secondary check: relative path — resolve it against cwd and then check.
        # gcc -E may emit relative paths when the input is given as a relative path.
        try:
            abs_fp = os.path.normpath(os.path.abspath(fpath_raw)).replace('\\', '/').lower()
            return abs_fp.startswith(src_dir_norm + '/') or abs_fp == src_dir_norm or abs_fp.startswith(src_dir_norm.rstrip('/') + '/')
        except Exception:
            return False

    in_local = False
    filtered = []
    for line in result.stdout.splitlines(keepends=True):
        m = re.match(r'^# \d+ "(.+?)"', line)
        if m:
            fpath = m.group(1)
            in_local = _is_local_path(fpath)
            continue
        if in_local:
            stripped = line.strip()
            if stripped.startswith('#pragma'):
                filtered.append('\n')
                continue
            filtered.append(line)

    if not filtered:
        return None

    source = ''.join(filtered)

    # Strip GCC extensions that pycparser cannot handle
    # __attribute__((...)) — use balanced-paren stripper to handle nested parens correctly.
    # The simple regex leaves dangling ')' because __attribute__((x(y))) has 3 close parens.
    def _strip_attribute(src):
        result = []
        i = 0
        while i < len(src):
            if src[i:i+13] == '__attribute__':
                before = src[i-1] if i > 0 else ' '
                if before.isalnum() or before == '_':
                    result.append(src[i])
                    i += 1
                    continue
                j = i + 13
                while j < len(src) and src[j] in ' \t':
                    j += 1
                if j < len(src) and src[j] == '(':
                    depth = 1
                    j += 1
                    while j < len(src) and depth > 0:
                        if src[j] == '(':
                            depth += 1
                        elif src[j] == ')':
                            depth -= 1
                        j += 1
                    # Preserve newlines for line number stability
                    result.append('\n' * src[i:j].count('\n'))
                    i = j
                    continue
            result.append(src[i])
            i += 1
        return ''.join(result)

    source = _strip_attribute(source)
    # __extension__
    source = re.sub(r'\b__extension__\b', '', source)
    # __inline__ / __inline / __forceinline
    source = re.sub(r'\b(__inline__|__inline|__forceinline)\b', 'inline', source)
    # __cdecl / __stdcall / __fastcall
    source = re.sub(r'\b(__cdecl|__stdcall|__fastcall|__thiscall)\b', '', source)
    # __restrict / __restrict__
    source = re.sub(r'\b(__restrict__|__restrict)\b', '', source)
    # Remove __declspec(...)
    source = re.sub(r'__declspec\s*\([^)]*\)', '', source)
    # __volatile__ -> volatile
    source = re.sub(r'\b__volatile__\b', 'volatile', source)
    # __const__ -> const
    source = re.sub(r'\b__const__\b', 'const', source)
    # __signed__ -> signed
    source = re.sub(r'\b__signed__\b', 'signed', source)
    # Replace calls to GCC builtins and intrinsics that pycparser can't handle.
    # We need balanced-paren replacement for multi-line, nested calls.
    def _replace_balanced_call(source, pattern, replacement='0'):
        """Find 'pattern(' and replace the entire balanced call with replacement."""
        result = []
        i = 0
        pat_bytes = pattern
        while i < len(source):
            # Check if pattern starts here
            if source[i:i+len(pat_bytes)] == pat_bytes and (i == 0 or not source[i-1].isalnum() and source[i-1] != '_'):
                # Find the opening paren
                j = i + len(pat_bytes)
                while j < len(source) and source[j].isspace():
                    j += 1
                if j < len(source) and source[j] == '(':
                    # Find matching close paren
                    depth = 1
                    j += 1
                    while j < len(source) and depth > 0:
                        if source[j] == '(':
                            depth += 1
                        elif source[j] == ')':
                            depth -= 1
                        j += 1
                    # j is now right after the closing paren
                    result.append(replacement)
                    i = j
                    continue
            result.append(source[i])
            i += 1
        return ''.join(result)

    # Replace GCC builtins that produce invalid pycparser input
    for builtin_name in ['__builtin_choose_expr', '__builtin_types_compatible_p',
                          '__builtin_expect', '__builtin_constant_p',
                          '__builtin_unreachable', '__builtin_trap',
                          '__builtin_va_start', '__builtin_va_end', '__builtin_va_arg',
                          '__builtin_offsetof', '__builtin_classify_type']:
        source = _replace_balanced_call(source, builtin_name, '0')

    # assert() macros expand to complex GCC expressions; replace entire expanded assert
    # GCC expands assert(x) into: ((void)(__builtin_expect(!(x), 0) ? __assert_fail(...) : 0))
    # or similar. Replace _assert/__assert_fail/__assert_rtn at the preprocess level.
    for assert_fn in ['__assert_fail', '__assert_rtn', '__assert_func', '_assert']:
        source = _replace_balanced_call(source, assert_fn, '0')

    # __typeof__(x), __typeof(x)
    source = _replace_balanced_call(source, '__typeof__', 'int')
    source = _replace_balanced_call(source, '__typeof', 'int')

    # Remaining __builtin_* calls with parens
    source = re.sub(r'\b__builtin_\w+\s*\([^)]*(?:\([^)]*\)[^)]*)*\)', '0', source)
    # Any remaining __builtin_ identifiers
    source = re.sub(r'\b__builtin_\w+\b', '0', source)
    # __isnan, __fpclassify, __isinf etc — math builtins (after __typeof cleanup)
    for math_fn in ['__isnanl', '__isnanf', '__isnan', '__isinfl', '__isinff', '__isinf',
                    '__fpclassifyl', '__fpclassifyf', '__fpclassify',
                    '__isfinitel', '__isfinitef', '__isfinite',
                    '__signbitl', '__signbitf', '__signbit']:
        source = _replace_balanced_call(source, math_fn, '0')
    # __uint128_t / __int128_t — GCC extension, not supported by pycparser.
    # Map to unsigned long long / long long (closest 64-bit approximation).
    source = re.sub(r'\b__uint128_t\b', 'unsigned long long', source)
    source = re.sub(r'\b__int128_t\b', 'long long', source)
    source = re.sub(r'\b__int128\b', 'long long', source)
    source = re.sub(r'\bunsigned __int128\b', 'unsigned long long', source)
    # _Noreturn, __noreturn__
    source = re.sub(r'\b(_Noreturn|__noreturn__)\b', '', source)
    # __PRETTY_FUNCTION__, __FUNCTION__, __func__
    source = re.sub(r'\b(__PRETTY_FUNCTION__|__FUNCTION__)\b', '"func"', source)
    # Remove __asm__ / __asm blocks (they can span lines — strip carefully)
    source = re.sub(r'__asm__\s*(?:__volatile__\s*)?\([^;]*\)', '', source, flags=re.DOTALL)
    source = re.sub(r'__asm\s*(?:volatile\s*)?\([^;]*\)', '', source, flags=re.DOTALL)
    # Strip MinGW/glibc assert expansions: (void)((!!expr) || (fallback,expr))
    # These contain C comma expressions which pycparser rejects in statement position.
    # Pattern: (void) ( (!! (...)) || (0,0) )  -- multi-line, with balanced parens.
    # Only replace (0,0) when it appears as the RHS of a || or && (assert-expansion context),
    # NOT as a bare function-call argument list like f(0, 0).
    source = re.sub(r'(?<=[|][|]|[&][&])\s*\(\s*0\s*,\s*0\s*\)', ' 0', source)
    # Remove empty lines caused by above stripping (collapse multiple blanks)
    source = re.sub(r'\n{3,}', '\n\n', source)

    return source


def preprocess_c_source(source, source_path=None):
    """Preprocess C source.

    Strategy:
    1. Try gcc -E for full macro expansion (best results).
    2. Fall back to minimal manual preprocessing.
    """
    # -- Strategy 1: gcc -E --------------------------------------------------
    if source_path and os.path.isfile(source_path):
        preprocessed = _gcc_preprocess(source_path)
        if preprocessed:
            # gcc -E already expanded system headers and local includes.
            # Prepend fake libc types so pycparser recognizes size_t, etc.
            fake_decls = _fake_decls_pycparser()
            return fake_decls + "\n" + preprocessed

    # -- Strategy 2: minimal manual preprocessing ----------------------------
    source = strip_block_comments(source)
    lines = source.split('\n')
    out = []
    for line in lines:
        stripped = line.strip()
        if stripped.startswith('#include'):
            continue  # skip includes
        if stripped.startswith('#define'):
            continue
        if stripped.startswith('#'):
            continue  # skip all preprocessor directives
        out.append(line)

    fake_decls = _fake_decls_pycparser()
    return fake_decls + "\n" + "\n".join(out)


def _fake_decls_pycparser():
    """Return fake type/function declarations so pycparser can parse standard C."""
    return "\n".join([
        "int printf(const char *fmt, ...);",
        "int puts(const char *s);",
        "int scanf(const char *fmt, ...);",
        "int strlen(const char *s);",
        "int strcmp(const char *a, const char *b);",
        "char *strcpy(char *dst, const char *src);",
        "char *strncpy(char *dst, const char *src, int n);",
        "char *strcat(char *dst, const char *src);",
        "char *strncat(char *dst, const char *src, int n);",
        "char *strstr(const char *hay, const char *needle);",
        "char *strchr(const char *s, int c);",
        "int strncmp(const char *a, const char *b, int n);",
        "int atoi(const char *s);",
        "double atof(const char *s);",
        "int abs(int x);",
        "void *malloc(int size);",
        "void *calloc(int n, int size);",
        "void *realloc(void *p, int size);",
        "void free(void *p);",
        "void exit(int code);",
        "int fprintf(int f, const char *fmt, ...);",
        "int sprintf(char *s, const char *fmt, ...);",
        "int snprintf(char *s, int n, const char *fmt, ...);",
        "void *memcpy(void *dst, const void *src, int n);",
        "void *memmove(void *dst, const void *src, int n);",
        "void *memset(void *dst, int c, int n);",
        "int memcmp(const void *a, const void *b, int n);",
        "typedef unsigned char uint8_t;",
        "typedef unsigned short uint16_t;",
        "typedef unsigned int uint32_t;",
        "typedef unsigned long uint64_t;",
        "typedef signed char int8_t;",
        "typedef short int16_t;",
        "typedef int int32_t;",
        "typedef long int64_t;",
        "typedef unsigned int uintptr_t;",
        "typedef int intptr_t;",
        "typedef int ptrdiff_t;",
        "typedef int bool;",
        "typedef int size_t;",
        "typedef int ssize_t;",
        "typedef unsigned int off_t;",
        "typedef unsigned int mode_t;",
        "typedef int FILE;",
        "typedef int va_list;",
    ])


def translate_file(input_path, output_path=None):
    """Translate a C source file to UL."""
    with open(input_path, 'r') as f:
        source = f.read()

    # Preprocess
    preprocessed = preprocess_c_source(source, source_path=input_path)

    # Parse
    parser = c_parser.CParser()
    try:
        ast = parser.parse(preprocessed, filename=input_path)
    except Exception as e:
        print(f"Parse error: {e}", file=sys.stderr)
        return None

    # Translate
    translator = C2UL()
    # Add comment header
    base = os.path.basename(input_path)
    translator.emit(f"// Translated from {base} by c2ul")

    translator.visit_FileAST(ast)

    # Output
    if output_path is None:
        output_path = os.path.splitext(input_path)[0] + ".ul"

    result = "\n".join(translator.lines) + "\n"

    with open(output_path, 'w') as f:
        f.write(result)

    # Report
    print(f"Translated: {translator.translated_lines} lines")
    print(f"Warnings: {len(translator.warnings)}")
    if translator.warnings:
        for lineno, reason in translator.warnings:
            print(f"  line {lineno}: {reason}")
    print(f"Output: {output_path}")

    return output_path


def main():
    if len(sys.argv) < 2:
        print("Usage: python c2ul.py input.c [-o output.ul]")
        sys.exit(1)

    input_path = sys.argv[1]
    output_path = None
    if "-o" in sys.argv:
        idx = sys.argv.index("-o")
        if idx + 1 < len(sys.argv):
            output_path = sys.argv[idx + 1]

    result = translate_file(input_path, output_path)
    if result is None:
        sys.exit(1)


if __name__ == "__main__":
    main()
