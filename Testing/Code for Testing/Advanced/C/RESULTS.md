# Phase 5C: Cross-Target Proof Results

Verifies that the same UL program produces **identical output** when compiled to Python and C.

**Date:** 2026-03-21
**Total tests:** 24 (T100-T115 + M001-M008)
**PASS (identical output):** 24
**MISMATCH (compiler bug):** 0
**Other failures:** 0

## Results Table

| Test | PY Compile | PY Run | C Compile | GCC | C Run | Match | Status |
|------|-----------|--------|-----------|-----|-------|-------|--------|
| T100 | YES | YES | YES | YES | YES | IDENTICAL | PASS |
| T101 | YES | YES | YES | YES | YES | IDENTICAL | PASS |
| T102 | YES | YES | YES | YES | YES | IDENTICAL | PASS |
| T103 | YES | YES | YES | YES | YES | IDENTICAL | PASS |
| T104 | YES | YES | YES | YES | YES | IDENTICAL | PASS |
| T105 | YES | YES | YES | YES | YES | IDENTICAL | PASS |
| T106 | YES | YES | YES | YES | YES | IDENTICAL | PASS |
| T107 | YES | YES | YES | YES | YES | IDENTICAL | PASS |
| T108 | YES | YES | YES | YES | YES | IDENTICAL | PASS |
| T109 | YES | YES | YES | YES | YES | IDENTICAL | PASS |
| T110 | YES | YES | YES | YES | YES | IDENTICAL | PASS |
| T111 | YES | YES | YES | YES | YES | IDENTICAL | PASS |
| T112 | YES | YES | YES | YES | YES | IDENTICAL | PASS |
| T113 | YES | YES | YES | YES | YES | IDENTICAL | PASS |
| T114 | YES | YES | YES | YES | YES | IDENTICAL | PASS |
| T115 | YES | YES | YES | YES | YES | IDENTICAL | PASS |
| M001 | YES | YES | YES | YES | YES | IDENTICAL | PASS |
| M002 | YES | YES | YES | YES | YES | IDENTICAL | PASS |
| M003 | YES | YES | YES | YES | YES | IDENTICAL | PASS |
| M004 | YES | YES | YES | YES | YES | IDENTICAL | PASS |
| M005 | YES | YES | YES | YES | YES | IDENTICAL | PASS |
| M006 | YES | YES | YES | YES | YES | IDENTICAL | PASS |
| M007 | YES | YES | YES | YES | YES | IDENTICAL | PASS |
| M008 | YES | YES | YES | YES | YES | IDENTICAL | PASS |

## Compiler Bugs Found and Fixed

Seven real compiler bugs were discovered during Phase 5C and fixed in codegen_c.py / semcheck.py:

### Bug 1: Bool prints 1/0 in C instead of True/False
- **Root cause:** C codegen used printf("%d", flag) for bool values.
- **Fix:** Added _is_bool_expr() helper; Print stmt now emits printf("%s", val ? "True" : "False") for bool.
- **Files:** Codegen/codegen_c.py

### Bug 2: Float precision mismatch (3.14 vs 3.140000)
- **Root cause:** UL float mapped to C float (32-bit), losing precision vs Python 64-bit. Also %f always prints 6 decimal places.
- **Fix 1:** Changed UL_TO_C["float"] from "float" to "double" to match Python semantics.
- **Fix 2:** Added __ul_print_float() helper using %.10g that preserves Python-style repr (e.g. 10.0 not 10).
- **Files:** Semantic/semcheck.py, Codegen/codegen_c.py

### Bug 3: array.len() emits undefined array_len() call
- **Root cause:** Static array .len() fell through to array_{method}() template, generating array_len(arr, size) which is not a real function.
- **Fix:** Added explicit node.method == "len" check in static array handler to return the size expression directly.
- **Files:** Codegen/codegen_c.py

### Bug 4: String concatenation on object fields generates invalid C
- **Root cause:** _is_string_expr() did not handle FieldAccess nodes or self references, so self.name + " str" used raw + instead of __ul_strcat().
- **Fix 1:** Extended _resolve_field_type() to handle the self identifier inside object methods.
- **Fix 2:** Added FieldAccess case to _is_string_expr().
- **Files:** Codegen/codegen_c.py

### Bug 5: Cross-nested function calls use unmangled names
- **Root cause:** When hoisting nested functions, _nested_name_map was only set during gen_function() for the outer function. Sibling nested functions emitted before the outer body didn't have the map, so calc() calling doubler() emitted unmangled doubler instead of main__doubler.
- **Fix:** Pre-populate _nested_name_map for ALL sibling nested functions before hoisting any of them.
- **Files:** Codegen/codegen_c.py

### Bug 6: list<string> treated as plain string variable for method dispatch
- **Root cause:** The check vtype.name == "string" matched both plain strings AND list<string> variables. This caused names.add("Dave") to emit str_add() instead of the list append helper.
- **Fix 1:** Added not vtype.is_list guard to the string method check.
- **Fix 2:** Added "add" as an alias for "append" in list method dispatch.
- **Files:** Codegen/codegen_c.py

### Bug 7: Inherited method return type not found in fmt_specifier
- **Root cause:** fmt_specifier() for MethodCall only searched direct object methods, not inherited ones. Calling an inherited describe() on a Dog instance returned %d instead of %s.
- **Fix:** Changed fmt_specifier() to walk the inheritance chain when looking up method return types.
- **Files:** Codegen/codegen_c.py

## Verified Passing Tests

All 24 programs produce identical output on both Python and C targets:

- **T100** - hello world, variables, print (bool, float, string, int)
- **T101** - functions, recursion, bool return types
- **T102** - control flow: if/else, match, for, while
- **T103** - collections: arrays, lists, slices, 'in' operator
- **T104** - types: structs, nested types
- **T105** - objects: inheritance, vtable dispatch, string method returns
- **T106** - error handling: try/catch/finally, Result types
- **T107** - file I/O: string operations
- **T108** - string operations: concat, index, slice
- **T109** - math: arithmetic, map_fn, filter_fn, sort
- **T110** - concurrency: basic functions (no spawn/wait needed)
- **T111** - DR directives: @dr types = strict
- **T112** - enums, constants, module-level vars
- **T113** - nested functions, default parameters, cross-nested calls
- **T114** - context managers (with/end with)
- **T115** - pipe operator, killswitch, bitwise operators
- **M001** - basics: variables, arithmetic, augmented assignment
- **M002** - functions, recursion, multiple calls
- **M003** - control flow: if/else, match, for, while
- **M004** - collections: arrays, list<string>, for each iteration
- **M005** - objects: Animal/Dog/Cat hierarchy with vtable dispatch
- **M006** - error handling: try/catch/Result types
- **M007** - casting: int/float/bool/string conversions
- **M008** - enums, constants, match on enum values
