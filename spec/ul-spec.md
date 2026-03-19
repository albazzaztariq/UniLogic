# UniLogic Language Specification

**Version:** 1.0-draft
**Date:** 2026-03-18

---

## 1. Lexical Structure

### 1.1 Character Set

UL source files are UTF-8 encoded. All keywords, operators, and identifiers use ASCII characters. String literals may contain any valid UTF-8 sequence.

### 1.2 Whitespace

Spaces, tabs, carriage returns, and newlines are insignificant except inside string literals. There are no indentation requirements.

### 1.3 Comments

Line comments begin with `//` and extend to end of line. There are no block comments.

```
// this is a comment
int x = 5  // inline comment
```

### 1.4 Keywords

56 reserved words. An identifier matching a keyword is always a keyword token.

**Control flow:** `function`, `end`, `returns`, `return`, `if`, `else`, `while`, `do`, `for`, `each`, `in`, `match`, `iterate`, `default`, `escape`, `continue`

**Declarations:** `type`, `inherits`, `new`, `fixed`, `constant`, `import`, `from`, `export`

**I/O:** `print`, `prompt`

**Concurrency:** `parallel`, `killswitch`, `teleport`, `portal`

**Hints:** `nocache`, `yield`, `yields`, `inline`, `pack`

**Literals:** `true`, `false`, `empty`

**Logical:** `and`, `or`, `not`, `equals`

**Bitwise:** `both1`, `both0`, `either1`, `delta`, `bitflip`, `negate`, `left`, `right`

**Memory:** `address`, `deref`, `memmove`, `memcopy`, `memset`, `memtake`, `memgive`

**Built-ins:** `size`, `change`, `absval`

**Types:** `int`, `integer`, `float`, `double`, `string`, `bool`, `none`, `complex`, `int8`, `int16`, `int32`, `int64`, `uint8`, `uint16`, `uint32`, `uint64`, `array`, `list`, `map`, `arena`, `file`

**Result:** `ok`, `error`, `some`

### 1.5 Operators

Listed by precedence (highest first):

| Precedence | Operators | Associativity |
|-----------|-----------|---------------|
| 1 (highest) | `()` `[]` `.` `->` `++` `--` | left-to-right |
| 2 | `-` (unary) `not` `address` `deref` | right-to-left |
| 3 | `*` `/` `%` | left-to-right |
| 4 | `+` `-` | left-to-right |
| 5 | `<` `>` `<=` `>=` | left-to-right |
| 6 | `==` `!=` | left-to-right |
| 7 | `and` | left-to-right |
| 8 | `or` | left-to-right |
| 9 (lowest) | `=` `+=` `-=` `*=` `/=` `%=` `left=` `right=` | right-to-left |

Additional operators: `?` (result propagation), `|>` (pipe), `|` (result type separator).

### 1.6 Literals

**Integer:** Decimal digits. `0x` or `0X` prefix for hexadecimal.
```
42
0xFF
0xDEAD0000
```

**Float:** Decimal digits with `.` and optional exponent.
```
3.14
1.0e-5
2.5E+3
```

**String:** Double-quoted. Escape sequences: `\n` `\t` `\\` `\"`.
```
"hello world"
"line one\nline two"
"tab\there"
```

**Boolean:** `true` or `false`.

**Empty:** `empty` (null/none value).

**Array:** Square-bracket delimited, comma-separated.
```
[1, 2, 3]
["a", "b", "c"]
```

---

## 2. Types

### 2.1 Primitive Types

| Type | Description | Size |
|------|------------|------|
| `int` | Signed 32-bit integer | 4 bytes |
| `int8` | Signed 8-bit integer | 1 byte |
| `int16` | Signed 16-bit integer | 2 bytes |
| `int32` | Signed 32-bit integer | 4 bytes |
| `int64` | Signed 64-bit integer | 8 bytes |
| `uint8` | Unsigned 8-bit integer | 1 byte |
| `uint16` | Unsigned 16-bit integer | 2 bytes |
| `uint32` | Unsigned 32-bit integer | 4 bytes |
| `uint64` | Unsigned 64-bit integer | 8 bytes |
| `float` | IEEE 754 32-bit | 4 bytes |
| `double` | IEEE 754 64-bit | 8 bytes |
| `string` | UTF-8 string (heap-allocated) | pointer |
| `bool` | Boolean (`true`/`false`) | 1 byte |
| `none` | Void/absent value | 0 bytes |
| `complex` | Complex number (reserved) | 16 bytes |

### 2.2 Compound Types

**Array:** `array T` declares a fixed-size array of type T. Size inferred from initializer or declared explicitly.
```
array int nums = [1, 2, 3]
```

**Map:** `map` (reserved). Key-value container.

**Pointer:** `<T>` wraps a type in angle brackets to indicate a pointer.
```
int <ptr> = address x
```

### 2.3 User-Defined Types

**Type declaration:** Value semantics. Fields only, no methods.
```
type Point
  float x
  float y
end type
```

**Inheritance:** Single inheritance via `inherits`.
```
type Point3D inherits Point
  float z
end type
```

### 2.4 Result Types

A function may return `T|error` to indicate it can succeed with type T or fail with an error.
```
function divide(int a, int b) returns int|error
  if b == 0
    return error "division by zero"
  end if
  return ok a / b
end function
```

---

## 3. Declarations

### 3.1 Function

```
function name(type param, type param, ...) [returns type]
  body
end function
```

If `returns` is omitted, the function returns `none`. The compiler inserts `return 0` at the OS level for `main` functions with no return type.

### 3.2 Variable

```
type name = expr          // initialized
type name                 // zero-initialized (0, 0.0, "", false, empty)
fixed type name = expr    // constant (immutable after initialization)
```

### 3.3 Type Declaration

```
type Name [inherits Parent]
  type field_name
  type field_name
  ...
end type
```

### 3.4 Foreign Import

```
import "library" function name(type param, ...) [returns type]
```

Supports variadic parameters via `...`:
```
import "stdio" function printf(string fmt, ...) returns int
```

### 3.5 Local Import

```
import "file.ul" function name(type param, ...) [returns type]
```

Imports a function declared in another UL source file.

---

## 4. Expressions

### 4.1 Arithmetic

`+`, `-`, `*`, `/`, `%`. Integer division truncates toward zero (C semantics). Division by zero is a runtime error.

### 4.2 Comparison

`==`, `!=`, `<`, `>`, `<=`, `>=`. Result is `bool`.

### 4.3 Logical

`and`, `or` (short-circuit), `not` (unary). Operands are truthy/falsy.

### 4.4 Cast

```
cast(expr, type)
```

Converts the value of `expr` to `type`. Float-to-int truncates. Int-to-float promotes. Any-to-string converts to string representation. Any-to-bool converts via truthiness.

### 4.5 Array Literal

```
[expr, expr, ...]
```

### 4.6 Index

```
expr[expr]
```

Out-of-bounds access is a runtime error when `safety = checked`.

### 4.7 Field Access

```
expr.field
```

### 4.8 Function Call

```
name(expr, expr, ...)
```

### 4.9 Result Propagation

```
expr?
```

If `expr` evaluates to `error`, the enclosing function immediately returns that error. If `ok`, the value is unwrapped.

### 4.10 Post-Increment / Post-Decrement

```
x++
x--
```

Returns the value before modification.

### 4.11 Unary

```
-expr          // arithmetic negation
not expr       // logical negation
address expr   // take memory address (returns pointer)
deref expr     // dereference pointer
```

---

## 5. Statements

### 5.1 Assignment

```
name = expr
name += expr
name -= expr
name *= expr
name /= expr
name %= expr
name left= expr     // bitwise left shift and assign
name right= expr    // bitwise right shift and assign
```

### 5.2 If / Else

```
if condition
  body
else
  body
end if
```

The `else` clause is optional.

### 5.3 While

```
while condition
  body
end while
```

### 5.4 For Each

```
for each var in iterable
  body
end for
```

The loop variable `var` is implicitly typed from the iterable's element type.

### 5.5 Match

```
match subject
  value1
    body
  value2
    body
  default
    body
end match
```

Case values must be literals (integer, float, string, bool, `empty`) or negative literals. The `default` case is optional.

### 5.6 Return

```
return expr         // return a value
return ok expr      // return success (result type)
return error "msg"  // return error (result type)
return              // return none (void functions)
```

### 5.7 Print

```
print expr
```

Prints the value followed by a newline. Booleans print as `0`/`1`. Floats print with `%f` format. Empty prints as `empty`.

### 5.8 Escape / Continue

```
escape      // break out of nearest loop
continue    // skip to next iteration
```

---

## 6. DR Directives

Declared at file level before any function or type declaration. Each directive is prefixed with `@dr`.

```
@dr memory = gc
@dr safety = checked
```

| Setting | Valid Values | Default |
|---------|-------------|---------|
| `memory` | `gc`, `manual`, `refcount`, `arena` | `gc` |
| `safety` | `checked`, `unchecked` | `checked` |
| `types` | `strict`, `dynamic` | `strict` |
| `int_width` | `32`, `64`, `platform` | `32` |
| `concurrency` | `threaded`, `parallel`, `async`, `cooperative` | `threaded` |

The compiler enforces that files with conflicting DR settings that call each other produce a warning at the call site.

---

## 7. Annotations

| Annotation | Scope | Description |
|-----------|-------|-------------|
| `@dr key = value` | File | Dynamic Runtime directive |
| `@norm N` | File or function | Set normalization level (0-3) |
| `@deprecated("msg")` | Function | Mark function as deprecated |
| `@fuse` | Block | Hint to fuse operations in this block |
| `@prefetch(distance=N)` | Loop or data structure | Insert prefetch instructions |
| `@layout = value` | Type | Control memory layout (SoA, AoS) |
| `@precision = value` | File or function | Set floating-point precision mode |
| `@sparsity = value` | Type | Declare tensor sparsity pattern |

---

## 8. Built-in Functions

| Function | Signature | Description |
|----------|-----------|-------------|
| `print` | `print expr` | Print value to stdout with newline |
| `prompt` | `prompt(string msg) returns string` | Read line from stdin |
| `absval` | `absval(numeric x) returns numeric` | Absolute value |
| `size` | `size(T) returns int` | Size of type in bytes |
| `cast` | `cast(expr, type) returns type` | Type conversion |
| `exit` | `exit(int code)` | Terminate program with exit code |
| `typeof` | `typeof(expr) returns string` | Runtime type name |
| `memtake` | `memtake(int bytes) returns <none>` | Allocate heap memory |
| `memgive` | `memgive(<none> ptr)` | Free heap memory |
| `memcopy` | `memcopy(<none> dst, <none> src, int n)` | Copy n bytes (no overlap) |
| `memmove` | `memmove(<none> dst, <none> src, int n)` | Copy n bytes (overlap safe) |
| `memset` | `memset(<none> ptr, int val, int n)` | Fill n bytes with val |
| `address` | `address expr` | Take memory address (unary operator) |
| `deref` | `deref expr` | Dereference pointer (unary operator) |

---

## 9. Standard Library

### 9.1 Math

`sqrt(x)`, `pow(x, y)`, `exp(x)`, `log(x)`, `log2(x)`, `log10(x)`, `floor(x)`, `ceil(x)`, `round(x)`, `min(a, b)`, `max(a, b)`, `clamp(x, lo, hi)`, `lerp(a, b, t)`.

### 9.2 String

`length(s)`, `substring(s, start, len)`, `index_of(s, sub)`, `replace(s, old, new)`, `split(s, delim)`, `join(arr, delim)`, `trim(s)`, `upper(s)`, `lower(s)`, `starts_with(s, prefix)`, `ends_with(s, suffix)`, `contains(s, sub)`.

### 9.3 Array

`length(arr)`, `push(arr, val)`, `pop(arr)`, `insert(arr, idx, val)`, `remove(arr, idx)`, `slice(arr, start, end)`, `reverse(arr)`, `sort(arr)`, `find(arr, val)`, `map(arr, fn)`, `filter(arr, fn)`, `reduce(arr, fn, init)`.

### 9.4 Map

`keys(m)`, `values(m)`, `has(m, key)`, `get(m, key)`, `set(m, key, val)`, `delete(m, key)`, `size(m)`.

### 9.5 File

`open(path [, mode])`, `read(f)`, `write(f, data)`, `close(f)`, `exists(path)`, `readlines(f)`.

### 9.6 JSON

`json_parse(s)`, `json_stringify(val)`.

---

## 10. Grammar (EBNF)

```ebnf
program        = { declaration } ;

declaration    = dr_directive
               | function_decl
               | type_decl
               | foreign_import
               | local_import ;

dr_directive   = "@dr" IDENT "=" IDENT ;

function_decl  = "function" IDENT "(" [ param_list ] ")" [ "returns" type ] body "end" "function" ;

type_decl      = "type" IDENT [ "inherits" IDENT ] { field } "end" "type" ;

foreign_import = "import" STRING "function" IDENT "(" [ param_list ] ")" [ "returns" type ] ;

local_import   = "import" STRING "function" IDENT "(" [ param_list ] ")" [ "returns" type ] ;

param_list     = param { "," param } ;
param          = type IDENT ;

field          = type IDENT ;

type           = [ "<" ] type_name [ ">" ]
               | "array" type_name ;

type_name      = "int" | "int8" | "int16" | "int32" | "int64"
               | "uint8" | "uint16" | "uint32" | "uint64"
               | "float" | "double" | "string" | "bool" | "none"
               | "complex" | "list" | "map" | "arena" | "file"
               | IDENT ;

body           = { statement } ;

statement      = var_decl
               | assignment
               | if_stmt
               | while_stmt
               | for_stmt
               | match_stmt
               | return_stmt
               | print_stmt
               | "escape"
               | "continue"
               | expr_stmt ;

var_decl       = [ "fixed" ] type IDENT [ "=" expr ] ;

assignment     = expr assign_op expr ;
assign_op      = "=" | "+=" | "-=" | "*=" | "/=" | "%=" | "left=" | "right=" ;

if_stmt        = "if" expr body [ "else" body ] "end" "if" ;

while_stmt     = "while" expr body "end" "while" ;

for_stmt       = "for" "each" IDENT "in" expr body "end" "for" ;

match_stmt     = "match" expr { match_case } [ default_case ] "end" "match" ;
match_case     = expr body ;
default_case   = "default" body ;

return_stmt    = "return" [ expr ]
               | "return" "ok" expr
               | "return" "error" expr ;

print_stmt     = "print" expr ;

expr_stmt      = expr ;

expr           = or_expr ;

or_expr        = and_expr { "or" and_expr } ;
and_expr       = not_expr { "and" not_expr } ;
not_expr       = "not" not_expr | comparison ;
comparison     = addition { ( "==" | "!=" | "<" | ">" | "<=" | ">=" ) addition } ;
addition       = multiplication { ( "+" | "-" ) multiplication } ;
multiplication = unary { ( "*" | "/" | "%" ) unary } ;
unary          = ( "-" | "address" | "deref" ) unary | postfix ;

postfix        = primary { call | index | field_access | cast_expr | "++" | "--" } ;
call           = "(" [ arg_list ] ")" ;
index          = "[" expr "]" ;
field_access   = "." IDENT ;
cast_expr      = "->" type ;

arg_list       = expr { "," expr } ;

primary        = INT_LITERAL
               | HEX_LITERAL
               | FLOAT_LITERAL
               | STRING_LITERAL
               | "true" | "false" | "empty"
               | IDENT
               | "change"
               | "[" [ arg_list ] "]"
               | "(" expr ")" ;

INT_LITERAL    = digit { digit } ;
HEX_LITERAL    = "0" ( "x" | "X" ) hex_digit { hex_digit } ;
FLOAT_LITERAL  = digit { digit } "." digit { digit } [ ( "e" | "E" ) [ "+" | "-" ] digit { digit } ] ;
STRING_LITERAL = '"' { char | escape } '"' ;
IDENT          = ( letter | "_" ) { letter | digit | "_" } ;

digit          = "0" ... "9" ;
hex_digit      = digit | "a" ... "f" | "A" ... "F" ;
letter         = "a" ... "z" | "A" ... "Z" ;
char           = any UTF-8 character except '"' and '\' and newline ;
escape         = '\' ( 'n' | 't' | '"' | '\' ) ;
```

---

## 11. Compilation Targets

The compiler emits code for the following targets:

| Target | Flag | Output | Description |
|--------|------|--------|-------------|
| C | `-t c` | `.c` | C99 source. Compile with gcc/clang for native binary. |
| Python | `-t python` | `.py` | Python 3.10+ source with dataclasses. |
| JavaScript | `-t js` | `.js` | ES6 source for browser or Node.js. |
| LLVM IR | `-t llvm` | `.ll` | LLVM IR text. Compile with clang for native binary. |
| WebAssembly | `-t wasm` | `.wasm` | Browser-portable bytecode. |

---

## 12. Semantics Notes

- Integer division truncates toward zero (C99 behavior).
- Short-circuit evaluation: `and` does not evaluate right operand if left is false. `or` does not evaluate right operand if left is true.
- Variable declarations in inner scopes shadow outer declarations.
- Function parameters are passed by value. Pointers enable pass-by-reference.
- The `?` operator in a non-result function prints the error to stderr and exits with code 1.
- `fixed` variables are immutable after initialization. Attempting to assign to a `fixed` variable is a compile error.
- `empty` is falsy. `0`, `0.0`, `""`, and `false` are falsy. All other values are truthy.
