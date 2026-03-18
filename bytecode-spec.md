# UniLogic Bytecode Format Specification

**Version:** 0.1
**Status:** Draft
**Date:** 2026-03-18

---

## 1. Design Decision: Stack-Based

The UL bytecode uses a **stack-based** architecture. Each instruction pushes to or pops from an operand stack rather than naming registers. This eliminates register allocation from the compiler, produces smaller bytecode (no register operands in most instructions), and simplifies the VM implementation to under 500 lines for a conforming interpreter. The tradeoff — more instructions per operation than a register machine — is acceptable because the VM's inner loop is a simple switch dispatch, and the reduced compiler complexity means faster iteration on the toolchain. The JVM and CPython both validate this approach at scale.

---

## 2. File Format

A `.ulb` file (UniLogic Bytecode) has the following layout:

```
┌──────────────────────────────────┐
│  Header              (16 bytes)  │
├──────────────────────────────────┤
│  Constant Pool       (variable)  │
├──────────────────────────────────┤
│  Function Table      (variable)  │
├──────────────────────────────────┤
│  Instruction Blocks  (variable)  │
└──────────────────────────────────┘
```

### 2.1 Header

| Offset | Size | Field | Value |
|--------|------|-------|-------|
| 0 | 4 | Magic bytes | `0x554C4243` (`ULBC` in ASCII) |
| 4 | 2 | Major version | `0x0001` |
| 6 | 2 | Minor version | `0x0000` |
| 8 | 4 | Constant pool entry count | u32, little-endian |
| 12 | 4 | Function table entry count | u32, little-endian |

All multi-byte integers are **little-endian** throughout the file.

### 2.2 Constant Pool

Immediately follows the header. Contains `constant_count` entries, each prefixed by a 1-byte type tag:

| Tag | Type | Encoding |
|-----|------|----------|
| `0x01` | int | 8 bytes, signed 64-bit LE |
| `0x02` | float | 8 bytes, IEEE 754 double LE |
| `0x03` | string | 4 bytes length (u32 LE) + N bytes UTF-8, no null terminator |
| `0x04` | bool | 1 byte (`0x00` = false, `0x01` = true) |
| `0x05` | empty | 0 bytes (null/none literal) |

Constants are referenced by **zero-based index** in instructions.

### 2.3 Function Table

Contains `function_count` entries, each:

| Offset | Size | Field |
|--------|------|-------|
| 0 | 4 | Name length (u32 LE) |
| 4 | N | Name (UTF-8 bytes) |
| 4+N | 2 | Parameter count (u16 LE) |
| 6+N | 2 | Local variable count (u16 LE, includes params) |
| 8+N | 4 | Instruction offset (u32 LE, byte offset into instruction block) |
| 12+N | 4 | Instruction length (u32 LE, byte count) |

Functions are referenced by **zero-based index**. Function 0 is not required to be `main`; the entry point is specified by the VM invocation (default: function named `main`).

### 2.4 Instruction Block

A single contiguous byte stream. Each function's instructions occupy a slice identified by its offset and length from the function table. Instructions are variable-width: 1-byte opcode optionally followed by operands.

---

## 3. Type Encoding on the Stack

The VM operand stack holds tagged values. Each stack slot is a discriminated union:

| Tag | Type | Stack representation |
|-----|------|---------------------|
| `0x01` | int | signed 64-bit integer |
| `0x02` | float | IEEE 754 64-bit double |
| `0x03` | string | pointer to heap-allocated UTF-8 string |
| `0x04` | bool | 0 or 1 |
| `0x05` | empty | null/none sentinel |

Type tags are carried at runtime. Type errors (e.g. adding a string to an int) produce a runtime error, not undefined behavior.

---

## 4. Instruction Set

All opcodes are 1 byte. Operands follow the opcode where specified.

### 4.1 Constants and Variables

| Opcode | Hex | Operands | Stack effect | Description |
|--------|-----|----------|-------------|-------------|
| `LOAD_CONST` | `0x01` | u16 index | → value | Push constant pool entry `index` onto stack |
| `LOAD_LOCAL` | `0x02` | u16 slot | → value | Push local variable `slot` onto stack |
| `STORE_LOCAL` | `0x03` | u16 slot | value → | Pop stack top into local variable `slot` |
| `LOAD_GLOBAL` | `0x04` | u16 index | → value | Push global variable `index` onto stack |
| `STORE_GLOBAL` | `0x05` | u16 index | value → | Pop stack top into global variable `index` |

Local variable slots are numbered 0..N-1 where slots 0..param_count-1 hold function parameters (populated by the caller before `CALL`).

### 4.2 Arithmetic and Logic

| Opcode | Hex | Operands | Stack effect | Description |
|--------|-----|----------|-------------|-------------|
| `ADD` | `0x10` | — | b, a → result | `a + b`. Int+int→int, float+float→float, int+float→float, string+string→concat |
| `SUB` | `0x11` | — | b, a → result | `a - b`. Numeric only |
| `MUL` | `0x12` | — | b, a → result | `a * b`. Numeric only |
| `DIV` | `0x13` | — | b, a → result | `a / b`. Int/int→truncated int, float involved→float. Division by zero = runtime error |
| `MOD` | `0x14` | — | b, a → result | `a % b`. Numeric only. Zero divisor = runtime error |
| `NEG` | `0x15` | — | a → result | `-a`. Numeric only |
| `NOT` | `0x16` | — | a → result | Logical not. Truthy→false, falsy→true |
| `AND` | `0x17` | — | b, a → result | Logical and (not short-circuit in bytecode; compiler emits jumps for short-circuit) |
| `OR` | `0x18` | — | b, a → result | Logical or |

### 4.3 Comparison

| Opcode | Hex | Operands | Stack effect | Description |
|--------|-----|----------|-------------|-------------|
| `CMP_EQ` | `0x20` | — | b, a → bool | `a == b` |
| `CMP_NE` | `0x21` | — | b, a → bool | `a != b` |
| `CMP_LT` | `0x22` | — | b, a → bool | `a < b` |
| `CMP_GT` | `0x23` | — | b, a → bool | `a > b` |
| `CMP_LE` | `0x24` | — | b, a → bool | `a <= b` |
| `CMP_GE` | `0x25` | — | b, a → bool | `a >= b` |

### 4.4 Control Flow

| Opcode | Hex | Operands | Stack effect | Description |
|--------|-----|----------|-------------|-------------|
| `JUMP` | `0x30` | i16 offset | — | Unconditional jump. PC += offset (signed, relative to instruction after operand) |
| `JUMP_IF_FALSE` | `0x31` | i16 offset | cond → | Pop top; if falsy, PC += offset |
| `JUMP_IF_TRUE` | `0x32` | i16 offset | cond → | Pop top; if truthy, PC += offset |

Jump offsets are **signed 16-bit**, relative to the byte immediately after the operand. This gives a range of ±32KB per jump. For larger functions, the compiler must use jump chains.

### 4.5 Functions

| Opcode | Hex | Operands | Stack effect | Description |
|--------|-----|----------|-------------|-------------|
| `CALL` | `0x40` | u16 func_index, u8 arg_count | arg_count values → return_value | Pop `arg_count` arguments (top = last arg), invoke function `func_index`, push return value |
| `RETURN` | `0x41` | — | value → | Pop return value, return to caller. If function is void, push `empty` before executing |
| `RETURN_NONE` | `0x42` | — | — | Return `empty` (void return shorthand) |

The VM maintains a **call stack** of frames. Each frame stores: return address (function index + PC), local variable array, and operand stack base pointer. On `CALL`, a new frame is pushed. On `RETURN`, the frame is popped and the return value is pushed onto the caller's operand stack.

### 4.6 Built-ins

| Opcode | Hex | Operands | Stack effect | Description |
|--------|-----|----------|-------------|-------------|
| `PRINT` | `0x50` | — | value → | Pop and print to stdout. Bool prints as `0`/`1`, float as `%f`, empty as `empty` |
| `POP` | `0x51` | — | value → | Discard stack top |
| `DUP` | `0x52` | — | a → a, a | Duplicate stack top |

### 4.7 Array Operations

| Opcode | Hex | Operands | Stack effect | Description |
|--------|-----|----------|-------------|-------------|
| `NEW_ARRAY` | `0x60` | u16 count | count values → array | Pop `count` values, create array (first popped = last element) |
| `INDEX_GET` | `0x61` | — | index, array → value | Pop index and array, push `array[index]`. Out of bounds = runtime error |
| `INDEX_SET` | `0x62` | — | value, index, array → | Set `array[index] = value` |
| `LENGTH` | `0x63` | — | array_or_string → int | Push length |

### 4.8 Type Operations

| Opcode | Hex | Operands | Stack effect | Description |
|--------|-----|----------|-------------|-------------|
| `CAST_INT` | `0x70` | — | value → int | Convert top to int. Float truncates, string parses, bool→0/1 |
| `CAST_FLOAT` | `0x71` | — | value → float | Convert top to float |
| `CAST_STRING` | `0x72` | — | value → string | Convert top to string representation |
| `CAST_BOOL` | `0x73` | — | value → bool | Truthy/falsy conversion |

---

## 5. Execution Model

1. The VM loads the `.ulb` file, validates the magic bytes and version, and parses the constant pool and function table into memory.
2. The VM locates the entry-point function (default: `main`) by name in the function table.
3. A root call frame is created with locals allocated to `local_count` slots, all initialized to `empty`.
4. The PC (program counter) is set to the function's instruction offset.
5. The VM enters the fetch-decode-execute loop:
   - Read opcode byte at PC, advance PC.
   - Read operands (if any), advance PC.
   - Execute the operation.
   - Repeat until `RETURN` with no caller frame (program exits).
6. The exit code is the integer value of the return value from `main`, or 0 if `main` returns `empty`.

### 5.1 Operand Stack

Each call frame has its own operand stack. Maximum stack depth per function is bounded and can be computed statically by the compiler (not stored in the file; the VM may use a fixed-size stack per frame or grow dynamically).

### 5.2 Error Handling

Runtime errors (division by zero, out-of-bounds index, type mismatch in arithmetic, stack underflow) halt execution with an error message including the function name and instruction offset. There are no exceptions or try/catch in v0.1.

---

## 6. Example

UL source:

```
function main() returns int
  int x = 3
  int y = 4
  print x + y
  return 0
end function
```

Bytecode (function `main`, 0 params, 2 locals):

```
LOAD_CONST  0      ; push 3 (constant pool index 0)
STORE_LOCAL 0      ; x = 3
LOAD_CONST  1      ; push 4 (constant pool index 1)
STORE_LOCAL 1      ; y = 4
LOAD_LOCAL  0      ; push x
LOAD_LOCAL  1      ; push y
ADD                ; x + y → 7
PRINT              ; print 7
LOAD_CONST  2      ; push 0 (constant pool index 2)
RETURN             ; return 0
```

Constant pool: `[int 3, int 4, int 0]`

Hex encoding of instructions (17 bytes):

```
01 00 00   ; LOAD_CONST 0
03 00 00   ; STORE_LOCAL 0
01 00 01   ; LOAD_CONST 1
03 00 01   ; STORE_LOCAL 1
02 00 00   ; LOAD_LOCAL 0
02 00 01   ; LOAD_LOCAL 1
10         ; ADD
50         ; PRINT
01 00 02   ; LOAD_CONST 2
41         ; RETURN
```

---

## 7. Reserved Opcodes

The range `0xE0–0xFF` is reserved for future use (closures, coroutines, match dispatch, DR enforcement, memory operations). Encountering a reserved opcode halts execution with an "unknown opcode" error.

---

## 8. Versioning

The major version increments on breaking changes to the instruction encoding or file format. The minor version increments on backward-compatible additions (new opcodes in unused ranges). A VM must reject files with a major version it does not support. A VM should accept files with a minor version greater than its own, ignoring unknown opcodes only if they appear in unreachable code paths.
