#!/usr/bin/env python3
"""
UniLogic Language Server — LSP for .ul files
Provides: semantic-token highlighting, diagnostics, hover (keyword glossary)
Launch: python lsp.py          (stdio transport — VS Code default)
        python lsp.py --tcp    (TCP on 127.0.0.1:2087 for debugging)
"""

import sys, os, re, logging, glob as globmod

# ── Add compiler source dirs to path so we can import lexer/parser/semcheck ──
_base = os.path.dirname(os.path.abspath(__file__))

# Bundled layout (vsix): compiler/ lives next to lsp.py
_bundled = os.path.join(_base, "compiler")
if os.path.isdir(_bundled):
    sys.path.insert(0, _bundled)
else:
    # Dev layout: lsp.py is in LSP/, compiler modules in sibling dirs
    _root = os.path.dirname(_base)
    for sub in ("Lexer", "Parser", "AST", "Semantic"):
        sys.path.insert(0, os.path.join(_root, sub))

from lexer    import tokenize, KEYWORDS
from parser   import parse
from semcheck import check, SemanticError
from ast_nodes import (FunctionDecl, ForeignImport, TypeDecl, VarDecl, Param,
                       If, While, For, Match, ExprStmt, Call,
                       DrDirective, NormDirective, Program)

from pygls.lsp.server import LanguageServer
from lsprotocol import types as lsp

import sys as _sys
_sys.stderr.write(f"[LSP] loaded from: {__file__}\n")

logging.basicConfig(filename=os.path.join(_base, "lsp.log"), level=logging.DEBUG)
log = logging.getLogger("ul-lsp")

server = LanguageServer("ul-language-server", "v0.1.0")

# ═══════════════════════════════════════════════════════════════════════════════
#  KEYWORD GLOSSARY — one-line definitions for every keyword in KEYWORDS
# ═══════════════════════════════════════════════════════════════════════════════

GLOSSARY = {
    # control flow
    "function":   "Declares a named function.",
    "end":        "Closes a block (function, if, while, for).",
    "returns":    "Specifies the return type of a function.",
    "return":     "Exits a function and optionally passes back a value.",
    "if":         "Begins a conditional branch.",
    "else":       "Alternate branch when the 'if' condition is false.",
    "while":      "Loops while a condition is true.",
    "do":         "Introduces a do-while loop body (reserved).",
    "for":        "Begins a for-each loop.",
    "each":       "Used with 'for' — 'for each var in iterable'.",
    "in":         "Specifies the iterable in a for-each loop.",
    "match":      "Pattern-matching construct (reserved).",
    "iterate":    "Explicit iterator loop (reserved).",
    "default":    "Default branch in a match block (reserved).",
    "escape":     "Breaks out of the nearest loop (like C 'break').",
    "continue":   "Skips to the next iteration of the nearest loop.",

    # declarations / modifiers
    "type":       "Declares a user-defined type / struct (reserved).",
    "inherits":   "Type inheritance (reserved).",
    "new":        "Instantiates a type (reserved).",
    "fixed":      "Marks a variable as constant (immutable after init).",
    "constant":   "Alias for 'fixed' — immutable binding (reserved).",
    "import":     "Imports definitions from another module (reserved).",
    "from":       "Specifies the source module for an import (reserved).",
    "export":     "Makes a declaration visible to other modules (reserved).",
    "print":      "Prints a value to standard output.",
    "prompt":     "Reads user input from stdin (reserved).",

    # concurrency / advanced
    "parallel":   "Marks a block for parallel execution (reserved).",
    "killswitch": "Aborts a parallel task (reserved).",
    "teleport":   "Transfers execution to a portal label (reserved).",
    "portal":     "Declares a teleport destination label (reserved).",

    # performance hints
    "nocache":    "Hints that a value should bypass CPU cache (reserved).",
    "yield":      "Yields control in a coroutine / generator (reserved).",
    "inline":     "Hints the compiler to inline a function (reserved).",
    "pack":       "Requests packed (no-padding) struct layout (reserved).",

    # literals / logic
    "true":       "Boolean literal — logical true.",
    "false":      "Boolean literal — logical false.",
    "empty":      "Null / absent value (like None or NULL).",
    "and":        "Logical AND operator.",
    "or":         "Logical OR operator.",
    "not":        "Logical NOT (unary negation).",
    "equals":     "Equality comparison (reserved — use '==' instead).",

    # bitwise operators
    "both1":      "Bitwise AND of two values.",
    "both0":      "Bitwise NOR of two values.",
    "either1":    "Bitwise OR of two values.",
    "delta":      "Bitwise XOR of two values.",
    "bitflip":    "Bitwise NOT (one's complement).",
    "negate":     "Arithmetic negation (two's complement).",
    "left":       "Bitwise left-shift (also 'left=' for compound).",
    "right":      "Bitwise right-shift (also 'right=' for compound).",

    # memory / pointers
    "address":    "Takes the memory address of a variable (like C '&').",
    "deref":      "Dereferences a pointer (like C '*ptr').",
    "memmove":    "Moves a block of memory (may overlap). Builtin.",
    "memcopy":    "Copies a block of memory (no overlap). Builtin.",
    "memset":     "Fills a block of memory with a byte value. Builtin.",
    "memtake":    "Allocates heap memory (like C 'malloc'). Builtin.",
    "memgive":    "Frees heap memory (like C 'free'). Builtin.",

    # builtins
    "size":       "Returns the size of a type or value in bytes. Builtin.",
    "cast":       "Converts a value to a different type. Usage: cast(value, type) — e.g. cast(x, float).",
    "absval":     "Returns the absolute value. Builtin.",
    "exit":       "Exits the program immediately with the given exit code. Usage: exit(0) for success, exit(1) for error.",

    # scalar types
    "int":        "32-bit signed integer type.",
    "integer":    "Alias for 'int'.",
    "float":      "32-bit IEEE floating-point type.",
    "double":     "64-bit IEEE floating-point type.",
    "string":     "Character string type (char* in C).",
    "bool":       "Boolean type (true / false).",
    "none":       "Void / no-value type.",
    "complex":    "Complex number type (reserved).",

    # sized integer types
    "int8":       "8-bit signed integer.",
    "int16":      "16-bit signed integer.",
    "int32":      "32-bit signed integer.",
    "int64":      "64-bit signed integer.",
    "uint8":      "8-bit unsigned integer.",
    "uint16":     "16-bit unsigned integer.",
    "uint32":     "32-bit unsigned integer.",
    "uint64":     "64-bit unsigned integer.",

    # collection types
    "array":      "Fixed-size array type — 'array int nums = [1,2,3]'.",
    "list":       "Dynamic-size list type (reserved).",
    "map":        "Key-value mapping type (reserved).",
    "arena":      "Arena memory allocator type (reserved).",
    "file":       "File handle type (reserved).",

    # result types
    "ok":         "Success variant of a Result type (reserved).",
    "error":      "Error variant of a Result type (reserved).",
    "some":       "Present variant of an Optional type (reserved).",
}


# ═══════════════════════════════════════════════════════════════════════════════
#  SEMANTIC TOKENS — syntax highlighting via LSP semantic tokens protocol
# ═══════════════════════════════════════════════════════════════════════════════

# Token types we report — order matters (index into this list = token type id)
TOKEN_TYPES = [
    "keyword",      # 0
    "function",     # 1
    "variable",     # 2
    "number",       # 3
    "string",       # 4
    "operator",     # 5
    "comment",      # 6
    "type",         # 7
    "parameter",    # 8
    "macro",        # 9  (for @ modifiers and # macros)
]

TOKEN_MODIFIERS = [
    "declaration",    # 0
    "readonly",       # 1
    "defaultLibrary", # 2
]

# Which keywords are "type" tokens
TYPE_KW = {
    "int", "integer", "float", "double", "string", "bool", "none", "complex",
    "int8", "int16", "int32", "int64",
    "uint8", "uint16", "uint32", "uint64",
    "array", "list", "map", "arena", "file",
}

LEGEND = lsp.SemanticTokensLegend(
    token_types=TOKEN_TYPES,
    token_modifiers=TOKEN_MODIFIERS,
)


# Modifier bit constants (index in TOKEN_MODIFIERS → bitmask)
MOD_DECLARATION    = 1 << 0   # "declaration"    index 0
MOD_READONLY       = 1 << 1   # "readonly"       index 1
MOD_DEFAULT_LIB    = 1 << 2   # "defaultLibrary" index 2

# Builtin function keywords that get the defaultLibrary modifier
BUILTIN_FUNCS = {
    "absval", "size", "memmove", "memcopy", "memset", "memtake", "memgive", "cast",
}


def _build_modifier_tables(source, filename):
    """Parse source into AST and return lookup tables for semantic token modifiers.
    Returns (decl_positions, fixed_vars, func_names, type_names) or Nones on failure.
      decl_positions: set of (0-based line, 0-based col) for declaration-site names
      fixed_vars:     set of variable names declared with 'fixed'
      func_names:     set of user-defined function names (for token type upgrade)
      type_names:     set of user-defined type names
    """
    program = _try_parse(source, filename)
    if not program:
        return set(), set(), set(), set()

    decl_positions = set()
    fixed_vars = set()
    func_names = set()
    type_names = set()

    for decl in program.decls:
        if isinstance(decl, FunctionDecl):
            # The function name token is at the declaration site
            decl_positions.add((decl.line - 1, decl.col - 1))
            func_names.add(decl.name)
            # Collect fixed vars from body
            _collect_fixed_vars(decl.body, fixed_vars)
        elif isinstance(decl, TypeDecl):
            decl_positions.add((decl.line - 1, decl.col - 1))
            type_names.add(decl.name)
        elif isinstance(decl, ForeignImport):
            decl_positions.add((decl.line - 1, decl.col - 1))
            func_names.add(decl.name)

    return decl_positions, fixed_vars, func_names, type_names


def _collect_fixed_vars(stmts, fixed_vars):
    """Recursively collect names of variables declared with 'fixed'."""
    for s in stmts:
        if isinstance(s, VarDecl) and s.fixed:
            fixed_vars.add(s.name)
        elif isinstance(s, If):
            _collect_fixed_vars(s.then_body, fixed_vars)
            _collect_fixed_vars(s.else_body, fixed_vars)
        elif isinstance(s, While):
            _collect_fixed_vars(s.body, fixed_vars)
        elif isinstance(s, For):
            _collect_fixed_vars(s.body, fixed_vars)
        elif isinstance(s, Match):
            for case in s.cases:
                _collect_fixed_vars(case.body, fixed_vars)


def _build_semantic_tokens(source: str, filename: str):
    """Tokenize source and produce the LSP integer-encoded semantic tokens data."""
    try:
        tokens = tokenize(source, filename)
    except SyntaxError:
        return []

    # Build modifier lookup tables from AST
    decl_positions, fixed_vars, func_names, type_names = _build_modifier_tables(source, filename)

    # We also need comment positions — the lexer skips them, so do a quick scan
    comment_spans = []
    for m in re.finditer(r"//[^\n]*", source):
        start = m.start()
        # compute line/col
        line = source[:start].count("\n")
        col = start - source.rfind("\n", 0, start) - 1
        length = m.end() - m.start()
        comment_spans.append((line, col, length))

    # Build raw list of (line, col, length, type_index, modifier_bits)
    raw = []

    for cline, ccol, clen in comment_spans:
        raw.append((cline, ccol, clen, 6, 0))  # 6 = comment

    for tok in tokens:
        kind, value, tline, tcol = tok
        # LSP is 0-based; our lexer is 1-based
        line = tline - 1
        col  = tcol - 1
        length = len(value)

        if kind == "EOF":
            continue

        if kind == "KW":
            if value in TYPE_KW:
                raw.append((line, col, length, 7, 0))    # type
            elif value in BUILTIN_FUNCS:
                raw.append((line, col, length, 5, MOD_DEFAULT_LIB))  # operator + defaultLibrary
            elif value in ("function", "end", "returns", "return",
                           "if", "else", "while", "for", "each", "in",
                           "escape", "continue", "match", "iterate",
                           "default", "do", "fixed", "constant",
                           "import", "from", "export",
                           "parallel", "killswitch", "teleport", "portal",
                           "nocache", "yield", "inline", "pack",
                           "print", "prompt"):
                raw.append((line, col, length, 0, 0))    # keyword
            elif value in ("and", "or", "not", "equals",
                           "both1", "both0", "either1", "delta",
                           "bitflip", "negate", "left", "right",
                           "address", "deref"):
                raw.append((line, col, length, 5, 0))    # operator
            elif value in ("true", "false", "empty"):
                raw.append((line, col, length, 3, 0))    # number (literal)
            elif value in ("ok", "error", "some", "new", "type", "inherits"):
                raw.append((line, col, length, 0, 0))    # keyword
            else:
                raw.append((line, col, length, 0, 0))    # keyword fallback

        elif kind == "IDENT":
            # Determine token type and modifiers from AST
            mods = 0
            if (line, col) in decl_positions:
                mods |= MOD_DECLARATION
            if value in fixed_vars:
                mods |= MOD_READONLY

            # Upgrade token type based on what the identifier refers to
            if value in func_names:
                raw.append((line, col, length, 1, mods))     # function
            elif value in type_names:
                raw.append((line, col, length, 7, mods))     # type
            else:
                raw.append((line, col, length, 2, mods))     # variable

        elif kind in ("INT", "FLOAT", "HEX"):
            raw.append((line, col, length, 3, 0))        # number

        elif kind == "STRING":
            # +2 for the quotes that the lexer stripped
            raw.append((line, col - 1, length + 2, 4, 0))  # string

        elif kind == "OP":
            raw.append((line, col, length, 5, 0))        # operator

        elif kind == "AT":
            raw.append((line, col, length, 9, 0))        # macro

        elif kind == "HASH":
            raw.append((line, col, length, 9, 0))        # macro

    # Sort by (line, col) then delta-encode
    raw.sort(key=lambda r: (r[0], r[1]))

    data = []
    prev_line = 0
    prev_col  = 0
    for line, col, length, type_idx, mod_bits in raw:
        delta_line = line - prev_line
        delta_col  = col - prev_col if delta_line == 0 else col
        data.extend([delta_line, delta_col, length, type_idx, mod_bits])
        prev_line = line
        prev_col  = col

    return data


# ═══════════════════════════════════════════════════════════════════════════════
#  DIAGNOSTICS — run lexer → parser → semcheck and report errors
# ═══════════════════════════════════════════════════════════════════════════════

def _diagnose(source: str, filename: str):
    """Run the full compiler front-end on source and return a list of lsp.Diagnostic."""
    diagnostics = []

    # ── Lex ──
    try:
        tokens = tokenize(source, filename)
    except SyntaxError as e:
        line, col = _parse_error_pos(str(e))
        diagnostics.append(lsp.Diagnostic(
            range=lsp.Range(
                start=lsp.Position(line=line, character=col),
                end=lsp.Position(line=line, character=col + 1),
            ),
            message=str(e),
            severity=lsp.DiagnosticSeverity.Error,
            source="ul-lexer",
        ))
        return diagnostics

    # ── Parse ──
    try:
        program = parse(tokens, filename)
    except SyntaxError as e:
        line, col = _parse_error_pos(str(e))
        diagnostics.append(lsp.Diagnostic(
            range=lsp.Range(
                start=lsp.Position(line=line, character=col),
                end=lsp.Position(line=line, character=col + 10),
            ),
            message=str(e),
            severity=lsp.DiagnosticSeverity.Error,
            source="ul-parser",
        ))
        return diagnostics

    # ── Semantic check ──
    try:
        check(program, filename)
    except SemanticError as e:
        line, col = _parse_error_pos(str(e))
        diagnostics.append(lsp.Diagnostic(
            range=lsp.Range(
                start=lsp.Position(line=line, character=col),
                end=lsp.Position(line=line, character=col + 10),
            ),
            message=str(e),
            severity=lsp.DiagnosticSeverity.Error,
            source="ul-semcheck",
        ))

    return diagnostics


def _parse_error_pos(msg: str):
    """Extract (0-based line, 0-based col) from error strings like 'file:3:5 — ...'"""
    m = re.search(r":(\d+):(\d+)", msg)
    if m:
        return int(m.group(1)) - 1, int(m.group(2)) - 1
    return 0, 0


# ═══════════════════════════════════════════════════════════════════════════════
#  AST INDEX — parse source and build symbol lookup tables
# ═══════════════════════════════════════════════════════════════════════════════

def _try_parse(source, filename):
    """Try to lex+parse source, return Program or None."""
    try:
        tokens = tokenize(source, filename)
        return parse(tokens, filename)
    except Exception:
        return None


def _build_index(program, doc_uri=None):
    """Walk AST and build lookup tables.
    Returns (functions, types, variables):
      functions: name → FunctionDecl or ForeignImport
      types:     name → TypeDecl
      variables: [(name, line, col, type_name)]
    If doc_uri is provided, also resolves import "file.ul" declarations
    and includes symbols from imported files.
    """
    functions = {}
    types = {}
    variables = []

    for decl in program.decls:
        if isinstance(decl, FunctionDecl):
            functions[decl.name] = decl
            for p in decl.params:
                variables.append((p.name, p.line, p.col, p.type_.name))
            _collect_vars(decl.body, variables)
        elif isinstance(decl, TypeDecl):
            types[decl.name] = decl
        elif isinstance(decl, ForeignImport):
            functions[decl.name] = decl
            # Resolve local .ul imports for richer symbol info
            if doc_uri and decl.lib.endswith(".ul"):
                _resolve_local_import(decl, doc_uri, functions, types)

    return functions, types, variables


def _resolve_local_import(decl, doc_uri, functions, types):
    """For import "file.ul" declarations, find the .ul file and merge its symbols."""
    try:
        from urllib.parse import unquote
        doc_path = unquote(doc_uri.replace("file:///", "").replace("file://", ""))
        if len(doc_path) >= 3 and doc_path[0] == '/' and doc_path[2] == ':':
            doc_path = doc_path[1:]
        import_path = os.path.join(os.path.dirname(doc_path), decl.lib)
        if not os.path.isfile(import_path):
            return
        with open(import_path, "r", encoding="utf-8") as f:
            source = f.read()
        program = _try_parse(source, decl.lib)
        if not program:
            return
        for d in program.decls:
            if isinstance(d, FunctionDecl) and d.name not in functions:
                # Tag it with the source file for go-to-definition
                d._import_uri = "file:///" + import_path.replace("\\", "/")
                functions[d.name] = d
            elif isinstance(d, TypeDecl) and d.name not in types:
                d._import_uri = "file:///" + import_path.replace("\\", "/")
                types[d.name] = d
    except Exception:
        pass


def _collect_vars(stmts, variables):
    """Recursively collect variable declarations from statements."""
    for s in stmts:
        if isinstance(s, VarDecl):
            variables.append((s.name, s.line, s.col, s.type_.name))
        elif isinstance(s, If):
            _collect_vars(s.then_body, variables)
            _collect_vars(s.else_body, variables)
        elif isinstance(s, While):
            _collect_vars(s.body, variables)
        elif isinstance(s, For):
            variables.append((s.var, s.line, s.col, "int"))
            _collect_vars(s.body, variables)
        elif isinstance(s, Match):
            for case in s.cases:
                _collect_vars(case.body, variables)


def _all_fields(types, type_name):
    """Return all fields for a type including inherited ones (parent first)."""
    decl = types.get(type_name)
    if not decl:
        return []
    if decl.parent:
        return _all_fields(types, decl.parent) + list(decl.fields)
    return list(decl.fields)


def _format_type_hover(types, type_name):
    """Build a markdown hover string describing a user-defined type."""
    decl = types.get(type_name)
    if not decl:
        return None
    fields = _all_fields(types, type_name)
    lines = [f"**type `{type_name}`**"]
    if decl.parent:
        lines[0] += f" inherits `{decl.parent}`"
    lines.append("```")
    for f in fields:
        lines.append(f"  {f.type_.name} {f.name}")
    lines.append("```")
    return "\n".join(lines)


# ═══════════════════════════════════════════════════════════════════════════════
#  LSP HANDLERS
# ═══════════════════════════════════════════════════════════════════════════════

@server.feature(lsp.TEXT_DOCUMENT_DID_OPEN)
def did_open(params: lsp.DidOpenTextDocumentParams):
    _publish_diagnostics(params.text_document.uri, params.text_document.text)


@server.feature(lsp.TEXT_DOCUMENT_DID_CHANGE)
def did_change(params: lsp.DidChangeTextDocumentParams):
    doc = server.workspace.get_text_document(params.text_document.uri)
    _publish_diagnostics(params.text_document.uri, doc.source)


@server.feature(lsp.TEXT_DOCUMENT_DID_SAVE)
def did_save(params: lsp.DidSaveTextDocumentParams):
    doc = server.workspace.get_text_document(params.text_document.uri)
    _publish_diagnostics(params.text_document.uri, doc.source)


def _get_dr_memory(source):
    """Extract @dr memory setting from source. Returns value string or None."""
    program = _try_parse(source, "<dr>")
    if program:
        for decl in program.decls:
            if isinstance(decl, DrDirective) and decl.key == "memory":
                return decl.value
    # Legacy comment fallback
    for line in source.splitlines()[:20]:
        m = re.match(r'^\s*//\s*memory\s*:\s*(\w+)', line)
        if m:
            return m.group(1).lower()
    return None


def _collect_all_calls(stmts, calls):
    """Recursively collect all Call nodes from statements. calls = list of Call."""
    for s in stmts:
        if isinstance(s, VarDecl) and s.init is not None:
            _collect_calls_expr(s.init, calls)
        elif isinstance(s, If):
            _collect_calls_expr(s.condition, calls)
            _collect_all_calls(s.then_body, calls)
            _collect_all_calls(s.else_body, calls)
        elif isinstance(s, While):
            _collect_calls_expr(s.condition, calls)
            _collect_all_calls(s.body, calls)
        elif isinstance(s, For):
            _collect_calls_expr(s.iterable, calls)
            _collect_all_calls(s.body, calls)
        elif isinstance(s, ExprStmt):
            _collect_calls_expr(s.expr, calls)
        elif hasattr(s, 'value') and s.value is not None:
            _collect_calls_expr(s.value, calls)


def _collect_calls_expr(node, calls):
    """Recursively collect Call nodes from an expression."""
    if isinstance(node, Call):
        calls.append(node)
        for arg in node.args:
            _collect_calls_expr(arg, calls)
    elif hasattr(node, 'left'):
        _collect_calls_expr(node.left, calls)
        _collect_calls_expr(node.right, calls)
    elif hasattr(node, 'operand'):
        _collect_calls_expr(node.operand, calls)
    elif hasattr(node, 'expr') and not isinstance(node, ExprStmt):
        _collect_calls_expr(node.expr, calls)
    elif hasattr(node, 'target') and hasattr(node, 'index'):
        _collect_calls_expr(node.target, calls)
        _collect_calls_expr(node.index, calls)
    elif hasattr(node, 'elements') and isinstance(getattr(node, 'elements', None), list):
        for e in node.elements:
            _collect_calls_expr(e, calls)


def _dead_code_diagnostics(source, filename, uri):
    """Find functions declared but never called. Returns list of Diagnostic."""
    program = _try_parse(source, filename)
    if not program:
        return []

    # Collect all declared function names (skip main — it's the entry point)
    declared = {}
    for decl in program.decls:
        if isinstance(decl, FunctionDecl) and decl.name != "main":
            declared[decl.name] = decl

    if not declared:
        return []

    # Collect all call names in the current file
    called = set()
    for decl in program.decls:
        if isinstance(decl, FunctionDecl):
            calls = []
            _collect_all_calls(decl.body, calls)
            for c in calls:
                called.add(c.name)

    # Also scan workspace files for calls to these functions
    for _, ws_source in _iter_workspace_ul_files(skip_uri=uri):
        ws_prog = _try_parse(ws_source, "<ws>")
        if ws_prog:
            for decl in ws_prog.decls:
                if isinstance(decl, FunctionDecl):
                    calls = []
                    _collect_all_calls(decl.body, calls)
                    for c in calls:
                        called.add(c.name)

    diags = []
    for name, decl in declared.items():
        if name not in called:
            diags.append(lsp.Diagnostic(
                range=lsp.Range(
                    start=lsp.Position(line=decl.line - 1, character=decl.col - 1),
                    end=lsp.Position(line=decl.line - 1, character=decl.col - 1 + len(name)),
                ),
                message=f"Function '{name}' is never called.",
                severity=lsp.DiagnosticSeverity.Hint,
                source="ul-deadcode",
                tags=[lsp.DiagnosticTag.Unnecessary],
            ))
    return diags


def _dr_boundary_diagnostics(source, filename, uri):
    """Check for cross-file DR boundary violations at call sites."""
    program = _try_parse(source, filename)
    if not program:
        return []

    caller_dr = _get_dr_memory(source)
    if not caller_dr:
        return []

    # Build a map of function name → (file_uri, dr_memory) for workspace files
    extern_dr = {}
    for ws_uri, ws_source in _iter_workspace_ul_files(skip_uri=uri):
        ws_dr = _get_dr_memory(ws_source)
        if not ws_dr:
            continue
        ws_prog = _try_parse(ws_source, os.path.basename(ws_uri))
        if ws_prog:
            for decl in ws_prog.decls:
                if isinstance(decl, FunctionDecl):
                    extern_dr[decl.name] = ws_dr

    if not extern_dr:
        return []

    # Find calls to external functions with different DR
    diags = []
    for decl in program.decls:
        if isinstance(decl, FunctionDecl):
            calls = []
            _collect_all_calls(decl.body, calls)
            for c in calls:
                if c.name in extern_dr and extern_dr[c.name] != caller_dr:
                    diags.append(lsp.Diagnostic(
                        range=lsp.Range(
                            start=lsp.Position(line=c.line - 1, character=c.col - 1),
                            end=lsp.Position(line=c.line - 1, character=c.col - 1 + len(c.name)),
                        ),
                        message=f"DR boundary: caller is memory={caller_dr}, callee '{c.name}' is memory={extern_dr[c.name]}.",
                        severity=lsp.DiagnosticSeverity.Warning,
                        source="ul-dr-boundary",
                    ))
    return diags


def _publish_diagnostics(uri: str, source: str):
    filename = os.path.basename(uri)
    diags = _diagnose(source, filename)
    diags.extend(_dead_code_diagnostics(source, filename, uri))
    diags.extend(_dr_boundary_diagnostics(source, filename, uri))
    server.text_document_publish_diagnostics(
        lsp.PublishDiagnosticsParams(uri=uri, diagnostics=diags)
    )


@server.feature(
    lsp.TEXT_DOCUMENT_SEMANTIC_TOKENS_FULL,
    lsp.SemanticTokensRegistrationOptions(
        legend=LEGEND,
        full=True,
    ),
)
def semantic_tokens_full(params: lsp.SemanticTokensParams):
    doc = server.workspace.get_text_document(params.text_document.uri)
    data = _build_semantic_tokens(doc.source, os.path.basename(doc.uri))
    return lsp.SemanticTokens(data=data)


def _word_under_cursor(doc_source, position):
    """Return (word, start_col, end_col) for the word under the cursor, or (None, 0, 0)."""
    lines = doc_source.splitlines()
    line = position.line
    col = position.character
    if line >= len(lines):
        return None, 0, 0
    text = lines[line]
    start = col
    while start > 0 and (text[start - 1].isalnum() or text[start - 1] == '_'):
        start -= 1
    end = col
    while end < len(text) and (text[end].isalnum() or text[end] == '_'):
        end += 1
    word = text[start:end]
    return (word if word else None), start, end


@server.feature(lsp.TEXT_DOCUMENT_HOVER)
def hover(params: lsp.HoverParams):
    doc = server.workspace.get_text_document(params.text_document.uri)
    word, start, end = _word_under_cursor(doc.source, params.position)
    line = params.position.line

    if not word:
        return None

    hover_range = lsp.Range(
        start=lsp.Position(line=line, character=start),
        end=lsp.Position(line=line, character=end),
    )

    # 1) Keyword glossary — always check first
    gloss = GLOSSARY.get(word)
    if gloss:
        return lsp.Hover(
            contents=lsp.MarkupContent(
                kind=lsp.MarkupKind.Markdown,
                value=f"**`{word}`** — {gloss}",
            ),
            range=hover_range,
        )

    # 2) AST-based hover — parse and look up the symbol
    filename = os.path.basename(doc.uri)
    program = _try_parse(doc.source, filename)
    if not program:
        return None
    functions, types, variables = _build_index(program)

    # Hovering on a type name → show type definition
    if word in types:
        md = _format_type_hover(types, word)
        if md:
            return lsp.Hover(
                contents=lsp.MarkupContent(kind=lsp.MarkupKind.Markdown, value=md),
                range=hover_range,
            )

    # Hovering on a function name → show signature
    if word in functions:
        fn = functions[word]
        if isinstance(fn, FunctionDecl):
            params_str = ", ".join(f"{p.type_.name} {p.name}" for p in fn.params)
            ret = fn.return_type.name if fn.return_type else "none"
            md = f"**function** `{fn.name}({params_str})` returns `{ret}`"
            return lsp.Hover(
                contents=lsp.MarkupContent(kind=lsp.MarkupKind.Markdown, value=md),
                range=hover_range,
            )
        elif isinstance(fn, ForeignImport):
            params_str = ", ".join(f"{p.type_.name} {p.name}" for p in fn.params)
            ret = fn.return_type.name if fn.return_type else "none"
            md = f"**import** `\"{fn.lib}\"` — `{fn.name}({params_str})` returns `{ret}`"
            return lsp.Hover(
                contents=lsp.MarkupContent(kind=lsp.MarkupKind.Markdown, value=md),
                range=hover_range,
            )

    # Hovering on a variable → show its type, expand if user-defined
    for vname, vline, vcol, vtype in variables:
        if vname == word:
            if vtype in types:
                type_md = _format_type_hover(types, vtype)
                md = f"`{vtype}` **{word}**\n\n{type_md}"
            else:
                md = f"`{vtype}` **{word}**"
            return lsp.Hover(
                contents=lsp.MarkupContent(kind=lsp.MarkupKind.Markdown, value=md),
                range=hover_range,
            )

    return None


def _search_workspace_for_symbol(word, current_uri):
    """Scan all .ul files in workspace folders for a function or type named `word`.
    Returns lsp.Location or None."""
    for folder in (server.workspace.folders or {}).values():
        # folder.uri is like 'file:///c%3A/Users/...'
        folder_path = folder.uri.replace("file:///", "").replace("file://", "")
        # Decode percent-encoding and fix drive letter
        from urllib.parse import unquote
        folder_path = unquote(folder_path)
        # On Windows, file:///c%3A/... → c:/...
        if len(folder_path) >= 2 and folder_path[1] == ':':
            pass  # already good
        elif len(folder_path) >= 3 and folder_path[0] == '/' and folder_path[2] == ':':
            folder_path = folder_path[1:]  # strip leading /

        for ul_path in globmod.glob(os.path.join(folder_path, "**", "*.ul"), recursive=True):
            # Skip the file we already searched
            ul_uri = "file:///" + ul_path.replace("\\", "/")
            if ul_uri == current_uri:
                continue
            try:
                with open(ul_path, "r", encoding="utf-8") as f:
                    source = f.read()
                program = _try_parse(source, os.path.basename(ul_path))
                if not program:
                    continue
                functions, types, _ = _build_index(program)

                if word in functions:
                    decl = functions[word]
                    return lsp.Location(
                        uri=ul_uri,
                        range=lsp.Range(
                            start=lsp.Position(line=decl.line - 1, character=decl.col - 1),
                            end=lsp.Position(line=decl.line - 1, character=decl.col - 1 + len(word)),
                        ),
                    )
                if word in types:
                    decl = types[word]
                    return lsp.Location(
                        uri=ul_uri,
                        range=lsp.Range(
                            start=lsp.Position(line=decl.line - 1, character=decl.col - 1),
                            end=lsp.Position(line=decl.line - 1, character=decl.col - 1 + len(word)),
                        ),
                    )
            except Exception:
                continue
    return None


@server.feature(lsp.TEXT_DOCUMENT_DEFINITION)
def definition(params: lsp.DefinitionParams):
    doc = server.workspace.get_text_document(params.text_document.uri)
    word, _, _ = _word_under_cursor(doc.source, params.position)

    if not word:
        return None

    # Skip keywords — they have no definition to jump to
    if word in GLOSSARY:
        return None

    filename = os.path.basename(doc.uri)
    program = _try_parse(doc.source, filename)
    if not program:
        return None
    functions, types, variables = _build_index(program, doc_uri=params.text_document.uri)

    uri = params.text_document.uri

    # Function name → jump to FunctionDecl (may be in imported file)
    if word in functions:
        decl = functions[word]
        target_uri = getattr(decl, '_import_uri', uri)
        target_line = decl.line - 1  # AST is 1-based, LSP is 0-based
        target_col = decl.col - 1
        return lsp.Location(
            uri=target_uri,
            range=lsp.Range(
                start=lsp.Position(line=target_line, character=target_col),
                end=lsp.Position(line=target_line, character=target_col + len(word)),
            ),
        )

    # Type name → jump to TypeDecl (may be in imported file)
    if word in types:
        decl = types[word]
        target_uri = getattr(decl, '_import_uri', uri)
        target_line = decl.line - 1
        target_col = decl.col - 1
        return lsp.Location(
            uri=target_uri,
            range=lsp.Range(
                start=lsp.Position(line=target_line, character=target_col),
                end=lsp.Position(line=target_line, character=target_col + len(word)),
            ),
        )

    # Variable name → jump to its VarDecl / Param
    for vname, vline, vcol, vtype in variables:
        if vname == word:
            target_line = vline - 1
            target_col = vcol - 1
            return lsp.Location(
                uri=uri,
                range=lsp.Range(
                    start=lsp.Position(line=target_line, character=target_col),
                    end=lsp.Position(line=target_line, character=target_col + len(word)),
                ),
            )

    # Not found locally — search other .ul files in the workspace
    cross_file = _search_workspace_for_symbol(word, uri)
    if cross_file:
        return cross_file

    return None


# ═══════════════════════════════════════════════════════════════════════════════
#  AUTOCOMPLETE — textDocument/completion
# ═══════════════════════════════════════════════════════════════════════════════

def _vars_in_scope(program, cursor_line):
    """Collect variables visible at cursor_line (1-based) with scope awareness.
    Returns a list of (name, type_name) tuples, no duplicates."""
    results = {}  # name → type_name  (later entries overwrite, which is fine)

    for decl in program.decls:
        if isinstance(decl, FunctionDecl):
            # Only include params/body vars if the cursor is inside this function
            if decl.line <= cursor_line:
                fn_end = _estimate_end_line(decl.body, decl.line)
                if cursor_line <= fn_end + 2:  # +2 for 'end function' line tolerance
                    for p in decl.params:
                        results[p.name] = p.type_.name
                    _collect_vars_in_scope(decl.body, cursor_line, results)
    return list(results.items())


def _collect_vars_in_scope(stmts, cursor_line, results):
    """Walk statements, add VarDecl/For vars that are declared before cursor_line."""
    for s in stmts:
        if isinstance(s, VarDecl):
            if s.line <= cursor_line:
                results[s.name] = s.type_.name
        elif isinstance(s, For):
            if s.line <= cursor_line:
                results[s.var] = "int"
            _collect_vars_in_scope(s.body, cursor_line, results)
        elif isinstance(s, If):
            _collect_vars_in_scope(s.then_body, cursor_line, results)
            _collect_vars_in_scope(s.else_body, cursor_line, results)
        elif isinstance(s, While):
            _collect_vars_in_scope(s.body, cursor_line, results)
        elif isinstance(s, Match):
            for case in s.cases:
                _collect_vars_in_scope(case.body, cursor_line, results)


def _estimate_end_line(body, start_line):
    """Estimate the last line number spanned by a list of statements."""
    last = start_line
    for s in body:
        sl = getattr(s, 'line', 0)
        if sl > last:
            last = sl
        # Recurse into sub-bodies for a better estimate
        for attr in ('body', 'then_body', 'else_body'):
            sub = getattr(s, attr, None)
            if sub and isinstance(sub, list):
                sub_last = _estimate_end_line(sub, last)
                if sub_last > last:
                    last = sub_last
        if isinstance(s, Match):
            for case in s.cases:
                sub_last = _estimate_end_line(case.body, last)
                if sub_last > last:
                    last = sub_last
    return last


@server.feature(
    lsp.TEXT_DOCUMENT_COMPLETION,
    lsp.CompletionOptions(trigger_characters=["."]),
)
def completions(params: lsp.CompletionParams):
    doc = server.workspace.get_text_document(params.text_document.uri)
    filename = os.path.basename(doc.uri)
    cursor_line_0 = params.position.line       # 0-based
    cursor_line_1 = cursor_line_0 + 1          # 1-based (AST convention)

    items = []
    seen = set()

    def add(label, kind, detail=None):
        if label in seen:
            return
        seen.add(label)
        items.append(lsp.CompletionItem(
            label=label,
            kind=kind,
            detail=detail,
            insert_text=label,
            insert_text_format=lsp.InsertTextFormat.PlainText,
            documentation=None,
        ))

    # 1) All UL keywords — short names only
    for kw in sorted(KEYWORDS):
        if kw in TYPE_KW:
            add(kw, lsp.CompletionItemKind.TypeParameter)
        else:
            add(kw, lsp.CompletionItemKind.Keyword)

    # 2–5) AST-based completions: functions, types, variables
    program = _try_parse(doc.source, filename)
    if program:
        functions, types, _ = _build_index(program)

        for name, decl in functions.items():
            if isinstance(decl, FunctionDecl):
                params_str = ", ".join(f"{p.type_.name} {p.name}" for p in decl.params)
                ret = decl.return_type.name if decl.return_type else "none"
                add(name, lsp.CompletionItemKind.Function, f"({params_str}) returns {ret}")
            elif isinstance(decl, ForeignImport):
                params_str = ", ".join(f"{p.type_.name} {p.name}" for p in decl.params)
                ret = decl.return_type.name if decl.return_type else "none"
                add(name, lsp.CompletionItemKind.Function, f"({params_str}) returns {ret}")

        for name, decl in types.items():
            add(name, lsp.CompletionItemKind.Class, "type")

        for vname, vtype in _vars_in_scope(program, cursor_line_1):
            add(vname, lsp.CompletionItemKind.Variable, vtype)

    return lsp.CompletionList(is_incomplete=False, items=items)


# ═══════════════════════════════════════════════════════════════════════════════
#  DOCUMENT FORMATTING
# ═══════════════════════════════════════════════════════════════════════════════

# Keywords that increase indent for the following lines
_INDENT_OPENERS = {
    "function", "if", "else", "while", "for", "match",
}

# Patterns that decrease indent (these lines are at the outer level)
_INDENT_CLOSERS = {
    "end", "else",
}

# Operators that should have spaces around them
_SPACED_OPS = re.compile(
    r'(?<=[^\s=!<>+\-*/%])([=!<>]=|[+\-*/%]=?|<<|>>|->|[=<>])'
    r'|([=!<>]=|[+\-*/%]=?|<<|>>|->|[=<>])(?=[^\s=!<>+\-*/%])'
)


def _format_source(source: str) -> str:
    """Format UniLogic source: fix indentation, spacing, blank lines."""
    lines = source.split("\n")
    out = []
    indent = 0
    prev_was_blank = False
    prev_was_end_function = False

    for raw_line in lines:
        stripped = raw_line.strip()

        # ── Blank line handling ──
        if not stripped:
            if prev_was_blank:
                continue  # collapse consecutive blanks
            prev_was_blank = True
            prev_was_end_function = False
            out.append("")
            continue
        prev_was_blank = False

        # ── Determine indent adjustment ──
        first_word = stripped.split()[0] if stripped.split() else ""
        # Remove // prefix for non-comment first-word detection
        is_comment = stripped.startswith("//")
        is_directive = stripped.startswith("@")

        # Check if this line closes a block (outdent before writing)
        if not is_comment and not is_directive and first_word in _INDENT_CLOSERS:
            indent = max(0, indent - 1)

        # ── Insert blank line between function declarations ──
        if first_word == "function" and out and out[-1] != "":
            # Ensure exactly one blank line before a new function
            if not prev_was_end_function or (out and out[-1] != ""):
                if out and out[-1] != "":
                    out.append("")

        # ── Build the formatted line ──
        if is_directive or is_comment:
            # Directives (@dr, @norm) and comments at current indent
            formatted = ("  " * indent) + stripped
        else:
            # Fix operator spacing on non-comment lines
            formatted = ("  " * indent) + _fix_operator_spacing(stripped)

        # Remove trailing whitespace
        out.append(formatted.rstrip())

        # Track end function for blank-line insertion
        prev_was_end_function = stripped.startswith("end function")

        # ── Check if this line opens a block (indent after writing) ──
        if not is_comment and not is_directive:
            if first_word in _INDENT_OPENERS:
                indent += 1

    # Ensure file ends with exactly one newline
    while out and out[-1] == "":
        out.pop()
    out.append("")

    return "\n".join(out)


def _fix_operator_spacing(line: str) -> str:
    """Add spaces around operators: a+b → a + b, x=1 → x = 1.
    Careful not to mangle strings, comments, or arrow operators."""
    # Don't touch lines that are just comments
    if line.lstrip().startswith("//"):
        return line

    # Split off trailing comment if any
    comment = ""
    in_string = False
    for i, ch in enumerate(line):
        if ch == '"' and (i == 0 or line[i-1] != '\\'):
            in_string = not in_string
        if ch == '/' and i + 1 < len(line) and line[i+1] == '/' and not in_string:
            comment = line[i:]
            line = line[:i]
            break

    # Process the code portion — apply spacing rules
    result = []
    i = 0
    while i < len(line):
        ch = line[i]

        # Skip strings
        if ch == '"':
            j = i + 1
            while j < len(line) and line[j] != '"':
                if line[j] == '\\':
                    j += 1
                j += 1
            result.append(line[i:j+1])
            i = j + 1
            continue

        # Arrow operator -> (don't space)
        if ch == '-' and i + 1 < len(line) and line[i+1] == '>':
            result.append("->")
            i += 2
            continue

        # Two-char operators: ==, !=, <=, >=, +=, -=, *=, /=, %=, <<, >>
        if i + 1 < len(line) and line[i:i+2] in ("==", "!=", "<=", ">=",
                                                    "+=", "-=", "*=", "/=", "%=",
                                                    "<<", ">>"):
            op = line[i:i+2]
            # Ensure space before
            if result and result[-1] != ' ' and result[-1] != '(':
                result.append(' ')
            result.append(op)
            i += 2
            # Ensure space after
            if i < len(line) and line[i] != ' ':
                result.append(' ')
            continue

        # Single-char operators that get spaces: = + - * / % < >
        # But NOT after ( or before ) or in unary context
        if ch in "=<>":
            if result and result[-1] != ' ' and result[-1] != '(':
                result.append(' ')
            result.append(ch)
            i += 1
            if i < len(line) and line[i] != ' ' and line[i] != '=':
                result.append(' ')
            continue

        if ch in "+-" and i > 0:
            prev_non_space = ''.join(result).rstrip()
            # Binary context: previous token ends with alnum, ), or ]
            if prev_non_space and prev_non_space[-1] in 'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_)]':
                if result and result[-1] != ' ':
                    result.append(' ')
                result.append(ch)
                i += 1
                if i < len(line) and line[i] != ' ':
                    result.append(' ')
                continue

        if ch in "*/%":
            if result and result[-1] != ' ' and result[-1] != '(':
                result.append(' ')
            result.append(ch)
            i += 1
            if i < len(line) and line[i] != ' ' and line[i] != ')':
                result.append(' ')
            continue

        result.append(ch)
        i += 1

    formatted = ''.join(result).rstrip()
    # Collapse multiple spaces (but preserve indentation)
    leading = len(formatted) - len(formatted.lstrip())
    body = re.sub(r'  +', ' ', formatted[leading:])
    formatted = formatted[:leading] + body

    if comment:
        formatted = formatted.rstrip() + "  " + comment
    return formatted


@server.feature(
    lsp.TEXT_DOCUMENT_FORMATTING,
    lsp.DocumentFormattingRegistrationOptions(),
)
def formatting(params: lsp.DocumentFormattingParams):
    doc = server.workspace.get_text_document(params.text_document.uri)
    original = doc.source
    formatted = _format_source(original)

    if formatted == original:
        return []

    # Replace the entire document
    line_count = len(original.split("\n"))
    return [lsp.TextEdit(
        range=lsp.Range(
            start=lsp.Position(line=0, character=0),
            end=lsp.Position(line=line_count, character=0),
        ),
        new_text=formatted,
    )]


# ═══════════════════════════════════════════════════════════════════════════════
#  DOCUMENT SYMBOLS — populates VS Code Outline panel and breadcrumbs
# ═══════════════════════════════════════════════════════════════════════════════

def _sym_range(start_line, start_col, end_line, end_col=0):
    """Build an LSP Range from 1-based AST coords to 0-based LSP coords."""
    return lsp.Range(
        start=lsp.Position(line=max(0, start_line - 1), character=max(0, start_col - 1)),
        end=lsp.Position(line=max(0, end_line - 1), character=end_col),
    )


def _find_end_line(source, start_line_0, end_keyword):
    """Scan source lines from start_line_0 to find 'end function' / 'end type' etc.
    Returns 1-based line number, or start_line_0+2 as fallback."""
    lines = source.splitlines()
    for i in range(start_line_0, len(lines)):
        if lines[i].strip().startswith(end_keyword):
            return i + 1  # 1-based
    return start_line_0 + 2


@server.feature(lsp.TEXT_DOCUMENT_DOCUMENT_SYMBOL)
def document_symbol(params: lsp.DocumentSymbolParams):
    try:
        return _document_symbol_impl(params)
    except Exception as e:
        log.error("documentSymbol error: %s", e, exc_info=True)
        import traceback; traceback.print_exc()
        return []


def _document_symbol_impl(params):
    doc = server.workspace.get_text_document(params.text_document.uri)
    filename = os.path.basename(doc.uri)
    program = _try_parse(doc.source, filename)
    if not program:
        return []

    source_lines = doc.source.splitlines()
    symbols = []

    def safe_range(start_line, start_col, end_line, end_col=None):
        """Build range from 1-based AST coords. Clamps to valid bounds."""
        sl = max(0, start_line - 1)
        sc = max(0, start_col - 1)
        el = max(sl, end_line - 1)
        ec = end_col if end_col is not None else (len(source_lines[el]) if el < len(source_lines) else 0)
        return lsp.Range(
            start=lsp.Position(line=sl, character=sc),
            end=lsp.Position(line=el, character=ec),
        )

    for decl in program.decls:
        if isinstance(decl, FunctionDecl):
            end_line = _find_end_line(doc.source, decl.line - 1, "end function")
            detail = "yields" if getattr(decl, 'is_generator', False) else "returns"
            ret_name = decl.return_type.name if decl.return_type else "none"
            params_str = ", ".join(f"{p.type_.name} {p.name}" for p in decl.params)
            fn_range = safe_range(decl.line, 1, end_line)
            fn_sel = safe_range(decl.line, decl.col, decl.line, decl.col - 1 + len(decl.name))

            # No children — they can cause range containment issues
            symbols.append(lsp.DocumentSymbol(
                name=decl.name,
                kind=lsp.SymbolKind.Function,
                range=fn_range,
                selection_range=fn_sel,
                detail=f"({params_str}) {detail} {ret_name}",
            ))

        elif isinstance(decl, TypeDecl):
            end_line = _find_end_line(doc.source, decl.line - 1, "end type")
            detail = f"inherits {decl.parent}" if decl.parent else "type"
            symbols.append(lsp.DocumentSymbol(
                name=decl.name,
                kind=lsp.SymbolKind.Class,
                range=safe_range(decl.line, 1, end_line),
                selection_range=safe_range(decl.line, decl.col, decl.line, decl.col - 1 + len(decl.name)),
                detail=detail,
            ))

        elif isinstance(decl, ForeignImport):
            params_str = ", ".join(f"{p.type_.name} {p.name}" for p in decl.params)
            ret_name = decl.return_type.name if decl.return_type else "none"
            line_end = len(source_lines[decl.line - 1]) if decl.line - 1 < len(source_lines) else 0
            symbols.append(lsp.DocumentSymbol(
                name=decl.name,
                kind=lsp.SymbolKind.Interface,
                range=safe_range(decl.line, 1, decl.line, line_end),
                selection_range=safe_range(decl.line, decl.col, decl.line, decl.col - 1 + len(decl.name)),
                detail=f'import "{decl.lib}" ({params_str}) returns {ret_name}',
            ))

        elif isinstance(decl, DrDirective):
            line_end = len(source_lines[decl.line - 1]) if decl.line - 1 < len(source_lines) else 0
            symbols.append(lsp.DocumentSymbol(
                name=f"@dr {decl.key}",
                kind=lsp.SymbolKind.Property,
                range=safe_range(decl.line, 1, decl.line, line_end),
                selection_range=safe_range(decl.line, decl.col, decl.line, decl.col + 2),
                detail=decl.value,
            ))

        elif isinstance(decl, NormDirective):
            line_end = len(source_lines[decl.line - 1]) if decl.line - 1 < len(source_lines) else 0
            symbols.append(lsp.DocumentSymbol(
                name="@norm",
                kind=lsp.SymbolKind.Property,
                range=safe_range(decl.line, 1, decl.line, line_end),
                selection_range=safe_range(decl.line, decl.col, decl.line, decl.col + 4),
                detail=str(decl.level),
            ))

    return symbols


# ═══════════════════════════════════════════════════════════════════════════════
#  SHARED — find all occurrences of an identifier across workspace
# ═══════════════════════════════════════════════════════════════════════════════

def _find_identifier_occurrences(source, word):
    """Find all (0-based line, 0-based col) positions of `word` as a whole identifier."""
    positions = []
    pattern = re.compile(r'\b' + re.escape(word) + r'\b')
    for i, line in enumerate(source.splitlines()):
        for m in pattern.finditer(line):
            positions.append((i, m.start()))
    return positions


def _workspace_folder_path(folder):
    """Convert a workspace folder URI to a local path."""
    from urllib.parse import unquote
    fp = folder.uri.replace("file:///", "").replace("file://", "")
    fp = unquote(fp)
    if len(fp) >= 3 and fp[0] == '/' and fp[2] == ':':
        fp = fp[1:]
    return fp


def _iter_workspace_ul_files(skip_uri=None):
    """Yield (ul_uri, source) for all .ul files in workspace folders."""
    for folder in (server.workspace.folders or {}).values():
        fp = _workspace_folder_path(folder)
        for ul_path in globmod.glob(os.path.join(fp, "**", "*.ul"), recursive=True):
            ul_uri = "file:///" + ul_path.replace("\\", "/")
            if ul_uri == skip_uri:
                continue
            try:
                with open(ul_path, "r", encoding="utf-8") as f:
                    yield ul_uri, f.read()
            except Exception:
                continue


# ═══════════════════════════════════════════════════════════════════════════════
#  RENAME SYMBOL — textDocument/rename
# ═══════════════════════════════════════════════════════════════════════════════

@server.feature(lsp.TEXT_DOCUMENT_RENAME)
def rename(params: lsp.RenameParams):
    doc = server.workspace.get_text_document(params.text_document.uri)
    word, _, _ = _word_under_cursor(doc.source, params.position)
    new_name = params.new_name

    if not word or word in GLOSSARY or word in KEYWORDS:
        return None

    changes = {}

    # Current file
    edits = []
    for line_0, col_0 in _find_identifier_occurrences(doc.source, word):
        edits.append(lsp.TextEdit(
            range=lsp.Range(
                start=lsp.Position(line=line_0, character=col_0),
                end=lsp.Position(line=line_0, character=col_0 + len(word)),
            ),
            new_text=new_name,
        ))
    if edits:
        changes[params.text_document.uri] = edits

    # Other workspace files
    for ul_uri, source in _iter_workspace_ul_files(skip_uri=params.text_document.uri):
        file_edits = []
        for line_0, col_0 in _find_identifier_occurrences(source, word):
            file_edits.append(lsp.TextEdit(
                range=lsp.Range(
                    start=lsp.Position(line=line_0, character=col_0),
                    end=lsp.Position(line=line_0, character=col_0 + len(word)),
                ),
                new_text=new_name,
            ))
        if file_edits:
            changes[ul_uri] = file_edits

    if not changes:
        return None
    return lsp.WorkspaceEdit(changes=changes)


# ═══════════════════════════════════════════════════════════════════════════════
#  FIND ALL REFERENCES — textDocument/references
# ═══════════════════════════════════════════════════════════════════════════════

@server.feature(lsp.TEXT_DOCUMENT_REFERENCES)
def references(params: lsp.ReferenceParams):
    doc = server.workspace.get_text_document(params.text_document.uri)
    word, _, _ = _word_under_cursor(doc.source, params.position)

    if not word or word in GLOSSARY or word in KEYWORDS:
        return []

    locations = []

    # Current file
    for line_0, col_0 in _find_identifier_occurrences(doc.source, word):
        locations.append(lsp.Location(
            uri=params.text_document.uri,
            range=lsp.Range(
                start=lsp.Position(line=line_0, character=col_0),
                end=lsp.Position(line=line_0, character=col_0 + len(word)),
            ),
        ))

    # Other workspace files
    for ul_uri, source in _iter_workspace_ul_files(skip_uri=params.text_document.uri):
        for line_0, col_0 in _find_identifier_occurrences(source, word):
            locations.append(lsp.Location(
                uri=ul_uri,
                range=lsp.Range(
                    start=lsp.Position(line=line_0, character=col_0),
                    end=lsp.Position(line=line_0, character=col_0 + len(word)),
                ),
            ))

    return locations


# ═══════════════════════════════════════════════════════════════════════════════
#  CODE ACTIONS — textDocument/codeAction
# ═══════════════════════════════════════════════════════════════════════════════

@server.feature(lsp.TEXT_DOCUMENT_CODE_ACTION)
def code_action(params: lsp.CodeActionParams):
    doc = server.workspace.get_text_document(params.text_document.uri)
    actions = []

    for diag in params.context.diagnostics:
        msg = diag.message
        line_0 = diag.range.start.line
        source_lines = doc.source.splitlines()
        line_text = source_lines[line_0] if line_0 < len(source_lines) else ""
        indent = len(line_text) - len(line_text.lstrip())
        indent_str = " " * indent

        # Undefined variable → suggest declaring it
        m = re.search(r"undefined variable '(\w+)'", msg)
        if m:
            var_name = m.group(1)
            new_text = f"{indent_str}int {var_name} = 0\n"
            edit = lsp.WorkspaceEdit(changes={
                params.text_document.uri: [lsp.TextEdit(
                    range=lsp.Range(
                        start=lsp.Position(line=line_0, character=0),
                        end=lsp.Position(line=line_0, character=0),
                    ),
                    new_text=new_text,
                )]
            })
            actions.append(lsp.CodeAction(
                title=f"Declare 'int {var_name} = 0'",
                kind=lsp.CodeActionKind.QuickFix,
                edit=edit,
                diagnostics=[diag],
            ))

        # Unknown function → suggest checking spelling
        m = re.search(r"undefined function '(\w+)'", msg) or re.search(r"unknown function '(\w+)'", msg)
        if m:
            func_name = m.group(1)
            # Find similar function names
            filename = os.path.basename(doc.uri)
            program = _try_parse(doc.source, filename)
            if program:
                functions, _, _ = _build_index(program)
                for known in functions:
                    # Simple edit distance: off by 1 char or case difference
                    if (len(known) == len(func_name) and
                        sum(1 for a, b in zip(known, func_name) if a != b) == 1):
                        repl_edit = lsp.WorkspaceEdit(changes={
                            params.text_document.uri: [lsp.TextEdit(
                                range=diag.range,
                                new_text=known,
                            )]
                        })
                        actions.append(lsp.CodeAction(
                            title=f"Did you mean '{known}'?",
                            kind=lsp.CodeActionKind.QuickFix,
                            edit=repl_edit,
                            diagnostics=[diag],
                        ))

        # Type mismatch → suggest cast
        m = re.search(r"type mismatch.*expected '(\w+)'.*got '(\w+)'", msg)
        if m:
            expected = m.group(1)
            got = m.group(2)
            # Find the expression on the line to wrap
            actions.append(lsp.CodeAction(
                title=f"Cast to {expected} using change(...)->{expected}",
                kind=lsp.CodeActionKind.QuickFix,
                diagnostics=[diag],
            ))

    return actions


# ═══════════════════════════════════════════════════════════════════════════════
#  INLAY HINTS — textDocument/inlayHint
# ═══════════════════════════════════════════════════════════════════════════════

@server.feature(lsp.TEXT_DOCUMENT_INLAY_HINT)
def inlay_hints(params: lsp.InlayHintParams):
    doc = server.workspace.get_text_document(params.text_document.uri)
    filename = os.path.basename(doc.uri)
    program = _try_parse(doc.source, filename)
    if not program:
        return []

    hints = []
    functions, types, _ = _build_index(program)
    start_line = params.range.start.line  # 0-based
    end_line = params.range.end.line      # 0-based

    for decl in program.decls:
        if not isinstance(decl, FunctionDecl):
            continue

        for stmt in decl.body:
            sl = getattr(stmt, 'line', 0) - 1  # to 0-based
            if sl < start_line or sl > end_line:
                continue

            # VarDecl → show ": type" after the variable name
            if isinstance(stmt, VarDecl):
                name_end = stmt.col - 1 + len(stmt.name)
                type_label = ("fixed " if stmt.fixed else "") + stmt.type_.name
                hints.append(lsp.InlayHint(
                    position=lsp.Position(line=sl, character=name_end),
                    label=f": {type_label}",
                    kind=lsp.InlayHintKind.Type,
                    padding_left=False,
                    padding_right=True,
                ))

            # ExprStmt/VarDecl with Call init → show parameter names at call site
            call_node = None
            if isinstance(stmt, VarDecl) and stmt.init is not None:
                if isinstance(stmt.init, Call):
                    call_node = stmt.init
            elif isinstance(stmt, ExprStmt):
                if isinstance(stmt.expr, Call):
                    call_node = stmt.expr

            if call_node and call_node.name in functions:
                fn = functions[call_node.name]
                if isinstance(fn, FunctionDecl):
                    for i, arg in enumerate(call_node.args):
                        if i < len(fn.params):
                            arg_line = getattr(arg, 'line', 0) - 1
                            arg_col = getattr(arg, 'col', 0) - 1
                            if arg_line >= start_line and arg_line <= end_line:
                                hints.append(lsp.InlayHint(
                                    position=lsp.Position(line=arg_line, character=arg_col),
                                    label=f"{fn.params[i].name}:",
                                    kind=lsp.InlayHintKind.Parameter,
                                    padding_left=False,
                                    padding_right=True,
                                ))

    return hints


# ═══════════════════════════════════════════════════════════════════════════════
#  WORKSPACE SYMBOL SEARCH — workspace/symbol (Ctrl+T)
# ═══════════════════════════════════════════════════════════════════════════════

@server.feature(lsp.WORKSPACE_SYMBOL)
def workspace_symbol(params: lsp.WorkspaceSymbolParams):
    query = params.query.lower()
    if not query:
        return []

    results = []

    # Search all open documents
    for doc_uri in list(server.workspace.text_documents.keys()):
        doc = server.workspace.get_text_document(doc_uri)
        program = _try_parse(doc.source, os.path.basename(doc.uri))
        if not program:
            continue
        _collect_workspace_symbols(program, doc.uri, query, results)

    # Search workspace .ul files on disk
    for ul_uri, source in _iter_workspace_ul_files():
        program = _try_parse(source, os.path.basename(ul_uri))
        if not program:
            continue
        _collect_workspace_symbols(program, ul_uri, query, results)

    return results


def _collect_workspace_symbols(program, uri, query, results):
    """Collect matching symbols from a parsed program into results list."""
    seen = set()
    for decl in program.decls:
        if isinstance(decl, FunctionDecl):
            if query in decl.name.lower() and decl.name not in seen:
                seen.add(decl.name)
                params_str = ", ".join(f"{p.type_.name} {p.name}" for p in decl.params)
                ret = decl.return_type.name if decl.return_type else "none"
                results.append(lsp.SymbolInformation(
                    name=decl.name,
                    kind=lsp.SymbolKind.Function,
                    location=lsp.Location(
                        uri=uri,
                        range=lsp.Range(
                            start=lsp.Position(line=decl.line - 1, character=decl.col - 1),
                            end=lsp.Position(line=decl.line - 1, character=decl.col - 1 + len(decl.name)),
                        ),
                    ),
                    container_name=f"({params_str}) returns {ret}",
                ))
        elif isinstance(decl, TypeDecl):
            if query in decl.name.lower() and decl.name not in seen:
                seen.add(decl.name)
                results.append(lsp.SymbolInformation(
                    name=decl.name,
                    kind=lsp.SymbolKind.Class,
                    location=lsp.Location(
                        uri=uri,
                        range=lsp.Range(
                            start=lsp.Position(line=decl.line - 1, character=decl.col - 1),
                            end=lsp.Position(line=decl.line - 1, character=decl.col - 1 + len(decl.name)),
                        ),
                    ),
                ))
        elif isinstance(decl, ForeignImport):
            if query in decl.name.lower() and decl.name not in seen:
                seen.add(decl.name)
                results.append(lsp.SymbolInformation(
                    name=decl.name,
                    kind=lsp.SymbolKind.Interface,
                    location=lsp.Location(
                        uri=uri,
                        range=lsp.Range(
                            start=lsp.Position(line=decl.line - 1, character=decl.col - 1),
                            end=lsp.Position(line=decl.line - 1, character=decl.col - 1 + len(decl.name)),
                        ),
                    ),
                    container_name=f'import "{decl.lib}"',
                ))


# ═══════════════════════════════════════════════════════════════════════════════
#  ENTRY POINT
# ═══════════════════════════════════════════════════════════════════════════════

if __name__ == "__main__":
    if "--tcp" in sys.argv:
        server.start_tcp("127.0.0.1", 2087)
    else:
        server.start_io()
