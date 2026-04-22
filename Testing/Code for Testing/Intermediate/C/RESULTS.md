# Phase 2B Results: UL -> C Multi-File Tests

**Date:** 2026-03-21
**Target:** C (compiled with gcc, MinGW-W64 15.2.0)
**Result:** 6/6 PASS

## Test Summary

| Test | Description | Status |
|------|-------------|--------|
| T220 | Two-file import — utility functions (clamp, abs, max, repeat) | PASS |
| T221 | Three-file import — geometry, convert, format modules | PASS |
| T222 | Stdlib FFI (string) + array/list builtins (C-adapted) | PASS |
| T223 | CSV data processing with string stdlib | PASS |
| T224 | CLI task manager — 3 support files (C-adapted) | PASS |
| T225 | Dungeon crawler game — 5 support files | PASS |

## C-Target Adaptations

### T222 (stdlib): Direct print of FFI string return values
- **Bug:** `print str_char_at(msg, 0)` generates `printf("%d\n", str_char_at(...))` — uses `%d`
  format specifier instead of `%s` for `char*` return values from stdlib FFI functions.
- **Workaround:** Store FFI result in a variable first, then print the variable.
  `string ch = str_char_at(msg, 0)` then `print ch` works correctly.

### T224 (CLI tool): cast() + string literal in concatenation
- **Bug:** `cast(x, string) + "literal"` generates `(_cast_buf + "literal")` which is C
  pointer arithmetic, not string concatenation. Should use `__ul_strcat()`.
- **Scope:** Affects any expression where `cast()` result is the left operand of `+`.
  When a string literal is the left operand (`"literal" + cast(x, string)`), it works.
- **Workaround:** Store `cast()` result in a variable first:
  `string s = cast(x, string)` then `s + "literal"` works via `__ul_strcat()`.
- **Additional fix:** Moved formatting logic into main to avoid cast-in-return in
  imported functions; imported modules remain stateless data/search functions.

## New C-Specific Compiler Bugs Found

4. **`printf` format specifier wrong for FFI string returns** — When a stdlib FFI function
   returns `char*` (like `str_char_at`), `print` generates `printf("%d\n", ...)` instead of
   `printf("%s\n", ...)`. Works correctly when result is assigned to a `string` variable first.

5. **`cast(x, string) + literal` generates pointer arithmetic** — `cast(int_var, string)`
   produces a `static char _cast_buf[64]` filled via `snprintf`. When this is used directly
   in `+`, the codegen emits `(_cast_buf + "str")` instead of `__ul_strcat(_cast_buf, "str")`.
   Only happens when `_cast_buf` is the left operand; `"str" + _cast_buf` wraps correctly.

## Regression Check

- Phase 1A (Python): 16/16 PASS
- Phase 1B (Python): 6/6 PASS
- Phase 2A (C single-file): 16/16 PASS
- Existing suite: 3/4 PASS (test_bitfield.ul unimplemented)
- No regressions introduced.
