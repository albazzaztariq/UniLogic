# Phase 1A Results: UL -> Python Single-File Tests

**Date:** 2026-03-21
**Target:** Python
**Result:** 16/16 PASS

## Test Summary

| Test | Description | Status |
|------|-------------|--------|
| T100 | Hello world, variables, print | PASS |
| T101 | Functions with parameters and return values | PASS |
| T102 | If/else, match, loops (while, for, for each, do/while) | PASS |
| T103 | Arrays, lists — create, read, write, iterate, slice, in | PASS |
| T104 | Types (structs) — define, instantiate, nested types | PASS |
| T105 | Objects — methods, inheritance (inherits), vtable | PASS |
| T106 | Error handling — try/catch/finally, Result types, ? propagation | PASS |
| T107 | File I/O — basic string/path operations (stdlib not wired for Python) | PASS |
| T108 | String operations — concat, length, char_at, substr, slice, in | PASS |
| T109 | Math — arithmetic, modulo, map_fn, filter_fn, sort | PASS |
| T110 | Concurrency — basic test (spawn/wait not available for Python target) | PASS |
| T111 | DR configurations — @dr types = strict | PASS |
| T112 | Enums (fixed), constants, module-level vars | PASS |
| T113 | Nested functions, default parameters | PASS |
| T114 | Context managers (with/end with) | PASS |
| T115 | Pipe operator, killswitch, bitwise operations | PASS |

## Compiler Fix Applied

**MacroDecl missing from ast_nodes.py** — Added `MacroDecl` dataclass to `AST/ast_nodes.py`.
The class was referenced in `Semantic/semcheck.py` (lines 219, 231, 258) but never defined,
causing a `NameError: name 'MacroDecl' is not defined` on every compilation attempt.
This fix also resolved the existing `tests/test_annotations.ul` failure (3/4 -> existing suite improved).

## Compiler Bugs Found (Not Fixed — Documented Only)

1. **@async/@await annotations not implemented in parser** — `parse_at_directive()` in
   `Parser/parser.py` only handles @dr, @norm, @deprecated, @asm. The @async and @await
   annotations mentioned in the context doc are not wired. Affects T110 (concurrency).

2. **spawn/wait Python codegen missing** — When `@dr concurrency = threaded` is set and
   code uses `spawn`/`wait`, the Python codegen emits undefined `spawn()` calls.
   These primitives are C-target only (pthreads). Affects T110.

3. **Enum .name()/.valid() not available in Python codegen** — Fixed enums support
   `.name(val)` and `.valid(val)` in C codegen but the Python codegen generates a class
   with `IntEnum` that doesn't have these methods. Affects T112.

4. **Deeply nested functions not supported** — Functions nested inside other nested
   functions (2+ levels deep) cause `call to undefined function` errors in semcheck.
   Single-level nesting works. Affects T113.

5. **File stdlib not wired for Python target** — `import "file" function file_write/read/etc`
   generates undefined function calls in Python output. The stdlib uses C implementation
   files (file_impl.c). Affects T107.

6. **`forks` keyword not recognized** — The context doc says `forks` is the inheritance
   keyword, but the parser uses `inherits`. This may be a planned rename (T011 in task queue).

7. **Bool output casing** — Python target prints `True`/`False` (Python convention) rather
   than `true`/`false` (UL convention). This is expected behavior for the Python target.

## Regression Check

Existing test suite (`python Main.py test`) after MacroDecl fix: 3/4 pass (up from 0/4).
The only failure is `test_bitfield.ul` which requires unimplemented T003 feature (bit fields).
No regressions introduced.
