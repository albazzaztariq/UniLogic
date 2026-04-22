# codegen_bytecode.py — UniLogic → ULB bytecode compiler
# Produces .ulb binary files for the ulvm C++ runtime.
# Part of the XPile compiler toolchain.

import struct
from ast_nodes import *

# ── Opcodes ──────────────────────────────────────────────────────────────────

OP_MOV            = 0x01
OP_LOAD_CONST     = 0x02
OP_ADD            = 0x03
OP_SUB            = 0x04
OP_MUL            = 0x05
OP_DIV            = 0x06
OP_CMP_LT         = 0x07
OP_JUMP_IF_FALSE  = 0x08
OP_CALL           = 0x09
OP_RETURN         = 0x0A
OP_PRINT          = 0x0B

OP_MOD            = 0x10
OP_NEG            = 0x11
OP_NOT            = 0x12
OP_AND            = 0x13
OP_OR             = 0x14
OP_ADD_IMM        = 0x15
OP_SUB_IMM        = 0x16

OP_CMP_EQ         = 0x20
OP_CMP_NE         = 0x21
OP_CMP_GT         = 0x22
OP_CMP_LE         = 0x23
OP_CMP_GE         = 0x24

OP_JUMP           = 0x30
OP_JUMP_IF_TRUE   = 0x31
OP_RETURN_NONE    = 0x32

OP_LOAD_GLOBAL    = 0x40
OP_STORE_GLOBAL   = 0x41

OP_NEW_ARRAY      = 0x60
OP_INDEX_GET      = 0x61
OP_INDEX_SET      = 0x62
OP_LENGTH         = 0x63

OP_CAST_INT       = 0x70
OP_CAST_FLOAT     = 0x71
OP_CAST_STRING    = 0x72
OP_CAST_BOOL      = 0x73

OP_INC_REG        = 0xA7
OP_DEC_REG        = 0xA8

# Concurrency opcodes (sequential fallback in VM)
OP_SPAWN          = 0xD0   # Form A: rA=ret, rB=func_idx, rC=arg_count — sync call fallback
OP_AWAIT          = 0xD1   # Form A: rA=dst, rB=handle_reg — NOP, value already there
OP_YIELD          = 0xD2   # Form A: no-op (cooperative scheduling placeholder)
OP_LOCK           = 0xD3   # Form A: rA=lock_id — no-op (single-threaded)
OP_UNLOCK         = 0xD4   # Form A: rA=lock_id — no-op (single-threaded)

OP_NOP            = 0xFF

OP_CALL_BUILTIN   = 0x90   # Form A: rA=first_arg/ret, rB=builtin_id, rC=arg_count

# Result type opcodes
OP_MAKE_OK        = 0xB0   # Form A: rA=dst(array), rB=value_reg
OP_MAKE_ERROR     = 0xB1   # Form A: rA=dst(array), rB=msg_reg
OP_CHECK_RESULT   = 0xB2   # Form A: rA=tag_dst, rB=val_dst, rC=result_reg
OP_JUMP_IF_ERROR  = 0xB3   # Form B: rA=tag_reg, imm16=offset
OP_ERROR_HALT     = 0xB4   # Form A: rA=error_msg_reg — print to stderr and exit(1)

# Builtin function IDs dispatched by the VM
BUILTIN_TABLE = {
    'str_len': 0, 'str_equals': 1, 'str_contains': 2,
    'str_starts_with': 3, 'str_ends_with': 4,
    'str_upper': 5, 'str_lower': 6, 'str_trim': 7, 'str_concat': 8,
    'strlen': 9, 'strstr': 10,
    'array_get': 20, 'array_set': 21, 'array_contains': 22,
    'array_index_of': 23, 'array_sum': 24, 'array_min': 25,
    'array_max': 26, 'array_reverse': 27, 'array_sort': 28,
    'array_count': 29,
    'absval': 40,
    'sqrt': 41, 'pow': 42, 'fmod': 43,
    'math_sqrt': 44, 'math_pow': 45, 'math_abs': 46,
    'math_floor': 47, 'math_ceil': 48,
    'math_min': 49, 'math_max': 50,
    'math_pi': 51, 'math_e': 52,
    'str_char_at': 53, 'str_substr': 54, 'str_index_of': 55,
}

# Method name → builtin function name
STRING_METHODS = {
    'len': 'str_len', 'contains': 'str_contains',
    'starts_with': 'str_starts_with', 'ends_with': 'str_ends_with',
    'upper': 'str_upper', 'lower': 'str_lower', 'trim': 'str_trim',
    'concat': 'str_concat',
    'char_at': 'str_char_at', 'substr': 'str_substr',
    'index_of': 'str_index_of',
}

ARRAY_METHODS = {
    'sum': 'array_sum', 'min': 'array_min', 'max': 'array_max',
    'sort': 'array_sort', 'reverse': 'array_reverse',
    'contains': 'array_contains', 'index_of': 'array_index_of',
    'count': 'array_count',
}

BINOP_MAP = {
    '+': OP_ADD, '-': OP_SUB, '*': OP_MUL, '/': OP_DIV, '%': OP_MOD,
    '<': OP_CMP_LT, '>': OP_CMP_GT, '<=': OP_CMP_LE, '>=': OP_CMP_GE,
    '==': OP_CMP_EQ, '!=': OP_CMP_NE,
    'and': OP_AND, 'or': OP_OR,
}


# ── Instruction encoding (little-endian, matches x86 memcpy in VM) ──────────

def encode_A(op, rA, rB=0, rC=0):
    """Form A: op:8 | rA:8 | rB:8 | rC:8"""
    return struct.pack('<BBBB', op, rA, rB, rC)

def encode_B(op, rA, imm16):
    """Form B: op:8 | rA:8 | imm16:16 (unsigned packing; works for signed via masking)"""
    return struct.pack('<BBH', op, rA, imm16 & 0xFFFF)

def encode_C(op, imm24):
    """Form C: op:8 | imm24:24"""
    raw = imm24 & 0x00FFFFFF
    return struct.pack('<I', op | (raw << 8))


# ── Compiled function info ───────────────────────────────────────────────────

class FuncInfo:
    def __init__(self, name, param_count, register_count, instructions, instr_count):
        self.name = name
        self.param_count = param_count
        self.register_count = register_count
        self.instructions = instructions      # raw bytes
        self.instr_count = instr_count
        self.instr_offset = 0                 # filled during serialization


# ── Bytecode generator (top-level) ──────────────────────────────────────────

class BytecodeGen:
    def __init__(self):
        self.const_pool = []        # list of (tag, data_bytes) tuples
        self.const_cache = {}       # (type_key, value) → pool index
        self.functions = []         # list of FuncInfo
        self.func_names = {}        # name → function index
        self._globals = {}          # name → global slot index
        self._next_global = 0
        self._constants = {}        # const name → pool index
        self._object_decls = {}     # type name → ObjectDecl
        self._object_layouts = {}   # type name → {field_name: index}

    # ── Constant pool ────────────────────────────────────────────────────────

    def add_const_int(self, v):
        key = ('int', v)
        if key in self.const_cache:
            return self.const_cache[key]
        idx = len(self.const_pool)
        self.const_pool.append((0x01, struct.pack('<q', v)))
        self.const_cache[key] = idx
        return idx

    def add_const_float(self, v):
        key = ('float', v)
        if key in self.const_cache:
            return self.const_cache[key]
        idx = len(self.const_pool)
        self.const_pool.append((0x02, struct.pack('<d', v)))
        self.const_cache[key] = idx
        return idx

    def add_const_string(self, v):
        key = ('str', v)
        if key in self.const_cache:
            return self.const_cache[key]
        idx = len(self.const_pool)
        enc = v.encode('utf-8')
        self.const_pool.append((0x03, struct.pack('<I', len(enc)) + enc))
        self.const_cache[key] = idx
        return idx

    def add_const_bool(self, v):
        key = ('bool', v)
        if key in self.const_cache:
            return self.const_cache[key]
        idx = len(self.const_pool)
        self.const_pool.append((0x04, struct.pack('<B', 1 if v else 0)))
        self.const_cache[key] = idx
        return idx

    def add_const_empty(self):
        key = ('empty',)
        if key in self.const_cache:
            return self.const_cache[key]
        idx = len(self.const_pool)
        self.const_pool.append((0x05, b''))
        self.const_cache[key] = idx
        return idx

    # ── Compilation ──────────────────────────────────────────────────────────

    def compile_program(self, program):
        # Process constants
        for d in program.decls:
            if isinstance(d, ConstDecl):
                if isinstance(d.value, IntLiteral):
                    self._constants[d.name] = self.add_const_int(d.value.value)
                elif isinstance(d.value, FloatLiteral):
                    self._constants[d.name] = self.add_const_float(d.value.value)
                elif isinstance(d.value, StringLiteral):
                    self._constants[d.name] = self.add_const_string(d.value.value)
                elif isinstance(d.value, BoolLiteral):
                    self._constants[d.name] = self.add_const_bool(d.value.value)

        # Process object declarations
        for d in program.decls:
            if isinstance(d, ObjectDecl):
                self._object_decls[d.name] = d
                fields = []
                if d.parent and d.parent in self._object_decls:
                    fields.extend(self._object_decls[d.parent].fields)
                fields.extend(d.fields)
                self._object_layouts[d.name] = {f.name: i for i, f in enumerate(fields)}

        # Register all function names first (for forward calls)
        # Skip imported functions that have matching VM builtins (stdlib wrappers)
        # Local imports without builtins are compiled normally
        func_decls = [d for d in program.decls
                      if isinstance(d, FunctionDecl)
                      and not (getattr(d, 'is_imported', False) and d.name in BUILTIN_TABLE)]
        # Flatten nested functions into the function list
        expanded = []
        for fn in func_decls:
            expanded.append(fn)
            for nf in getattr(fn, 'nested_functions', []):
                expanded.append(nf)
        func_decls = expanded
        for i, fn in enumerate(func_decls):
            self.func_names[fn.name] = i

        # Register object methods as compiled functions
        method_funcs = []
        for d in program.decls:
            if isinstance(d, ObjectDecl):
                for method in d.methods:
                    method_key = f"{d.name}_{method.name}"
                    idx = len(func_decls) + len(method_funcs)
                    self.func_names[method_key] = idx
                    method_funcs.append((d, method))

        # Compile each function
        for fn in func_decls:
            fc = FuncCompiler(self, fn)
            fc.compile()
            self.functions.append(fc.to_func_info())

        # Compile object methods (self as implicit first param)
        for obj_decl, method in method_funcs:
            fc = FuncCompiler(self, method, obj_context=obj_decl)
            fc.compile()
            fi = fc.to_func_info()
            fi.name = f"{obj_decl.name}_{method.name}"
            self.functions.append(fi)

    # ── Serialization ────────────────────────────────────────────────────────

    def serialize(self):
        buf = bytearray()

        # Compute instruction offsets
        all_instrs = bytearray()
        for fi in self.functions:
            fi.instr_offset = len(all_instrs)
            all_instrs.extend(fi.instructions)

        # Header (24 bytes)
        buf.extend(struct.pack('<I', 0x554C4243))     # magic "ULBC"
        buf.extend(struct.pack('<H', 0x0100))          # ver_maj
        buf.extend(struct.pack('<H', 0x0000))          # ver_min
        buf.extend(struct.pack('<I', len(self.const_pool)))
        buf.extend(struct.pack('<I', len(self.functions)))
        buf.extend(struct.pack('<I', 0))               # block_count
        buf.extend(struct.pack('<I', 0))               # edge_count

        # Constant pool
        for tag, data in self.const_pool:
            buf.append(tag)
            buf.extend(data)

        # Function table
        for fi in self.functions:
            name_bytes = fi.name.encode('utf-8')
            buf.extend(struct.pack('<I', len(name_bytes)))
            buf.extend(name_bytes)
            buf.extend(struct.pack('<B', fi.param_count))
            buf.extend(struct.pack('<B', fi.register_count))
            buf.extend(struct.pack('<I', fi.instr_offset))
            buf.extend(struct.pack('<I', fi.instr_count))
            buf.extend(struct.pack('<I', 0))           # first_block_idx
            buf.extend(struct.pack('<H', 0))           # block_count

        # Instruction stream
        buf.extend(all_instrs)

        return bytes(buf)


# ── Per-function compiler ────────────────────────────────────────────────────

class FuncCompiler:
    def __init__(self, gen, fn, obj_context=None):
        self.gen = gen
        self.fn = fn
        self.obj_context = obj_context
        self.obj_layout = gen._object_layouts.get(obj_context.name) if obj_context else None
        self.instructions = bytearray()
        self.instr_count = 0
        self.var_types = {}         # var name → type name (for object dispatch)

        # Register allocation
        self.vars = {}              # name → register
        self._var_regs = set()      # registers permanently assigned to variables
        self._next_var = 0

        # If compiling an object method, inject implicit self at r0
        if obj_context:
            self.vars['__self__'] = self._next_var
            self._var_regs.add(self._next_var)
            self._next_var += 1

        # Pre-allocate param registers (r0..r(n-1))
        for p in fn.params:
            self.vars[p.name] = self._next_var
            self._var_regs.add(self._next_var)
            self._next_var += 1

        # Pre-scan for all local variable declarations
        self._scan_vars(fn.body)

        # Temps start above all pre-allocated vars
        self.temp_base = self._next_var
        self.temp_top = self.temp_base
        self.max_reg = self.temp_base

        # Loop context (stacks for break/continue patching)
        self.loop_starts = []
        self.break_patches = []
        self.continue_patches = []

    def _scan_vars(self, stmts):
        """Walk statements to find all VarDecl, TupleDestructure, and for-loop variable names."""
        for stmt in stmts:
            if isinstance(stmt, VarDecl):
                if stmt.name not in self.vars:
                    self.vars[stmt.name] = self._next_var
                    self._var_regs.add(self._next_var)
                    self._next_var += 1
            elif isinstance(stmt, TupleDestructure):
                for t_, name in stmt.targets:
                    if name not in self.vars:
                        self.vars[name] = self._next_var
                        self._var_regs.add(self._next_var)
                        self._next_var += 1
            elif isinstance(stmt, For):
                if stmt.var not in self.vars:
                    self.vars[stmt.var] = self._next_var
                    self._var_regs.add(self._next_var)
                    self._next_var += 1
                self._scan_vars(stmt.body)
            elif isinstance(stmt, While):
                self._scan_vars(stmt.body)
            elif isinstance(stmt, If):
                self._scan_vars(stmt.then_body)
                self._scan_vars(stmt.else_body)
            elif isinstance(stmt, Match):
                for case in stmt.cases:
                    self._scan_vars(case.body)

    # ── Register helpers ─────────────────────────────────────────────────────

    def push_temp(self):
        r = self.temp_top
        self.temp_top += 1
        if self.temp_top > self.max_reg:
            self.max_reg = self.temp_top
        return r

    def pop_temp(self):
        self.temp_top -= 1

    def is_temp(self, r):
        return r >= self.temp_base and r not in self._var_regs

    def free_if_temp(self, r):
        if self.is_temp(r):
            self.pop_temp()

    # ── Instruction emit helpers ─────────────────────────────────────────────

    def emit_A(self, op, rA, rB=0, rC=0):
        self.instructions.extend(encode_A(op, rA, rB, rC))
        idx = self.instr_count
        self.instr_count += 1
        return idx

    def emit_B(self, op, rA, imm16):
        self.instructions.extend(encode_B(op, rA, imm16))
        idx = self.instr_count
        self.instr_count += 1
        return idx

    def emit_C(self, op, imm24):
        self.instructions.extend(encode_C(op, imm24))
        idx = self.instr_count
        self.instr_count += 1
        return idx

    def current_pc(self):
        return self.instr_count

    def patch_jump(self, instr_idx, offset):
        """Patch a Form B jump (JUMP_IF_FALSE/JUMP_IF_TRUE) with signed offset."""
        byte_pos = instr_idx * 4
        old = struct.unpack_from('<I', self.instructions, byte_pos)[0]
        op = old & 0xFF
        rA = (old >> 8) & 0xFF
        self.instructions[byte_pos:byte_pos + 4] = encode_B(op, rA, offset)

    def patch_jump_c(self, instr_idx, offset):
        """Patch a Form C jump (OP_JUMP) with signed 24-bit offset."""
        byte_pos = instr_idx * 4
        self.instructions[byte_pos:byte_pos + 4] = encode_C(OP_JUMP, offset)

    # ── Top-level compile ────────────────────────────────────────────────────

    def compile(self):
        for stmt in self.fn.body:
            self.compile_stmt(stmt)
        # Implicit return at end of function (may be dead code after explicit return)
        if self.fn.name == 'main':
            r = self.push_temp()
            ci = self.gen.add_const_int(0)
            self.emit_B(OP_LOAD_CONST, r, ci)
            self.emit_A(OP_RETURN, r)
            self.pop_temp()
        else:
            self.emit_C(OP_RETURN_NONE, 0)

    def to_func_info(self):
        param_count = len(self.fn.params)
        if self.obj_context:
            param_count += 1  # implicit self
        return FuncInfo(
            self.fn.name,
            param_count,
            min(self.max_reg, 255),
            bytes(self.instructions),
            self.instr_count
        )

    # ── Statement compilation ────────────────────────────────────────────────

    def compile_stmt(self, stmt):
        if isinstance(stmt, VarDecl):
            self._compile_var_decl(stmt)
        elif isinstance(stmt, Assign):
            self._compile_assign(stmt)
        elif isinstance(stmt, Print):
            self._compile_print(stmt)
        elif isinstance(stmt, Return):
            self._compile_return(stmt)
        elif isinstance(stmt, If):
            self._compile_if(stmt)
        elif isinstance(stmt, While):
            self._compile_while(stmt)
        elif isinstance(stmt, For):
            self._compile_for(stmt)
        elif isinstance(stmt, ExprStmt):
            r = self.compile_expr(stmt.expr)
            self.free_if_temp(r)
        elif isinstance(stmt, Escape):
            self._compile_escape()
        elif isinstance(stmt, Continue):
            self._compile_continue()
        elif isinstance(stmt, PostIncrement):
            if isinstance(stmt.operand, Identifier) and stmt.operand.name in self.vars:
                self.emit_A(OP_INC_REG, self.vars[stmt.operand.name])
        elif isinstance(stmt, PostDecrement):
            if isinstance(stmt.operand, Identifier) and stmt.operand.name in self.vars:
                self.emit_A(OP_DEC_REG, self.vars[stmt.operand.name])
        elif isinstance(stmt, Match):
            self._compile_match(stmt)
        elif isinstance(stmt, TupleDestructure):
            self._compile_tuple_destructure(stmt)
        elif isinstance(stmt, NormDirective):
            pass  # no-op in bytecode
        elif isinstance(stmt, Yield):
            # Cooperative yield — emit OP_YIELD (no-op in sequential VM)
            self.emit_A(OP_YIELD, 0)

    def _compile_var_decl(self, stmt):
        var_r = self.vars[stmt.name]
        # Object type — create array with field slots
        if stmt.type_.name in self.gen._object_decls:
            self.var_types[stmt.name] = stmt.type_.name
            layout = self.gen._object_layouts[stmt.type_.name]
            field_count = len(layout)
            size_r = self.push_temp()
            self.emit_B(OP_LOAD_CONST, size_r, self.gen.add_const_int(field_count))
            self.emit_A(OP_NEW_ARRAY, var_r, 0, size_r)
            self.pop_temp()
            return
        if stmt.init is not None:
            init_r = self.compile_expr(stmt.init)
            if init_r != var_r:
                self.emit_A(OP_MOV, var_r, init_r)
            self.free_if_temp(init_r)
        else:
            ci = self.gen.add_const_int(0)
            self.emit_B(OP_LOAD_CONST, var_r, ci)

    def _compile_assign(self, stmt):
        if isinstance(stmt.target, Identifier):
            name = stmt.target.name
            # Global?
            if name in self.gen._globals and name not in self.vars:
                val_r = self.compile_expr(stmt.value)
                self.emit_B(OP_STORE_GLOBAL, val_r, self.gen._globals[name])
                self.free_if_temp(val_r)
                return
            r = self.vars.get(name)
            if r is None:
                # Shouldn't happen after semcheck, but handle gracefully
                return
            if stmt.op == '=':
                val_r = self.compile_expr(stmt.value)
                if val_r != r:
                    self.emit_A(OP_MOV, r, val_r)
                self.free_if_temp(val_r)
            else:
                # Compound: +=, -=, *=, /=, %=
                val_r = self.compile_expr(stmt.value)
                base_op = stmt.op[0]
                opcode = BINOP_MAP.get(base_op)
                if opcode:
                    self.emit_A(opcode, r, r, val_r)
                self.free_if_temp(val_r)

        elif isinstance(stmt.target, Index):
            arr_r = self.compile_expr(stmt.target.target)
            idx_r = self.compile_expr(stmt.target.index)
            val_r = self.compile_expr(stmt.value)
            self.emit_A(OP_INDEX_SET, arr_r, idx_r, val_r)
            self.free_if_temp(val_r)
            self.free_if_temp(idx_r)
            self.free_if_temp(arr_r)

        elif isinstance(stmt.target, FieldAccess):
            obj_r = self.compile_expr(stmt.target.target)
            obj_type = None
            if isinstance(stmt.target.target, Identifier):
                obj_type = self.var_types.get(stmt.target.target.name)
            if obj_type and obj_type in self.gen._object_layouts:
                field_idx = self.gen._object_layouts[obj_type].get(stmt.target.field, 0)
                idx_r = self.push_temp()
                self.emit_B(OP_LOAD_CONST, idx_r, self.gen.add_const_int(field_idx))
                val_r = self.compile_expr(stmt.value)
                self.emit_A(OP_INDEX_SET, obj_r, idx_r, val_r)
                self.free_if_temp(val_r)
                self.pop_temp()  # idx_r
            self.free_if_temp(obj_r)

    def _compile_print(self, stmt):
        r = self.compile_expr(stmt.value)
        self.emit_A(OP_PRINT, r)
        self.free_if_temp(r)

    def _compile_return(self, stmt):
        if stmt.value is None:
            self.emit_C(OP_RETURN_NONE, 0)
        else:
            r = self.compile_expr(stmt.value)
            self.emit_A(OP_RETURN, r)
            self.free_if_temp(r)

    def _compile_if(self, stmt):
        cond_r = self.compile_expr(stmt.condition)
        jump_false_pc = self.current_pc()
        self.emit_B(OP_JUMP_IF_FALSE, cond_r, 0)
        self.free_if_temp(cond_r)

        for s in stmt.then_body:
            self.compile_stmt(s)

        if stmt.else_body:
            jump_over_pc = self.current_pc()
            self.emit_C(OP_JUMP, 0)
            else_start = self.current_pc()
            self.patch_jump(jump_false_pc, else_start - jump_false_pc - 1)

            for s in stmt.else_body:
                self.compile_stmt(s)

            end_pc = self.current_pc()
            self.patch_jump_c(jump_over_pc, end_pc - jump_over_pc - 1)
        else:
            end_pc = self.current_pc()
            self.patch_jump(jump_false_pc, end_pc - jump_false_pc - 1)

    def _compile_while(self, stmt):
        loop_start = self.current_pc()
        self.loop_starts.append(loop_start)
        self.break_patches.append([])
        self.continue_patches.append([])

        cond_r = self.compile_expr(stmt.condition)
        jump_false_pc = self.current_pc()
        self.emit_B(OP_JUMP_IF_FALSE, cond_r, 0)
        self.free_if_temp(cond_r)

        for s in stmt.body:
            self.compile_stmt(s)

        # Jump back to loop start
        back_pc = self.current_pc()
        self.emit_C(OP_JUMP, loop_start - back_pc - 1)

        end_pc = self.current_pc()
        self.patch_jump(jump_false_pc, end_pc - jump_false_pc - 1)

        # Patch break/continue
        for bp in self.break_patches.pop():
            self.patch_jump_c(bp, end_pc - bp - 1)
        for cp in self.continue_patches.pop():
            self.patch_jump_c(cp, loop_start - cp - 1)
        self.loop_starts.pop()

    def _compile_for(self, stmt):
        # parallel for — sequential fallback in bytecode VM
        # (emit NOP marker so VM knows it was intended as parallel)
        if getattr(stmt, 'parallel', False):
            self.emit_A(OP_NOP, 0)  # parallel-for marker (no-op)
        if isinstance(stmt.iterable, Call) and stmt.iterable.name == 'range':
            self._compile_for_range(stmt)
        else:
            self._compile_for_array(stmt)

    def _compile_for_range(self, stmt):
        saved_temp = self.temp_top
        args = stmt.iterable.args

        # Compile start and end
        if len(args) == 1:
            start_r = self.push_temp()
            self.emit_B(OP_LOAD_CONST, start_r, self.gen.add_const_int(0))
            end_r = self.compile_expr(args[0])
        elif len(args) >= 2:
            start_r = self.compile_expr(args[0])
            end_r = self.compile_expr(args[1])
        else:
            return

        # Step
        has_step = len(args) >= 3
        if has_step:
            step_r = self.compile_expr(args[2])

        # Initialize loop variable
        var_r = self.vars[stmt.var]
        self.emit_A(OP_MOV, var_r, start_r)

        loop_start = self.current_pc()
        self.loop_starts.append(loop_start)
        self.break_patches.append([])
        self.continue_patches.append([])

        # Condition: var < end
        cond_r = self.push_temp()
        self.emit_A(OP_CMP_LT, cond_r, var_r, end_r)
        jump_false_pc = self.current_pc()
        self.emit_B(OP_JUMP_IF_FALSE, cond_r, 0)
        self.pop_temp()  # cond_r

        # Body
        for s in stmt.body:
            self.compile_stmt(s)

        # Increment
        if has_step:
            self.emit_A(OP_ADD, var_r, var_r, step_r)
        else:
            self.emit_A(OP_INC_REG, var_r)

        # Jump back
        back_pc = self.current_pc()
        self.emit_C(OP_JUMP, loop_start - back_pc - 1)

        end_pc = self.current_pc()
        self.patch_jump(jump_false_pc, end_pc - jump_false_pc - 1)

        for bp in self.break_patches.pop():
            self.patch_jump_c(bp, end_pc - bp - 1)
        for cp in self.continue_patches.pop():
            self.patch_jump_c(cp, loop_start - cp - 1)
        self.loop_starts.pop()

        # Restore temps (start, end, step are no longer needed)
        self.temp_top = saved_temp

    def _compile_for_array(self, stmt):
        saved_temp = self.temp_top

        # Compile iterable
        iter_r = self.compile_expr(stmt.iterable)

        # Get length
        len_r = self.push_temp()
        self.emit_A(OP_LENGTH, len_r, iter_r)

        # Index counter
        idx_r = self.push_temp()
        self.emit_B(OP_LOAD_CONST, idx_r, self.gen.add_const_int(0))

        # Loop variable (pre-allocated)
        var_r = self.vars[stmt.var]

        loop_start = self.current_pc()
        self.loop_starts.append(loop_start)
        self.break_patches.append([])
        self.continue_patches.append([])

        # Condition: idx < length
        cond_r = self.push_temp()
        self.emit_A(OP_CMP_LT, cond_r, idx_r, len_r)
        jump_false_pc = self.current_pc()
        self.emit_B(OP_JUMP_IF_FALSE, cond_r, 0)
        self.pop_temp()  # cond_r

        # Load element: var = array[idx]
        self.emit_A(OP_INDEX_GET, var_r, iter_r, idx_r)

        # Body
        for s in stmt.body:
            self.compile_stmt(s)

        # Increment idx
        self.emit_A(OP_INC_REG, idx_r)

        # Jump back
        back_pc = self.current_pc()
        self.emit_C(OP_JUMP, loop_start - back_pc - 1)

        end_pc = self.current_pc()
        self.patch_jump(jump_false_pc, end_pc - jump_false_pc - 1)

        for bp in self.break_patches.pop():
            self.patch_jump_c(bp, end_pc - bp - 1)
        for cp in self.continue_patches.pop():
            self.patch_jump_c(cp, loop_start - cp - 1)
        self.loop_starts.pop()

        self.temp_top = saved_temp

    def _compile_escape(self):
        if self.break_patches:
            pc = self.current_pc()
            self.emit_C(OP_JUMP, 0)
            self.break_patches[-1].append(pc)

    def _compile_continue(self):
        if self.continue_patches:
            pc = self.current_pc()
            self.emit_C(OP_JUMP, 0)
            self.continue_patches[-1].append(pc)

    def _compile_match(self, stmt):
        subject_r = self.compile_expr(stmt.subject)
        end_jumps = []

        for case in stmt.cases:
            if case.value is not None:
                val_r = self.compile_expr(case.value)
                cond_r = self.push_temp()
                self.emit_A(OP_CMP_EQ, cond_r, subject_r, val_r)
                jump_next = self.current_pc()
                self.emit_B(OP_JUMP_IF_FALSE, cond_r, 0)
                self.pop_temp()   # cond_r
                self.free_if_temp(val_r)

                for s in case.body:
                    self.compile_stmt(s)
                end_jumps.append(self.current_pc())
                self.emit_C(OP_JUMP, 0)
                self.patch_jump(jump_next, self.current_pc() - jump_next - 1)
            else:
                # Default case
                for s in case.body:
                    self.compile_stmt(s)

        end_pc = self.current_pc()
        for ej in end_jumps:
            self.patch_jump_c(ej, end_pc - ej - 1)
        self.free_if_temp(subject_r)

    # ── Expression compilation ───────────────────────────────────────────────
    # Returns the register where the result is stored.
    # If is_temp(r) is True, the caller must free it when done.

    def compile_expr(self, expr):
        if isinstance(expr, IntLiteral):
            r = self.push_temp()
            self.emit_B(OP_LOAD_CONST, r, self.gen.add_const_int(expr.value))
            return r

        if isinstance(expr, FloatLiteral):
            r = self.push_temp()
            self.emit_B(OP_LOAD_CONST, r, self.gen.add_const_float(expr.value))
            return r

        if isinstance(expr, StringLiteral):
            r = self.push_temp()
            self.emit_B(OP_LOAD_CONST, r, self.gen.add_const_string(expr.value))
            return r

        if isinstance(expr, BoolLiteral):
            r = self.push_temp()
            self.emit_B(OP_LOAD_CONST, r, self.gen.add_const_bool(expr.value))
            return r

        if isinstance(expr, EmptyLiteral):
            r = self.push_temp()
            self.emit_B(OP_LOAD_CONST, r, self.gen.add_const_empty())
            return r

        if isinstance(expr, Identifier):
            if expr.name in self.vars:
                return self.vars[expr.name]
            if expr.name in self.gen._globals:
                r = self.push_temp()
                self.emit_B(OP_LOAD_GLOBAL, r, self.gen._globals[expr.name])
                return r
            # Check constants
            if expr.name in self.gen._constants:
                r = self.push_temp()
                self.emit_B(OP_LOAD_CONST, r, self.gen._constants[expr.name])
                return r
            # Object field access in method context (self is r0)
            if self.obj_layout and expr.name in self.obj_layout:
                field_idx = self.obj_layout[expr.name]
                idx_r = self.push_temp()
                self.emit_B(OP_LOAD_CONST, idx_r, self.gen.add_const_int(field_idx))
                result_r = self.push_temp()
                self.emit_A(OP_INDEX_GET, result_r, self.vars['__self__'], idx_r)
                self.pop_temp()  # idx_r
                return result_r
            raise NameError(f"undefined variable '{expr.name}' at line {expr.line}")

        if isinstance(expr, BinaryOp):
            return self._compile_binop(expr)

        if isinstance(expr, UnaryOp):
            return self._compile_unaryop(expr)

        if isinstance(expr, Call):
            return self._compile_call(expr)

        if isinstance(expr, Cast):
            return self._compile_cast(expr)

        if isinstance(expr, Index):
            arr_r = self.compile_expr(expr.target)
            idx_r = self.compile_expr(expr.index)
            result_r = self.push_temp()
            self.emit_A(OP_INDEX_GET, result_r, arr_r, idx_r)
            self.free_if_temp(idx_r)
            self.free_if_temp(arr_r)
            return result_r

        if isinstance(expr, ArrayLiteral):
            return self._compile_array_literal(expr)

        if isinstance(expr, ArrayComprehension):
            return self._compile_array_comprehension(expr)

        if isinstance(expr, MethodCall):
            return self._compile_method_call(expr)

        if isinstance(expr, FieldAccess):
            return self._compile_field_access(expr)

        if isinstance(expr, TupleLiteral):
            return self._compile_tuple_literal(expr)

        if isinstance(expr, PostIncrement):
            r = self.compile_expr(expr.operand)
            if self.is_temp(r):
                self.emit_A(OP_INC_REG, r)
                return r
            result_r = self.push_temp()
            self.emit_A(OP_MOV, result_r, r)
            self.emit_A(OP_INC_REG, r)
            return result_r

        if isinstance(expr, PostDecrement):
            r = self.compile_expr(expr.operand)
            if self.is_temp(r):
                self.emit_A(OP_DEC_REG, r)
                return r
            result_r = self.push_temp()
            self.emit_A(OP_MOV, result_r, r)
            self.emit_A(OP_DEC_REG, r)
            return result_r

        if isinstance(expr, OkResult):
            return self._compile_ok_result(expr)

        if isinstance(expr, ErrorResult):
            return self._compile_error_result(expr)

        if isinstance(expr, ResultPropagation):
            return self._compile_result_propagation(expr)

        if isinstance(expr, StructLiteral):
            return self._compile_struct_literal(expr)

        raise NotImplementedError(f"bytecode: unsupported expression {type(expr).__name__}")

    def _compile_binop(self, expr):
        left_r = self.compile_expr(expr.left)
        right_r = self.compile_expr(expr.right)
        opcode = BINOP_MAP.get(expr.op)
        if opcode is None:
            raise NotImplementedError(f"bytecode: binary operator '{expr.op}'")

        if self.is_temp(left_r):
            self.emit_A(opcode, left_r, left_r, right_r)
            self.free_if_temp(right_r)
            return left_r
        elif self.is_temp(right_r):
            self.emit_A(opcode, right_r, left_r, right_r)
            return right_r
        else:
            result_r = self.push_temp()
            self.emit_A(opcode, result_r, left_r, right_r)
            return result_r

    def _compile_unaryop(self, expr):
        operand_r = self.compile_expr(expr.operand)
        if self.is_temp(operand_r):
            if expr.op == '-':
                self.emit_A(OP_NEG, operand_r, operand_r)
            elif expr.op == 'not':
                self.emit_A(OP_NOT, operand_r, operand_r)
            else:
                raise NotImplementedError(f"bytecode: unary '{expr.op}'")
            return operand_r
        else:
            result_r = self.push_temp()
            if expr.op == '-':
                self.emit_A(OP_NEG, result_r, operand_r)
            elif expr.op == 'not':
                self.emit_A(OP_NOT, result_r, operand_r)
            else:
                raise NotImplementedError(f"bytecode: unary '{expr.op}'")
            return result_r

    def _compile_call(self, expr):
        # ── Concurrency builtins (sequential fallback) ──
        if expr.name == 'spawn':
            return self._compile_spawn(expr)
        if expr.name == 'wait':
            return self._compile_await(expr)
        if expr.name == 'lock':
            # lock() — emit OP_LOCK (no-op in VM)
            r = self.push_temp()
            self.emit_B(OP_LOAD_CONST, r, self.gen.add_const_int(0))
            self.emit_A(OP_LOCK, r)
            return r
        if expr.name == 'unlock':
            # unlock() — emit OP_UNLOCK (no-op in VM)
            r = self.push_temp()
            self.emit_B(OP_LOAD_CONST, r, self.gen.add_const_int(0))
            self.emit_A(OP_UNLOCK, r)
            return r
        if expr.name == 'yield_now' or expr.name == 'schedule':
            # Cooperative scheduling — emit OP_YIELD (no-op in VM)
            r = self.push_temp()
            self.emit_A(OP_YIELD, 0)
            self.emit_B(OP_LOAD_CONST, r, self.gen.add_const_int(0))
            return r

        # ── Builtins ──
        if expr.name == 'len':
            arg_r = self.compile_expr(expr.args[0])
            if self.is_temp(arg_r):
                self.emit_A(OP_LENGTH, arg_r, arg_r)
                return arg_r
            result_r = self.push_temp()
            self.emit_A(OP_LENGTH, result_r, arg_r)
            return result_r

        if expr.name == 'exit':
            if expr.args:
                r = self.compile_expr(expr.args[0])
                self.emit_A(OP_RETURN, r)
                self.free_if_temp(r)
            else:
                self.emit_C(OP_RETURN_NONE, 0)
            # Unreachable; return a dummy temp
            return self.push_temp()

        if expr.name == 'range':
            # range() as a standalone expression (not in for/comprehension)
            # Build an array
            return self._compile_range_to_array(expr)

        # change()/cast() — type conversion, just compile inner expression with cast
        if expr.name in ('change', 'cast') and len(expr.args) == 1:
            return self.compile_expr(expr.args[0])

        # ── User function call ──
        func_idx = self.gen.func_names.get(expr.name)
        if func_idx is None:
            # Check builtins (stdlib / foreign imports)
            builtin_id = BUILTIN_TABLE.get(expr.name)
            if builtin_id is not None:
                return self._compile_builtin_call(builtin_id, expr.args)
            raise NameError(f"bytecode: undefined function '{expr.name}'")

        saved_temp = self.temp_top
        n_args = len(expr.args)

        # Compile arguments into natural positions
        arg_regs = []
        for arg in expr.args:
            arg_regs.append(self.compile_expr(arg))

        # Allocate consecutive call area (at least 1 slot for return value)
        n_slots = max(n_args, 1)
        first_arg = self.temp_top
        for _ in range(n_slots):
            self.push_temp()

        # Move args into call area
        for i, ar in enumerate(arg_regs):
            target = first_arg + i
            if ar != target:
                self.emit_A(OP_MOV, target, ar)

        # Emit call
        self.emit_A(OP_CALL, first_arg, func_idx, n_args)

        # Result is in first_arg. Move to saved_temp position.
        result_r = saved_temp
        if first_arg != result_r:
            self.emit_A(OP_MOV, result_r, first_arg)
        self.temp_top = saved_temp + 1
        return result_r

    def _compile_cast(self, expr):
        src_r = self.compile_expr(expr.expr)
        target = expr.target_type.name
        cast_op = {'int': OP_CAST_INT, 'float': OP_CAST_FLOAT,
                    'string': OP_CAST_STRING, 'bool': OP_CAST_BOOL}.get(target)
        if cast_op is None:
            return src_r
        if self.is_temp(src_r):
            self.emit_A(cast_op, src_r, src_r)
            return src_r
        result_r = self.push_temp()
        self.emit_A(cast_op, result_r, src_r)
        return result_r

    def _compile_array_literal(self, expr):
        saved_temp = self.temp_top
        n = len(expr.elements)

        # Size
        size_r = self.push_temp()
        self.emit_B(OP_LOAD_CONST, size_r, self.gen.add_const_int(n))

        # Create array
        arr_r = self.push_temp()
        self.emit_A(OP_NEW_ARRAY, arr_r, 0, size_r)

        # Fill elements
        idx_r = self.push_temp()
        for i, elem in enumerate(expr.elements):
            self.emit_B(OP_LOAD_CONST, idx_r, self.gen.add_const_int(i))
            val_r = self.compile_expr(elem)
            self.emit_A(OP_INDEX_SET, arr_r, idx_r, val_r)
            self.free_if_temp(val_r)

        # Move result to saved position
        result_r = saved_temp
        if arr_r != result_r:
            self.emit_A(OP_MOV, result_r, arr_r)
        self.temp_top = saved_temp + 1
        return result_r

    def _compile_array_comprehension(self, expr):
        """Compile [expr for var in range(start, end)]."""
        if not (isinstance(expr.iterable, Call) and expr.iterable.name == 'range'):
            raise NotImplementedError("bytecode: comprehension only supports range()")

        saved_temp = self.temp_top
        args = expr.iterable.args

        # Start and end
        if len(args) == 1:
            start_r = self.push_temp()
            self.emit_B(OP_LOAD_CONST, start_r, self.gen.add_const_int(0))
            end_r = self.compile_expr(args[0])
        elif len(args) >= 2:
            start_r = self.compile_expr(args[0])
            end_r = self.compile_expr(args[1])
        else:
            raise ValueError("range() requires at least 1 argument")

        # Size = end - start
        size_r = self.push_temp()
        self.emit_A(OP_SUB, size_r, end_r, start_r)

        # Create array
        arr_r = self.push_temp()
        self.emit_A(OP_NEW_ARRAY, arr_r, 0, size_r)

        # Index counter = 0
        idx_r = self.push_temp()
        self.emit_B(OP_LOAD_CONST, idx_r, self.gen.add_const_int(0))

        # Comprehension variable
        comp_var_r = self.push_temp()
        self.vars[expr.var] = comp_var_r
        self._var_regs.add(comp_var_r)
        self.emit_A(OP_MOV, comp_var_r, start_r)

        # Loop condition
        loop_start = self.current_pc()
        cond_r = self.push_temp()
        self.emit_A(OP_CMP_LT, cond_r, comp_var_r, end_r)
        jump_false_pc = self.current_pc()
        self.emit_B(OP_JUMP_IF_FALSE, cond_r, 0)
        self.pop_temp()  # cond_r

        # Compile comprehension expression
        expr_r = self.compile_expr(expr.expr)

        # Store in array
        self.emit_A(OP_INDEX_SET, arr_r, idx_r, expr_r)
        self.free_if_temp(expr_r)

        # Increment
        self.emit_A(OP_INC_REG, idx_r)
        self.emit_A(OP_INC_REG, comp_var_r)

        # Jump back
        back_pc = self.current_pc()
        self.emit_C(OP_JUMP, loop_start - back_pc - 1)

        # Patch exit
        end_pc = self.current_pc()
        self.patch_jump(jump_false_pc, end_pc - jump_false_pc - 1)

        # Cleanup comprehension variable
        del self.vars[expr.var]
        self._var_regs.discard(comp_var_r)

        # Move arr to saved_temp position
        result_r = saved_temp
        if arr_r != result_r:
            self.emit_A(OP_MOV, result_r, arr_r)
        self.temp_top = saved_temp + 1
        return result_r

    def _compile_range_to_array(self, expr):
        """Compile range(args) as a standalone expression returning an array."""
        saved_temp = self.temp_top
        args = expr.args

        if len(args) == 1:
            start_r = self.push_temp()
            self.emit_B(OP_LOAD_CONST, start_r, self.gen.add_const_int(0))
            end_r = self.compile_expr(args[0])
        elif len(args) >= 2:
            start_r = self.compile_expr(args[0])
            end_r = self.compile_expr(args[1])
        else:
            raise ValueError("range() requires at least 1 argument")

        # Size
        size_r = self.push_temp()
        self.emit_A(OP_SUB, size_r, end_r, start_r)

        # Create array
        arr_r = self.push_temp()
        self.emit_A(OP_NEW_ARRAY, arr_r, 0, size_r)

        # Fill with sequential values
        idx_r = self.push_temp()
        self.emit_B(OP_LOAD_CONST, idx_r, self.gen.add_const_int(0))
        val_r = self.push_temp()
        self.emit_A(OP_MOV, val_r, start_r)

        loop_start = self.current_pc()
        cond_r = self.push_temp()
        self.emit_A(OP_CMP_LT, cond_r, val_r, end_r)
        jump_pc = self.current_pc()
        self.emit_B(OP_JUMP_IF_FALSE, cond_r, 0)
        self.pop_temp()

        self.emit_A(OP_INDEX_SET, arr_r, idx_r, val_r)
        self.emit_A(OP_INC_REG, idx_r)
        self.emit_A(OP_INC_REG, val_r)

        back_pc = self.current_pc()
        self.emit_C(OP_JUMP, loop_start - back_pc - 1)
        self.patch_jump(jump_pc, self.current_pc() - jump_pc - 1)

        result_r = saved_temp
        if arr_r != result_r:
            self.emit_A(OP_MOV, result_r, arr_r)
        self.temp_top = saved_temp + 1
        return result_r

    def _compile_builtin_call(self, builtin_id, args):
        """Emit OP_CALL_BUILTIN for VM-internal functions."""
        saved_temp = self.temp_top
        n_args = len(args)

        arg_regs = []
        for arg in args:
            arg_regs.append(self.compile_expr(arg))

        n_slots = max(n_args, 1)
        first_arg = self.temp_top
        for _ in range(n_slots):
            self.push_temp()

        for i, ar in enumerate(arg_regs):
            target = first_arg + i
            if ar != target:
                self.emit_A(OP_MOV, target, ar)

        self.emit_A(OP_CALL_BUILTIN, first_arg, builtin_id, n_args)

        result_r = saved_temp
        if first_arg != result_r:
            self.emit_A(OP_MOV, result_r, first_arg)
        self.temp_top = saved_temp + 1
        return result_r

    def _compile_func_call_with_args(self, func_idx, arg_exprs):
        """Compile OP_CALL with pre-determined arg expressions and function index."""
        saved_temp = self.temp_top
        n_args = len(arg_exprs)

        arg_regs = []
        for arg in arg_exprs:
            arg_regs.append(self.compile_expr(arg))

        n_slots = max(n_args, 1)
        first_arg = self.temp_top
        for _ in range(n_slots):
            self.push_temp()

        for i, ar in enumerate(arg_regs):
            target = first_arg + i
            if ar != target:
                self.emit_A(OP_MOV, target, ar)

        self.emit_A(OP_CALL, first_arg, func_idx, n_args)

        result_r = saved_temp
        if first_arg != result_r:
            self.emit_A(OP_MOV, result_r, first_arg)
        self.temp_top = saved_temp + 1
        return result_r

    def _compile_method_call(self, expr):
        """Compile obj.method(args) — rewrite to function call or builtin."""
        # String methods: s.len() → str_len(s)
        if expr.method in STRING_METHODS:
            func_name = STRING_METHODS[expr.method]
            all_args = [expr.target] + expr.args
            func_idx = self.gen.func_names.get(func_name)
            if func_idx is not None:
                return self._compile_func_call_with_args(func_idx, all_args)
            builtin_id = BUILTIN_TABLE.get(func_name)
            if builtin_id is not None:
                return self._compile_builtin_call(builtin_id, all_args)

        # Array methods: nums.sort() → array_sort(nums, 0)
        if expr.method in ARRAY_METHODS:
            func_name = ARRAY_METHODS[expr.method]
            # Inject size placeholder (VM ignores it, uses arr->size())
            size_node = IntLiteral(value=0, line=expr.line, col=expr.col)
            all_args = [expr.target, size_node] + expr.args
            func_idx = self.gen.func_names.get(func_name)
            if func_idx is not None:
                return self._compile_func_call_with_args(func_idx, all_args)
            builtin_id = BUILTIN_TABLE.get(func_name)
            if builtin_id is not None:
                return self._compile_builtin_call(builtin_id, all_args)

        # Object method call: d.speak() → Dog_speak(d)
        if isinstance(expr.target, Identifier):
            obj_type = self.var_types.get(expr.target.name)
            if obj_type:
                method_key = f"{obj_type}_{expr.method}"
                func_idx = self.gen.func_names.get(method_key)
                if func_idx is not None:
                    return self._compile_func_call_with_args(func_idx, [expr.target] + expr.args)

        raise NotImplementedError(f"bytecode: unsupported method call .{expr.method}")

    def _compile_field_access(self, expr):
        """Compile obj.field → INDEX_GET with known field index."""
        obj_r = self.compile_expr(expr.target)
        obj_type = None
        if isinstance(expr.target, Identifier):
            obj_type = self.var_types.get(expr.target.name)
        if obj_type and obj_type in self.gen._object_layouts:
            field_idx = self.gen._object_layouts[obj_type].get(expr.field, 0)
            idx_r = self.push_temp()
            self.emit_B(OP_LOAD_CONST, idx_r, self.gen.add_const_int(field_idx))
            result_r = self.push_temp()
            self.emit_A(OP_INDEX_GET, result_r, obj_r, idx_r)
            self.pop_temp()  # idx_r
            self.free_if_temp(obj_r)
            return result_r
        return obj_r

    def _compile_tuple_literal(self, expr):
        """Compile (a, b, ...) as an array for multi-value return."""
        saved_temp = self.temp_top
        n = len(expr.elements)
        size_r = self.push_temp()
        self.emit_B(OP_LOAD_CONST, size_r, self.gen.add_const_int(n))
        arr_r = self.push_temp()
        self.emit_A(OP_NEW_ARRAY, arr_r, 0, size_r)
        idx_r = self.push_temp()
        for i, elem in enumerate(expr.elements):
            self.emit_B(OP_LOAD_CONST, idx_r, self.gen.add_const_int(i))
            val_r = self.compile_expr(elem)
            self.emit_A(OP_INDEX_SET, arr_r, idx_r, val_r)
            self.free_if_temp(val_r)
        result_r = saved_temp
        if arr_r != result_r:
            self.emit_A(OP_MOV, result_r, arr_r)
        self.temp_top = saved_temp + 1
        return result_r

    def _compile_tuple_destructure(self, stmt):
        """Compile (int lo, int hi) = expr — extract elements from array."""
        val_r = self.compile_expr(stmt.value)
        idx_r = self.push_temp()
        for i, (t_, name) in enumerate(stmt.targets):
            var_r = self.vars[name]
            self.emit_B(OP_LOAD_CONST, idx_r, self.gen.add_const_int(i))
            self.emit_A(OP_INDEX_GET, var_r, val_r, idx_r)
        self.pop_temp()  # idx_r
        self.free_if_temp(val_r)

    # ── Result types ─────────────────────────────────────────────────────

    def _compile_ok_result(self, expr):
        """Compile 'ok value' → 2-element array [0, value]."""
        val_r = self.compile_expr(expr.value)
        result_r = self.push_temp()
        self.emit_A(OP_MAKE_OK, result_r, val_r)
        self.free_if_temp(val_r)
        return result_r

    def _compile_error_result(self, expr):
        """Compile 'error msg' → 2-element array [1, msg]."""
        msg_r = self.compile_expr(expr.value)
        result_r = self.push_temp()
        self.emit_A(OP_MAKE_ERROR, result_r, msg_r)
        self.free_if_temp(msg_r)
        return result_r

    def _compile_result_propagation(self, expr):
        """Compile 'expr?' — check result, propagate error up call stack."""
        saved_temp = self.temp_top
        # Evaluate the expression that returns a result
        res_r = self.compile_expr(expr.expr)
        # Unpack: tag_r = result[0], val_r = result[1]
        tag_r = self.push_temp()
        val_r = self.push_temp()
        self.emit_A(OP_CHECK_RESULT, tag_r, val_r, res_r)
        # If error, propagate — jump to error handler
        error_jump_pc = self.current_pc()
        self.emit_B(OP_JUMP_IF_ERROR, tag_r, 0)
        # OK path: move unwrapped value to saved_temp position
        result_r = saved_temp
        if val_r != result_r:
            self.emit_A(OP_MOV, result_r, val_r)
        # Jump over error handler
        skip_error_pc = self.current_pc()
        self.emit_C(OP_JUMP, 0)
        # Error handler: re-wrap as error result and return it
        error_start = self.current_pc()
        self.patch_jump(error_jump_pc, error_start - error_jump_pc - 1)
        # In main, halt with error message. In other functions, return the error.
        if self.fn.name == 'main':
            self.emit_A(OP_ERROR_HALT, val_r)
        else:
            err_result_r = self.push_temp()
            self.emit_A(OP_MAKE_ERROR, err_result_r, val_r)
            self.emit_A(OP_RETURN, err_result_r)
            self.pop_temp()
        # Patch skip jump
        end_pc = self.current_pc()
        self.patch_jump_c(skip_error_pc, end_pc - skip_error_pc - 1)
        # Collapse temps — result is at saved_temp
        self.temp_top = saved_temp + 1
        return result_r

    # ── Struct literals ──────────────────────────────────────────────────

    def _compile_struct_literal(self, expr):
        """Compile TypeName{field1: val1, ...} as array with field slots."""
        saved_temp = self.temp_top
        layout = self.gen._object_layouts.get(expr.type_name, {})
        n = max(len(layout), len(expr.fields))
        size_r = self.push_temp()
        self.emit_B(OP_LOAD_CONST, size_r, self.gen.add_const_int(n))
        arr_r = self.push_temp()
        self.emit_A(OP_NEW_ARRAY, arr_r, 0, size_r)
        idx_r = self.push_temp()
        for fname, fval in expr.fields:
            field_idx = layout.get(fname, 0)
            self.emit_B(OP_LOAD_CONST, idx_r, self.gen.add_const_int(field_idx))
            val_r = self.compile_expr(fval)
            self.emit_A(OP_INDEX_SET, arr_r, idx_r, val_r)
            self.free_if_temp(val_r)
        result_r = saved_temp
        if arr_r != result_r:
            self.emit_A(OP_MOV, result_r, arr_r)
        self.temp_top = saved_temp + 1
        return result_r

    # ── Concurrency (sequential fallback) ────────────────────────────────

    def _compile_spawn(self, expr):
        """Compile spawn(func(args)) — sequential fallback via OP_SPAWN.
        spawn takes a Call node as its argument; we extract the inner call
        and emit OP_SPAWN instead of OP_CALL."""
        if expr.args and isinstance(expr.args[0], Call):
            inner = expr.args[0]
            func_idx = self.gen.func_names.get(inner.name)
            if func_idx is not None:
                saved_temp = self.temp_top
                n_args = len(inner.args)
                arg_regs = []
                for arg in inner.args:
                    arg_regs.append(self.compile_expr(arg))
                n_slots = max(n_args, 1)
                first_arg = self.temp_top
                for _ in range(n_slots):
                    self.push_temp()
                for i, ar in enumerate(arg_regs):
                    target = first_arg + i
                    if ar != target:
                        self.emit_A(OP_MOV, target, ar)
                # OP_SPAWN instead of OP_CALL — VM executes synchronously
                self.emit_A(OP_SPAWN, first_arg, func_idx, n_args)
                result_r = saved_temp
                if first_arg != result_r:
                    self.emit_A(OP_MOV, result_r, first_arg)
                self.temp_top = saved_temp + 1
                return result_r
        # Fallback: treat as regular expression, return 0
        r = self.push_temp()
        self.emit_B(OP_LOAD_CONST, r, self.gen.add_const_int(0))
        return r

    def _compile_await(self, expr):
        """Compile wait(handle) — sequential fallback via OP_AWAIT.
        The handle already contains the result from the synchronous spawn."""
        if expr.args:
            handle_r = self.compile_expr(expr.args[0])
            result_r = self.push_temp()
            self.emit_A(OP_AWAIT, result_r, handle_r)
            self.free_if_temp(handle_r)
            return result_r
        r = self.push_temp()
        self.emit_B(OP_LOAD_CONST, r, self.gen.add_const_int(0))
        return r


# ── Public interface (matches other codegen modules) ─────────────────────────

def generate(program, profile=False):
    """Compile a UniLogic AST to .ulb bytecode. Returns bytes."""
    gen = BytecodeGen()
    gen.compile_program(program)
    return gen.serialize()
