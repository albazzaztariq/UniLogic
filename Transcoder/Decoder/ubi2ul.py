# ubi2ul.py — UniLogic .ubi bytecode decompiler
# Reads a .ubi file (or stripped .ulb) and reconstructs readable UniLogic source.
#
# With debug section present: uses variable names, types, function signatures,
# and line number table to produce output close to the original source.
#
# Without debug section (stripped): emits generic slot names (_r0, _r1, ...) and
# infers types from instruction patterns where possible.
#
# Usage (standalone):
#   python ubi2ul.py program.ubi
#   python ubi2ul.py program.ubi -o out.ul
#   python ubi2ul.py program.ubi --no-debug    # ignore debug section even if present
#
# Usage (as library):
#   from ubi2ul import decompile_file, decompile_bytes
#   source = decompile_file("program.ubi")
#   source = decompile_bytes(data)

import struct
import sys
import os
from io import BytesIO


# ── ULEB128 decoder ──────────────────────────────────────────────────────────

def read_uleb128(stream):
    """Read one ULEB128-encoded unsigned integer from stream. Returns int."""
    result = 0
    shift = 0
    while True:
        byte = stream.read(1)
        if not byte:
            raise EOFError("Unexpected EOF reading ULEB128")
        b = byte[0]
        result |= (b & 0x7F) << shift
        if not (b & 0x80):
            break
        shift += 7
    return result


def zigzag_decode(n):
    """Decode a zigzag-encoded unsigned int back to signed."""
    if n % 2 == 0:
        return n // 2
    return -(n // 2) - 1


# ── UL type tag names (§6.3 of spec) ─────────────────────────────────────────

TYPE_TAG_NAMES = {
    0x01: 'int',
    0x02: 'float',
    0x03: 'string',
    0x04: 'bool',
    0x05: 'empty',
    0x06: 'array',
    0x07: 'type',
    0x08: 'object',
    0x09: 'Result',
    0x0A: 'list',
    0xFF: 'unknown',
}

DEBUG_MARKER = bytes([0x55, 0x42, 0x44, 0x42, 0xDB, 0x00, 0xDB, 0x00])

FLAG_HAS_TYPES       = 0x01
FLAG_HAS_LINES       = 0x02
FLAG_HAS_SOURCE_NAME = 0x04


# ── Opcode names for disassembly fallback ─────────────────────────────────────

OPCODE_NAMES = {
    0x01: 'MOV',
    0x02: 'LOAD_CONST',
    0x03: 'ADD',
    0x04: 'SUB',
    0x05: 'MUL',
    0x06: 'DIV',
    0x07: 'CMP_LT',
    0x08: 'JUMP_IF_FALSE',
    0x09: 'CALL',
    0x0A: 'RETURN',
    0x0B: 'PRINT',
    0x10: 'MOD',
    0x11: 'NEG',
    0x12: 'NOT',
    0x13: 'AND',
    0x14: 'OR',
    0x15: 'ADD_IMM',
    0x16: 'SUB_IMM',
    0x20: 'CMP_EQ',
    0x21: 'CMP_NE',
    0x22: 'CMP_GT',
    0x23: 'CMP_LE',
    0x24: 'CMP_GE',
    0x30: 'JUMP',
    0x31: 'JUMP_IF_TRUE',
    0x32: 'RETURN_NONE',
    0x40: 'LOAD_GLOBAL',
    0x41: 'STORE_GLOBAL',
    0x60: 'NEW_ARRAY',
    0x61: 'INDEX_GET',
    0x62: 'INDEX_SET',
    0x63: 'LENGTH',
    0x70: 'CAST_INT',
    0x71: 'CAST_FLOAT',
    0x72: 'CAST_STRING',
    0x73: 'CAST_BOOL',
    0xA7: 'INC_REG',
    0xA8: 'DEC_REG',
    0x90: 'CALL_BUILTIN',
    0xB0: 'MAKE_OK',
    0xB1: 'MAKE_ERROR',
    0xB2: 'CHECK_RESULT',
    0xB3: 'JUMP_IF_ERROR',
    0xB4: 'ERROR_HALT',
    0xD0: 'SPAWN',
    0xD1: 'AWAIT',
    0xD2: 'YIELD',
    0xD3: 'LOCK',
    0xD4: 'UNLOCK',
    0xFF: 'NOP',
}


# ── .ulb binary parser ────────────────────────────────────────────────────────

class ULBFile:
    """Parsed representation of the .ulb portion of a .ubi file."""

    def __init__(self):
        self.magic = 0
        self.ver_maj = 0
        self.ver_min = 0
        self.const_pool = []     # list of (tag, raw_bytes)
        self.functions = []      # list of FuncRecord
        self.instr_bytes = b''   # raw instruction stream
        self.debug_section_offset = None  # byte offset of debug marker, or None


class FuncRecord:
    def __init__(self, name, param_count, register_count, instr_offset, instr_count):
        self.name = name
        self.param_count = param_count
        self.register_count = register_count
        self.instr_offset = instr_offset    # byte offset within instr_bytes
        self.instr_count = instr_count


def _read_u8(buf, pos):
    return buf[pos], pos + 1

def _read_u16_le(buf, pos):
    return struct.unpack_from('<H', buf, pos)[0], pos + 2

def _read_u32_le(buf, pos):
    return struct.unpack_from('<I', buf, pos)[0], pos + 4

def _read_u64_le(buf, pos):
    return struct.unpack_from('<Q', buf, pos)[0], pos + 8


def parse_ulb(data):
    """Parse the .ulb portion of a .ubi file. Returns ULBFile."""
    f = ULBFile()
    pos = 0

    # Header (24 bytes)
    f.magic,    pos = _read_u32_le(data, pos)
    f.ver_maj,  pos = _read_u16_le(data, pos)
    f.ver_min,  pos = _read_u16_le(data, pos)
    const_count, pos = _read_u32_le(data, pos)
    func_count,  pos = _read_u32_le(data, pos)
    block_count, pos = _read_u32_le(data, pos)
    edge_count,  pos = _read_u32_le(data, pos)

    # Constant pool
    for _ in range(const_count):
        tag = data[pos]; pos += 1
        if tag == 0x01:   # int
            val_bytes = data[pos:pos+8]; pos += 8
        elif tag == 0x02: # float
            val_bytes = data[pos:pos+8]; pos += 8
        elif tag == 0x03: # string
            slen = struct.unpack_from('<I', data, pos)[0]; pos += 4
            val_bytes = data[pos:pos+slen]; pos += slen
        elif tag == 0x04: # bool
            val_bytes = data[pos:pos+1]; pos += 1
        elif tag == 0x05: # empty
            val_bytes = b''
        else:
            # Unknown tag — cannot parse further
            val_bytes = b''
        f.const_pool.append((tag, val_bytes))

    # Function table
    func_entries = []
    for _ in range(func_count):
        name_len = struct.unpack_from('<I', data, pos)[0]; pos += 4
        name = data[pos:pos+name_len].decode('utf-8', errors='replace'); pos += name_len
        param_count = data[pos]; pos += 1
        register_count = data[pos]; pos += 1
        instr_offset = struct.unpack_from('<I', data, pos)[0]; pos += 4
        instr_count  = struct.unpack_from('<I', data, pos)[0]; pos += 4
        _first_block  = struct.unpack_from('<I', data, pos)[0]; pos += 4  # first_block_idx
        _block_count  = struct.unpack_from('<H', data, pos)[0]; pos += 2  # block_count (u16)
        func_entries.append((name, param_count, register_count, instr_offset, instr_count))

    # Instruction stream starts at current pos
    instr_stream_start = pos
    # Total instruction bytes = sum of all function instr_count * 4
    total_instr_bytes = sum(e[4] for e in func_entries) * 4
    f.instr_bytes = data[instr_stream_start:instr_stream_start + total_instr_bytes]
    pos = instr_stream_start + total_instr_bytes

    # Build FuncRecord list with absolute offsets.
    # instr_offset in the .ulb function table is already in bytes.
    for name, param_count, register_count, instr_offset, instr_count in func_entries:
        f.functions.append(FuncRecord(name, param_count, register_count,
                                      instr_offset, instr_count))

    # Check for debug marker
    marker_pos = data.find(DEBUG_MARKER, pos)
    if marker_pos != -1:
        f.debug_section_offset = marker_pos

    return f, pos


# ── Debug section parser ──────────────────────────────────────────────────────

class DebugInfo:
    """Parsed debug section from a .ubi file."""

    def __init__(self):
        self.version = 0
        self.flags = 0
        self.source_filename = None
        self.heap = b''

        # var_map: (func_index, reg_slot) → VarInfo
        self.var_map = {}
        # func_map: func_index → FuncDebugInfo
        self.func_map = {}
        # line_map: func_index → list of (instr_index, line_number), sorted
        self.line_map = {}
        # type_map: type_index → TypeInfo
        self.type_map = {}


class VarInfo:
    __slots__ = ['name', 'ul_type_tag', 'type_ref', 'scope_start', 'scope_end']
    def __init__(self, name, ul_type_tag, type_ref, scope_start, scope_end):
        self.name = name
        self.ul_type_tag = ul_type_tag
        self.type_ref = type_ref
        self.scope_start = scope_start
        self.scope_end = scope_end


class FuncDebugInfo:
    __slots__ = ['func_index', 'name', 'params', 'return_type_tag',
                 'return_type_ref', 'first_line']
    def __init__(self, func_index, name, params, return_type_tag,
                 return_type_ref, first_line):
        self.func_index = func_index
        self.name = name
        # params: list of (reg_slot, name, ul_type_tag, type_ref)
        self.params = params
        self.return_type_tag = return_type_tag
        self.return_type_ref = return_type_ref
        self.first_line = first_line


class TypeInfo:
    __slots__ = ['type_index', 'kind', 'name', 'fields', 'variants',
                 'element_type_tag', 'element_type_ref', 'static_size']
    def __init__(self, type_index, kind, name):
        self.type_index = type_index
        self.kind = kind
        self.name = name
        self.fields = []     # list of (name, ul_type_tag, type_ref, flags) for struct/object
        self.variants = []   # list of (name, value) for enum
        self.element_type_tag = 0x01
        self.element_type_ref = 0
        self.static_size = 0


def _heap_str(heap, offset, length):
    """Extract a string from the string heap."""
    return heap[offset:offset+length].decode('utf-8', errors='replace')


def parse_debug_section(data, marker_pos):
    """Parse the debug section starting at marker_pos. Returns DebugInfo or None."""
    dbg = DebugInfo()
    pos = marker_pos + 8  # skip 8-byte marker

    if pos + 32 > len(data):
        return None

    # Debug header (32 bytes)
    dbg.version    = data[pos]; pos += 1
    dbg.flags      = data[pos]; pos += 1
    _reserved      = struct.unpack_from('<H', data, pos)[0]; pos += 2
    heap_size      = struct.unpack_from('<I', data, pos)[0]; pos += 4
    vartab_offset  = struct.unpack_from('<I', data, pos)[0]; pos += 4
    typetab_offset = struct.unpack_from('<I', data, pos)[0]; pos += 4
    functab_offset = struct.unpack_from('<I', data, pos)[0]; pos += 4
    linetab_offset = struct.unpack_from('<I', data, pos)[0]; pos += 4
    src_name_off   = struct.unpack_from('<I', data, pos)[0]; pos += 4
    src_name_len   = struct.unpack_from('<I', data, pos)[0]; pos += 4  # UBI v1.0: source name length

    # Header base = marker_pos + 8 (start of header)
    header_base = marker_pos + 8

    # String heap immediately follows the header
    heap_start = header_base + 32
    dbg.heap = data[heap_start:heap_start + heap_size]

    if dbg.flags & FLAG_HAS_SOURCE_NAME and src_name_len > 0:
        try:
            dbg.source_filename = dbg.heap[src_name_off:src_name_off + src_name_len].decode('utf-8', errors='replace')
        except Exception:
            dbg.source_filename = None

    # ── Variable Name Table ───────────────────────────────────────────────────
    vartab_base = header_base + vartab_offset
    s = BytesIO(data[vartab_base:])
    entry_count = read_uleb128(s)
    for _ in range(entry_count):
        func_index  = read_uleb128(s)
        reg_slot    = s.read(1)[0]
        name_off    = read_uleb128(s)
        name_len    = read_uleb128(s)
        ul_type_tag = s.read(1)[0]
        type_ref    = read_uleb128(s)
        scope_start = read_uleb128(s)
        scope_end   = read_uleb128(s)
        name = _heap_str(dbg.heap, name_off, name_len)
        dbg.var_map[(func_index, reg_slot)] = VarInfo(
            name, ul_type_tag, type_ref, scope_start, scope_end)

    # ── Type Table ────────────────────────────────────────────────────────────
    typetab_base = header_base + typetab_offset
    s = BytesIO(data[typetab_base:])
    type_count = read_uleb128(s)
    for _ in range(type_count):
        type_index  = read_uleb128(s)
        kind        = s.read(1)[0]
        name_off    = read_uleb128(s)
        name_len    = read_uleb128(s)
        name = _heap_str(dbg.heap, name_off, name_len)
        ti = TypeInfo(type_index, kind, name)
        if kind in (0x03, 0x04):  # array or list
            _field_count = read_uleb128(s)  # always 0
            ti.element_type_tag = s.read(1)[0]
            ti.element_type_ref = read_uleb128(s)
            ti.static_size      = read_uleb128(s)
        elif kind == 0x06:  # fixed/enum
            _field_count = read_uleb128(s)  # always 0
            variant_count = read_uleb128(s)
            for _ in range(variant_count):
                v_off = read_uleb128(s)
                v_len = read_uleb128(s)
                v_val = read_uleb128(s)
                vname = _heap_str(dbg.heap, v_off, v_len)
                ti.variants.append((vname, v_val))
        else:  # struct (0x01) or object (0x02)
            field_count = read_uleb128(s)
            for _ in range(field_count):
                _field_index = read_uleb128(s)
                f_off  = read_uleb128(s)
                f_len  = read_uleb128(s)
                f_tag  = s.read(1)[0]
                f_ref  = read_uleb128(s)
                f_flags = s.read(1)[0]
                fname = _heap_str(dbg.heap, f_off, f_len)
                ti.fields.append((fname, f_tag, f_ref, f_flags))
        dbg.type_map[type_index] = ti

    # ── Function Debug Table ──────────────────────────────────────────────────
    functab_base = header_base + functab_offset
    s = BytesIO(data[functab_base:])
    func_count = read_uleb128(s)
    for _ in range(func_count):
        func_index  = read_uleb128(s)
        name_off    = read_uleb128(s)
        name_len    = read_uleb128(s)
        fname = _heap_str(dbg.heap, name_off, name_len)
        param_count = s.read(1)[0]
        params = []
        for _ in range(param_count):
            reg_slot = s.read(1)[0]
            p_off    = read_uleb128(s)
            p_len    = read_uleb128(s)
            p_tag    = s.read(1)[0]
            p_ref    = read_uleb128(s)
            pname = _heap_str(dbg.heap, p_off, p_len)
            params.append((reg_slot, pname, p_tag, p_ref))
        ret_tag = s.read(1)[0]
        ret_ref = read_uleb128(s)
        first_line = read_uleb128(s)
        dbg.func_map[func_index] = FuncDebugInfo(
            func_index, fname, params, ret_tag, ret_ref, first_line)

    # ── Line Number Table ─────────────────────────────────────────────────────
    linetab_base = header_base + linetab_offset
    s = BytesIO(data[linetab_base:])
    func_count_lt = read_uleb128(s)
    for _ in range(func_count_lt):
        fi           = read_uleb128(s)
        entry_count  = read_uleb128(s)
        first_line   = read_uleb128(s)
        points = []
        cur_instr = 0
        cur_line  = first_line
        for _ in range(entry_count):
            instr_delta = read_uleb128(s)
            line_delta_enc = read_uleb128(s)
            cur_instr += instr_delta
            cur_line  += zigzag_decode(line_delta_enc)
            points.append((cur_instr, cur_line))
        dbg.line_map[fi] = points

    return dbg


# ── Constant value formatter ──────────────────────────────────────────────────

def fmt_const(tag, raw):
    """Format a constant pool entry as a UL literal string."""
    if tag == 0x01:  # int
        return str(struct.unpack('<q', raw)[0])
    if tag == 0x02:  # float
        v = struct.unpack('<d', raw)[0]
        s = repr(v)
        return s if '.' in s or 'e' in s else s + '.0'
    if tag == 0x03:  # string
        text = raw.decode('utf-8', errors='replace')
        return '"' + text.replace('\\', '\\\\').replace('"', '\\"') + '"'
    if tag == 0x04:  # bool
        return 'true' if raw[0] else 'false'
    if tag == 0x05:  # empty
        return 'empty'
    return f'<const tag=0x{tag:02X}>'


def ul_type_str(tag, type_ref, type_map):
    """Convert a UL type tag + type_ref into a UL type string."""
    name = TYPE_TAG_NAMES.get(tag, f'unknown_0x{tag:02X}')
    if tag == 0x05:
        return 'none'
    if tag in (0x06, 0x07, 0x08, 0x09, 0x0A) and type_ref and type_ref in type_map:
        ti = type_map[type_ref]
        if tag == 0x06:
            elem_name = TYPE_TAG_NAMES.get(ti.element_type_tag, 'int')
            size_suffix = f'[{ti.static_size}]' if ti.static_size else ''
            return f'array {elem_name}{size_suffix}'
        if tag == 0x0A:
            elem_name = TYPE_TAG_NAMES.get(ti.element_type_tag, 'int')
            return f'list {elem_name}'
        return ti.name
    return name


# ── Per-function decompiler ───────────────────────────────────────────────────

class FuncDecompiler:
    """Decompiles one function's instruction stream to UL source text."""

    def __init__(self, fi, func_idx, const_pool, dbg, gen_func_map):
        self.fi = fi                 # FuncRecord
        self.func_idx = func_idx
        self.const_pool = const_pool
        self.dbg = dbg               # DebugInfo or None
        self.gen_func_map = gen_func_map   # func_index → FuncRecord
        self.lines = []

    def reg_name(self, slot, instr_idx=0):
        """Return the source name for a register slot, or _rN if unknown."""
        if self.dbg:
            key = (self.func_idx, slot)
            vi = self.dbg.var_map.get(key)
            if vi and vi.scope_start <= instr_idx:
                if vi.scope_end == 0xFFFFFFFF or instr_idx < vi.scope_end:
                    return vi.name
        return f'_r{slot}'

    def param_name(self, slot):
        """Return the parameter name for a register slot (always visible)."""
        if self.dbg:
            fi_dbg = self.dbg.func_map.get(self.func_idx)
            if fi_dbg:
                for reg_slot, pname, _, _ in fi_dbg.params:
                    if reg_slot == slot:
                        return pname
            key = (self.func_idx, slot)
            vi = self.dbg.var_map.get(key)
            if vi:
                return vi.name
        return f'_r{slot}'

    def source_line(self, instr_idx):
        """Return the source line for instr_idx, or 0 if unknown."""
        if not self.dbg:
            return 0
        points = self.dbg.line_map.get(self.func_idx, [])
        last_line = 0
        for iidx, line in points:
            if iidx > instr_idx:
                break
            last_line = line
        return last_line

    def var_type_str(self, slot, instr_idx=0):
        """Return the UL type string for a register slot."""
        if self.dbg:
            key = (self.func_idx, slot)
            vi = self.dbg.var_map.get(key)
            if vi:
                return ul_type_str(vi.ul_type_tag, vi.type_ref,
                                   self.dbg.type_map)
        return 'int'

    def decompile(self):
        """Return reconstructed UL source for this function as a list of lines."""
        out = []
        fi_dbg = self.dbg.func_map.get(self.func_idx) if self.dbg else None

        # ── Function signature ────────────────────────────────────────────────
        if fi_dbg:
            fname = fi_dbg.name
            params_parts = []
            for reg_slot, pname, p_tag, p_ref in fi_dbg.params:
                if pname == '__self__':
                    continue
                ptype = ul_type_str(p_tag, p_ref,
                                    self.dbg.type_map if self.dbg else {})
                params_parts.append(f'{pname} {ptype}')
            ret_tag = fi_dbg.return_type_tag
            if ret_tag == 0x05 or ret_tag == 0:
                ret_clause = ''
            else:
                ret_type = ul_type_str(ret_tag, fi_dbg.return_type_ref,
                                       self.dbg.type_map if self.dbg else {})
                ret_clause = f' returns {ret_type}'
            sig = f'function {fname}({", ".join(params_parts)}){ret_clause}'
        else:
            fname = self.fi.name
            # Fall back: number params from slot 0..param_count-1
            params_parts = [f'_r{i} int' for i in range(self.fi.param_count)]
            sig = f'function {fname}({", ".join(params_parts)})'

        out.append(sig)

        # ── Instruction decompilation ─────────────────────────────────────────
        instrs = self.fi
        raw = self.fi
        instr_bytes = self._func_bytes()
        n = len(instr_bytes) // 4

        # Track which slots have been declared already (to emit decl once)
        declared = set(range(self.fi.param_count))
        # For object methods with self, slot 0 is implicit
        if fi_dbg:
            for reg_slot, pname, _, _ in fi_dbg.params:
                declared.add(reg_slot)

        indent = '    '
        prev_line = 0

        for i in range(n):
            word = struct.unpack_from('<I', instr_bytes, i * 4)[0]
            op   = word & 0xFF
            rA   = (word >> 8) & 0xFF
            rB   = (word >> 16) & 0xFF
            rC   = (word >> 24) & 0xFF
            imm16 = (word >> 16) & 0xFFFF
            # Signed imm16
            simm16 = imm16 if imm16 < 0x8000 else imm16 - 0x10000
            # Form C imm24 (signed)
            raw24  = (word >> 8) & 0xFFFFFF
            simm24 = raw24 if raw24 < 0x800000 else raw24 - 0x1000000

            src_line = self.source_line(i)
            line_comment = f'  -- line {src_line}' if src_line and src_line != prev_line else ''
            if src_line:
                prev_line = src_line

            a = self.reg_name(rA, i)
            b = self.reg_name(rB, i)
            c = self.reg_name(rC, i)

            # Check if rA needs a declaration
            def maybe_decl(slot, instr_i):
                if slot not in declared:
                    declared.add(slot)
                    vtype = self.var_type_str(slot, instr_i)
                    vname = self.reg_name(slot, instr_i)
                    return f'{indent}{vtype} {vname} = '
                return None

            def emit(text, comment=''):
                full = indent + text
                if comment:
                    full += comment
                out.append(full)

            # ── Decode each opcode into approximate UL syntax ─────────────────
            if op == 0x01:   # MOV rA, rB
                decl = maybe_decl(rA, i)
                if decl:
                    out.append(decl + b + line_comment)
                else:
                    emit(f'{a} = {b}', line_comment)

            elif op == 0x02: # LOAD_CONST rA, imm16
                const_str = self._const_str(imm16)
                decl = maybe_decl(rA, i)
                if decl:
                    out.append(decl + const_str + line_comment)
                else:
                    emit(f'{a} = {const_str}', line_comment)

            elif op in (0x03, 0x04, 0x05, 0x06, 0x10,  # ADD SUB MUL DIV MOD
                        0x07, 0x20, 0x21, 0x22, 0x23, 0x24,  # CMP*
                        0x13, 0x14):  # AND OR
                op_sym = {
                    0x03: '+', 0x04: '-', 0x05: '*', 0x06: '/',
                    0x10: '%', 0x07: '<', 0x22: '>', 0x23: '<=', 0x24: '>=',
                    0x20: '==', 0x21: '!=', 0x13: 'and', 0x14: 'or',
                }.get(op, '?')
                decl = maybe_decl(rA, i)
                if decl:
                    out.append(decl + f'{b} {op_sym} {c}' + line_comment)
                else:
                    emit(f'{a} = {b} {op_sym} {c}', line_comment)

            elif op == 0x11: # NEG rA, rB
                decl = maybe_decl(rA, i)
                if decl:
                    out.append(decl + f'-{b}' + line_comment)
                else:
                    emit(f'{a} = -{b}', line_comment)

            elif op == 0x12: # NOT rA, rB
                decl = maybe_decl(rA, i)
                if decl:
                    out.append(decl + f'not {b}' + line_comment)
                else:
                    emit(f'{a} = not {b}', line_comment)

            elif op == 0x08: # JUMP_IF_FALSE rA, imm16
                target = i + 1 + simm16
                emit(f'-- if not {a}: jump to instr {target}', line_comment)

            elif op == 0x31: # JUMP_IF_TRUE rA, imm16
                target = i + 1 + simm16
                emit(f'-- if {a}: jump to instr {target}', line_comment)

            elif op == 0x30: # JUMP imm24
                target = i + 1 + simm24
                emit(f'-- jump to instr {target}', line_comment)

            elif op == 0x09: # CALL rA, func_idx, arg_count
                callee_idx = rB
                arg_count  = rC
                callee_name = (self.gen_func_map[callee_idx].name
                               if callee_idx < len(self.gen_func_map) else f'func_{callee_idx}')
                args = ', '.join(self.reg_name(rA + k, i) for k in range(arg_count))
                decl = maybe_decl(rA, i)
                if decl:
                    out.append(decl + f'{callee_name}({args})' + line_comment)
                else:
                    emit(f'{a} = {callee_name}({args})', line_comment)

            elif op == 0x90: # CALL_BUILTIN rA, builtin_id, arg_count
                builtin_id = rB
                arg_count  = rC
                args = ', '.join(self.reg_name(rA + k, i) for k in range(arg_count))
                decl = maybe_decl(rA, i)
                if decl:
                    out.append(decl + f'builtin_{builtin_id}({args})' + line_comment)
                else:
                    emit(f'{a} = builtin_{builtin_id}({args})', line_comment)

            elif op == 0x0A: # RETURN rA
                emit(f'return {a}', line_comment)

            elif op == 0x32: # RETURN_NONE
                emit('return', line_comment)

            elif op == 0x0B: # PRINT rA
                emit(f'print {a}', line_comment)

            elif op == 0x40: # LOAD_GLOBAL rA, imm16
                decl = maybe_decl(rA, i)
                if decl:
                    out.append(decl + f'_global[{imm16}]' + line_comment)
                else:
                    emit(f'{a} = _global[{imm16}]', line_comment)

            elif op == 0x41: # STORE_GLOBAL rA, imm16
                emit(f'_global[{imm16}] = {a}', line_comment)

            elif op == 0x60: # NEW_ARRAY rA, _, size_reg
                size_r = self.reg_name(rC, i)
                decl = maybe_decl(rA, i)
                if decl:
                    out.append(decl + f'new array[{size_r}]' + line_comment)
                else:
                    emit(f'{a} = new array[{size_r}]', line_comment)

            elif op == 0x61: # INDEX_GET rA, arr, idx
                decl = maybe_decl(rA, i)
                if decl:
                    out.append(decl + f'{b}[{c}]' + line_comment)
                else:
                    emit(f'{a} = {b}[{c}]', line_comment)

            elif op == 0x62: # INDEX_SET arr, idx, val
                emit(f'{a}[{b}] = {c}', line_comment)

            elif op == 0x63: # LENGTH rA, arr
                decl = maybe_decl(rA, i)
                if decl:
                    out.append(decl + f'len({b})' + line_comment)
                else:
                    emit(f'{a} = len({b})', line_comment)

            elif op in (0x70, 0x71, 0x72, 0x73):  # CAST_*
                cast_name = {0x70: 'int', 0x71: 'float',
                             0x72: 'string', 0x73: 'bool'}[op]
                decl = maybe_decl(rA, i)
                if decl:
                    out.append(decl + f'{cast_name}({b})' + line_comment)
                else:
                    emit(f'{a} = {cast_name}({b})', line_comment)

            elif op == 0xA7: # INC_REG rA
                emit(f'{a}++', line_comment)

            elif op == 0xA8: # DEC_REG rA
                emit(f'{a}--', line_comment)

            elif op == 0xB0: # MAKE_OK rA, rB
                decl = maybe_decl(rA, i)
                if decl:
                    out.append(decl + f'ok {b}' + line_comment)
                else:
                    emit(f'{a} = ok {b}', line_comment)

            elif op == 0xB1: # MAKE_ERROR rA, rB
                decl = maybe_decl(rA, i)
                if decl:
                    out.append(decl + f'error {b}' + line_comment)
                else:
                    emit(f'{a} = error {b}', line_comment)

            elif op == 0xB2: # CHECK_RESULT tag_dst, val_dst, res_reg
                emit(f'{a}, {b} = check_result({c})', line_comment)

            elif op == 0xB3: # JUMP_IF_ERROR tag_reg, imm16
                target = i + 1 + simm16
                emit(f'-- if {a} is error: jump to instr {target}', line_comment)

            elif op == 0xB4: # ERROR_HALT rA
                emit(f'-- error_halt({a})', line_comment)

            elif op == 0xD0: # SPAWN
                callee_idx = rB
                arg_count  = rC
                callee_name = (self.gen_func_map[callee_idx].name
                               if callee_idx < len(self.gen_func_map) else f'func_{callee_idx}')
                args = ', '.join(self.reg_name(rA + k, i) for k in range(arg_count))
                decl = maybe_decl(rA, i)
                if decl:
                    out.append(decl + f'spawn({callee_name}({args}))' + line_comment)
                else:
                    emit(f'{a} = spawn({callee_name}({args}))', line_comment)

            elif op == 0xD1: # AWAIT
                emit(f'{a} = wait({b})', line_comment)

            elif op == 0xD2: # YIELD
                emit('yield_now()', line_comment)

            elif op == 0xFF: # NOP
                emit('-- nop', line_comment)

            else:
                opname = OPCODE_NAMES.get(op, f'0x{op:02X}')
                emit(f'-- {opname} r{rA}, r{rB}, r{rC}', line_comment)

        out.append('end function')
        out.append('')
        return out

    def _func_bytes(self):
        """Return the raw instruction bytes for this function."""
        start = self.fi.instr_offset
        end   = start + self.fi.instr_count * 4
        return self.gen_func_map._instr_bytes[start:end]

    def _const_str(self, pool_idx):
        if pool_idx < len(self.const_pool):
            tag, raw = self.const_pool[pool_idx]
            return fmt_const(tag, raw)
        return f'<const {pool_idx}>'


# ── Top-level decompiler ──────────────────────────────────────────────────────

class FuncMap:
    """Wrapper around a list of FuncRecord that also holds instr_bytes."""
    def __init__(self, functions, instr_bytes):
        self._funcs = functions
        self._instr_bytes = instr_bytes

    def __len__(self):
        return len(self._funcs)

    def __getitem__(self, idx):
        return self._funcs[idx]


def decompile_bytes(data, use_debug=True):
    """Decompile a .ubi binary (bytes) to UL source text.

    Args:
        data:      Raw bytes of a .ubi or .ulb file.
        use_debug: If False, ignore the debug section even if present.

    Returns:
        str — reconstructed UniLogic source code.
    """
    ulb, _ = parse_ulb(data)
    func_map = FuncMap(ulb.functions, ulb.instr_bytes)

    dbg = None
    if use_debug and ulb.debug_section_offset is not None:
        try:
            dbg = parse_debug_section(data, ulb.debug_section_offset)
        except Exception as e:
            dbg = None
            # Continue without debug info rather than crashing

    output_lines = []

    if dbg and dbg.source_filename:
        output_lines.append(f'-- Decompiled from: {dbg.source_filename}')
        output_lines.append('')
    elif dbg:
        output_lines.append('-- Decompiled from .ubi (debug info present)')
        output_lines.append('')
    else:
        output_lines.append('-- Decompiled from .ubi (no debug info, using slot names)')
        output_lines.append('')

    # Emit type definitions from debug info
    if dbg and dbg.type_map:
        for tidx in sorted(dbg.type_map.keys()):
            ti = dbg.type_map[tidx]
            if ti.kind == 0x01:   # struct/type
                output_lines.append(f'type {ti.name}')
                for fname, ftag, fref, _ in ti.fields:
                    ftype = ul_type_str(ftag, fref, dbg.type_map)
                    output_lines.append(f'    {ftype} {fname}')
                output_lines.append('end type')
                output_lines.append('')
            elif ti.kind == 0x02:  # object
                output_lines.append(f'object {ti.name}')
                for fname, ftag, fref, _ in ti.fields:
                    ftype = ul_type_str(ftag, fref, dbg.type_map)
                    output_lines.append(f'    {ftype} {fname}')
                output_lines.append('end object')
                output_lines.append('')
            elif ti.kind == 0x06:  # enum/fixed
                output_lines.append(f'fixed {ti.name}')
                for vname, vval in ti.variants:
                    output_lines.append(f'    {vname} = {vval}')
                output_lines.append('end fixed')
                output_lines.append('')

    # Decompile each function
    for func_idx, fi in enumerate(ulb.functions):
        fd = FuncDecompiler(fi, func_idx, ulb.const_pool, dbg, func_map)
        func_lines = fd.decompile()
        output_lines.extend(func_lines)

    return '\n'.join(output_lines)


def decompile_file(path, use_debug=True):
    """Decompile a .ubi file. Returns reconstructed UL source as a string."""
    with open(path, 'rb') as f:
        data = f.read()
    return decompile_bytes(data, use_debug=use_debug)


# ── CLI entry point ───────────────────────────────────────────────────────────

def main():
    import argparse
    parser = argparse.ArgumentParser(
        description='ubi2ul — UniLogic .ubi bytecode decompiler')
    parser.add_argument('input', help='Input .ubi file')
    parser.add_argument('-o', '--output', default=None,
                        help='Output .ul file (default: stdout)')
    parser.add_argument('--no-debug', action='store_true',
                        help='Ignore debug section even if present')
    args = parser.parse_args()

    if not os.path.isfile(args.input):
        print(f'ubi2ul: error: file not found: {args.input}', file=sys.stderr)
        sys.exit(1)

    source = decompile_file(args.input, use_debug=not args.no_debug)

    if args.output:
        with open(args.output, 'w', encoding='utf-8') as f:
            f.write(source)
        print(f'ubi2ul: wrote {args.output}')
    else:
        print(source)


if __name__ == '__main__':
    main()
