# Real-World C Validation Report

**Date:** 2026-03-21
**Tool:** `Tools/c2ul.py` (C-to-UL transpiler using pycparser)
**Pipeline:** C source -> c2ul -> UL -> Main.py -t c -> gcc -> run -> compare

---

## Summary

Three real-world C repositories tested: antirez/sds, DaveGamble/cJSON, jgm/cmark.
Three standalone round-trip tests (C->UL->C->gcc->run) completed with exact output match.
Full-file translation of cJSON.c (3206 lines) tested for translation coverage; final gcc compile
fails due to language-level type erasure limitations.

| Test | Source | Result | Notes |
|------|--------|--------|-------|
| cmark/cmark_ctype | cmark ctype lookup table | PASS | Pure int table, exact output |
| cmark/utf8_is_space | cmark utf8proc_is_space | PASS | Pure int logic, exact output |
| cJSON/parse_hex4 | cJSON hex parser | PASS | String indexing, exact output |
| SDS/sds_ll2str | SDS integer-to-string | BLOCKED | Pointer arithmetic wall |
| cJSON full file | cJSON.c (3206 lines) | PARTIAL | UL compiled; gcc fails (type erasure) |

---

## Test 1: cmark_ctype (PASS)

**Source:** `Tests/RealWorld/C/test_cmark_ctype.c` (extracted from `cmark/src/cmark_ctype.c`)
**Function:** `cmark_ispunct(c)` - lookup table of 128 ASCII character categories

**Result:** All 128 character codes produce correct ispunct results. Exact output match.

**Bugs Fixed:**
- UL Parser nested `if` inside `else` block (Parser/parser.py:566)
- UL Parser bare `return` before `else` keyword (Parser/parser.py:806)

---

## Test 2: cmark utf8proc_is_space (PASS)

**Source:** `Tests/RealWorld/C/test_cmark_utf8_space.c` (extracted from `cmark/src/utf8.c`)
**Function:** `cmark_utf8proc_is_space(uc)` - Unicode Zs class + control chars

**Result:** 16 test cases - TAB, LF, FF, CR, SPACE, NBSP, Unicode spaces, non-space chars. Exact output match.

No new bugs needed. c2ul handled the nested `or` / `and` expression chain correctly.

---

## Test 3: cJSON parse_hex4 (PASS)

**Source:** `Tests/RealWorld/C/test_cjson_hex.c` (extracted from `cJSON/cJSON.c`)
**Function:** `parse_hex4(input)` - 4-char hex string to unsigned int

**Result:** 10 test cases (all-zeros, partial, 0xFFFF, mixed case, lowercase, invalid->0). Exact output match.

**Bugs Fixed:**

### Bug 1: c2ul mapped `char*` to `int` instead of `string`

`_map_type_specifier()` maps `char` to `"int"`. The `PtrDecl` branch checked `if inner == "char"`
which never fired because `inner` was already `"int"`. Fix: directly inspect the raw AST node.

```python
if isinstance(node, c_ast.PtrDecl):
    inner_node = node.type
    if isinstance(inner_node, c_ast.TypeDecl):
        spec = inner_node.type
        if isinstance(spec, c_ast.IdentifierType) and 'char' in spec.names:
            return "string"
    inner = self.map_type(node.type)
    return inner
```

**File:** `Tools/c2ul.py` (map_type PtrDecl branch)

### Bug 2: C codegen emitted `str_char_at(s,i)` (returning `char*`) for `int c = s[i]`

`str_char_at` returns a `char*` single-char string. Assigning to `int c` set `c` to the
pointer value, not the char code. Fix: `_string_index_as_int` flag — when LHS type is
numeric and RHS is an Index node, emit `(unsigned char)(s[i])` instead of `str_char_at`:

```python
if getattr(self, '_string_index_as_int', False):
    return f"(unsigned char)({node.target.name}[{index_expr}])"
```

**File:** `Codegen/codegen_c.py` (Index gen_expr + VarDecl + Assign handlers)

---

## Test 4: SDS sds_ll2str (BLOCKED - Pointer Arithmetic Wall)

**Source:** `Tests/RealWorld/C/test_sds_numconv.c` (extracted from `sds/sds.c`)
**Functions:** `sds_ll2str`, `sds_ull2str` - integer to decimal string conversion

**Blocker:** Both functions use C pointer arithmetic that has no UL equivalent:
- `*p++ = '0' + (v % 10)` - dereference-and-increment (write through pointer)
- `l = (int)(p - s)` - pointer subtraction to get string length
- `aux = *s; *s = *p; *p = aux` - pointer-based swap

c2ul translates pointer variables as `int`, producing integer assignments that drop the
write-to-memory semantics entirely. The UL code compiles but is semantically incorrect.

**Status:** Documented wall. UL has no mutable pointer-to-array concept. No fix attempted.

---

## Bugs Fixed This Session

| # | File | Bug | Fix |
|---|------|-----|-----|
| 1 | `Tools/c2ul.py` | `char*` params typed as `int` instead of `string` | Check raw AST PtrDecl node for `char` |
| 2 | `Codegen/codegen_c.py` | `string[i]` yields `char*` when assigned to `int` | `_string_index_as_int` flag emits `(unsigned char)(s[i])` |

---

## Bugs Fixed in Prior Sessions (cJSON.c Translation)

| # | File | Bug | Fix |
|---|------|-----|-----|
| 1 | `Parser/parser.py` | Nested `if` inside `else` block parsed incorrectly | Check line numbers for same-line else-if |
| 2 | `Parser/parser.py` | Bare `return` before `else` breaks parser | Add `else` to stop-token check |
| 3 | `Tools/c2ul.py` | C function named `print` clashed with UL keyword | Apply `_safe_name()` at function call fallthrough |
| 4 | `Tools/c2ul.py` | `switch default:` with no preceding `case` emitted stray `else` | Emit default body as standalone when `first_case=True` |
| 5 | `Tools/c2ul.py` | Initialized globals emitted as `constant` | Non-string globals become variable declarations |
| 6 | `Tools/c2ul.py` | Inline `// comment` in expression context broke UL syntax | All expression-context placeholders return plain `"0"` |
| 7 | `Tools/c2ul.py` | C stdlib names used as pointer values emitted as undefined vars | Return `"0"` for stdlib names in identifier (non-call) context |

---

## Full cJSON.c Translation (Partial)

**Source:** `cJSON/cJSON.c` (3206 lines, ~123 functions)
**c2ul output:** `cJSON/cJSON.ul` (2167 lines, 113 functions)

| Stage | Result |
|-------|--------|
| C preprocessing (gcc -E) | PASS |
| pycparser AST parse | PASS |
| c2ul translation | PASS - 2167 lines, 13 ternary warnings |
| UL compilation (-t c) | PASS |
| gcc compile of UL-generated C | FAIL - 775 errors (type erasure) |

**Root cause:** All C pointer/struct types erase to `int` in UL. Field accesses like
`item->valuestring` become `int` variable accesses, which is invalid C.

---

## Known Walls (Language-Level Gaps)

| Gap | Impact |
|-----|--------|
| Pointer arithmetic (`*p++`, `p-s`) | Buffer writes lost; length calc wrong |
| Struct/pointer type erasure | Field access fails in generated C |
| Function pointers | vtable pattern incorrect |
| C stdlib without UL equiv | sprintf, memset, tolower dropped |
| sizeof / offsetof | Not translated |

---

## Files Created/Modified

| File | Description |
|------|-------------|
| `test_cjson_hex.c` | Standalone cJSON parse_hex4 test source |
| `test_cjson_hex.ul` | c2ul output |
| `test_cjson_hex_rt.c` | UL->C compiled output |
| `test_cmark_utf8_space.c` | Standalone cmark utf8_is_space test source |
| `test_cmark_utf8_space.ul` | c2ul output |
| `test_cmark_utf8_space_rt.c` | UL->C compiled output |
| `test_sds_numconv.c` | SDS numconv test (documents pointer arithmetic wall) |
| `test_sds_numconv.ul` | c2ul output (incorrect semantics due to wall) |
| `Tools/c2ul.py` | Fixed `char*` to `string` type mapping |
| `Codegen/codegen_c.py` | Fixed `string[i]` to int char code; typedef enum; _declared_vars |
| `Parser/parser.py` | Fixed nested if-in-else; bare return before else |
