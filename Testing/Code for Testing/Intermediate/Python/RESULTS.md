# Phase 3A Results: py2ul Simple Python Scripts

**Date:** 2026-03-21
**Tool:** py2ul.py (Python-to-UL transpiler)
**Targets:** Python + C (both tested)
**Result:** 4/4 PASS (both targets)

## Test Summary

| Test | Description | py2ul Warnings | Manual Fixes | Python | C |
|------|-------------|----------------|--------------|--------|---|
| T300 | FizzBuzz — conditionals, string returns, modulo | 0 | 1 | PASS | PASS |
| T301 | Calculator — recursion, composition, integer math | 0 | 1 | PASS | PASS |
| T302 | Statistics — array operations, bubble sort, median | 0 | 2 | PASS | PASS |
| T303 | Text processing — reverse, palindrome, char count | 0 | 3 | PASS | PASS |

## py2ul Issues Found

### Issue 1: `if __name__ == "__main__"` block emitted (all tests)
- **Severity:** Always requires manual fix
- **What:** py2ul emits `if __name__ == "__main__"` / `main()` / `end if` at file scope
- **Why it fails:** UL parser only allows `function`, `object`, `type`, etc. at module level
- **Fix:** Delete the block. UL calls `main()` automatically.

### Issue 2: Bare `list` type mapped to `array string` (T302)
- **Severity:** Wrong type inference
- **What:** Python `def foo(data: list)` maps to `array string data` instead of `array int`
- **Why:** py2ul's `map_type` maps bare `list`/`List` to `array string` as default
- **Fix:** Manually change `array string` to `array int` for integer data.
- **Recommendation:** py2ul should infer element type from usage or default to `int`.

### Issue 3: `ord()` and `chr()` not available in UL (T303)
- **Severity:** Requires code rewrite
- **What:** Python's `ord(ch)` and `chr(n)` have no UL equivalent
- **Why:** UL doesn't expose character ordinal operations
- **Fix:** Removed `to_upper()` function and its test calls. No workaround possible
  without UL stdlib support for character codes.

### Issue 4: `s.len()` and `s[i]` for strings (T303)
- **Severity:** May work on Python target but not C target
- **What:** py2ul translates `len(s)` to `s.len()` and `s[i]` stays as-is
- **Why:** UL string stdlib uses `strlen()` and `str_char_at()` FFI functions
- **Fix:** Changed to `import "string"` FFI functions and explicit calls.
  However, `s.len()` and `s[i]` DO compile — they work on the Python target.
  For maximum portability, use the stdlib FFI.

## What py2ul Got Right

- Function signatures with type annotations translate perfectly
- `while` loops, `if/else/elif` chains translate correctly
- `str()` calls correctly map to `cast(x, string)`
- String concatenation with `+` works
- Recursive functions (factorial, gcd) work
- Modulo operator `%` preserves correctly
- Integer division `//` maps to `/` (UL integer division)
- Variable declarations with type annotations translate correctly
- Docstrings translate to `//` comments

## Regression Check

- Phase 1A (Python): 16/16 PASS
- Phase 1B (Python): 6/6 PASS
- Phase 2A (C): 16/16 PASS
- Phase 2B (C): 6/6 PASS
- Existing suite: 3/4 PASS (test_bitfield.ul unimplemented)
- No regressions introduced.
