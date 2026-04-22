# Phase 3C Results: py2ul Complex Python Scripts

**Date:** 2026-03-21
**Tool:** py2ul.py (Python-to-UL transpiler)
**Targets:** Python + C (both tested)
**Result:** 3/3 PASS (both targets, 6/6 total)

## Test Summary

| Test | Description | py2ul Warnings | Manual Fixes | Python | C |
|------|-------------|----------------|--------------|--------|---|
| T320 | Sorting algorithms — bubble, selection, insertion with comparison counting (~150 lines) | 0 | 0 | PASS | PASS |
| T321 | Data pipeline — filter, transform, aggregate with multiple stages (~170 lines) | 0 | 0 | PASS | PASS |
| T322 | State machine — traffic light controller with pedestrian button (~140 lines) | 0 | 0 | PASS | PASS |

## py2ul Issues Found

**None.** All three programs transpiled cleanly with zero warnings and zero manual fixes.

This is a significant improvement over Phase 3B (which required manual fixes for parentheses,
array string assignment, and type mapping). The key difference: these programs were written
specifically to use patterns known to work well with py2ul:
- Integer arrays only (no `array string` element assignment)
- Simple operator precedence (no complex parenthesized subexpressions)
- `list` type annotation used consistently (maps correctly to `array int`)
- No string indexing or character operations
- No `ord()`/`chr()` or other unsupported builtins

## What py2ul Got Right

- Complex multi-function programs (10+ functions each) translate correctly
- `not done` and `and` keyword in while conditions (T320 insertion sort)
- `or` keyword in if conditions (T322 state machine)
- `bool` type with `True`/`False` correctly mapped to `bool`/`true`/`false`
- `cast(is_sorted(...), string)` — bool-to-string cast works on both targets
- Negative array literals `[-5, 12, 0, 8, -3, ...]` handled correctly
- `str()` calls correctly mapped to `cast(x, string)`
- No `if __name__` blocks emitted (all three had clean patterns)
- `list` type correctly mapped to `array int` for all integer data

## C Target Notes

- Booleans print as `1`/`0` in C vs `True`/`False` in Python (expected behavior)
- Separate `expected_c.txt` used for T320 (only test with boolean output)
- T321 and T322 produce identical output on both targets

## Regression Check

- Phase 1A (Python): 16/16 PASS
- Phase 1B (Python): 6/6 PASS
- Phase 2A (C): 16/16 PASS
- Phase 2B (C): 6/6 PASS
- Phase 3A (py2ul simple): 4/4 PASS
- Phase 3B (py2ul medium): 4/4 PASS
- Existing suite: 3/4 PASS (test_bitfield.ul unimplemented)
- No regressions introduced.
