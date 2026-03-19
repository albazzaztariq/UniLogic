// ulvm.cpp — UniLogic Bytecode Virtual Machine
// Reads .ulb files and executes them.
// Build: g++ -O2 -o ulvm VM/ulvm.cpp
// Usage: ulvm program.ulb

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <cctype>
#include <algorithm>
#include <vector>
#include <string>
#include <unordered_map>

// ── Magic & version ─────────────────────────────────────────────────────────

static constexpr uint32_t ULB_MAGIC   = 0x554C4243; // "ULBC"
static constexpr uint16_t ULB_VER_MAJ = 0x0100;

// ── Opcodes ─────────────────────────────────────────────────────────────────

enum Op : uint8_t {
    // Hot path
    OP_MOV            = 0x01,
    OP_LOAD_CONST     = 0x02,
    OP_ADD            = 0x03,
    OP_SUB            = 0x04,
    OP_MUL            = 0x05,
    OP_DIV            = 0x06,
    OP_CMP_LT         = 0x07,
    OP_JUMP_IF_FALSE  = 0x08,
    OP_CALL           = 0x09,
    OP_RETURN         = 0x0A,
    OP_PRINT          = 0x0B,

    // Arithmetic/logic
    OP_MOD            = 0x10,
    OP_NEG            = 0x11,
    OP_NOT            = 0x12,
    OP_AND            = 0x13,
    OP_OR             = 0x14,
    OP_ADD_IMM        = 0x15,
    OP_SUB_IMM        = 0x16,

    // Comparison
    OP_CMP_EQ         = 0x20,
    OP_CMP_NE         = 0x21,
    OP_CMP_GT         = 0x22,
    OP_CMP_LE         = 0x23,
    OP_CMP_GE         = 0x24,

    // Control flow
    OP_JUMP           = 0x30,
    OP_JUMP_IF_TRUE   = 0x31,
    OP_RETURN_NONE    = 0x32,

    // Globals
    OP_LOAD_GLOBAL    = 0x40,
    OP_STORE_GLOBAL   = 0x41,

    // Struct/array access
    OP_LOAD_STRUCT_FIELD  = 0x48,
    OP_LOAD_ARRAY_SEQ     = 0x49,
    OP_LOAD_ARRAY_STRIDE  = 0x4A,
    OP_STORE_STRUCT_FIELD = 0x4B,

    // Kill bit variants
    OP_ADD_K          = 0x50,
    OP_ADD_KK         = 0x51,
    OP_SUB_K          = 0x52,
    OP_SUB_KK         = 0x53,
    OP_MUL_K          = 0x54,
    OP_MUL_KK         = 0x55,
    OP_DIV_K          = 0x56,
    OP_DIV_KK         = 0x57,
    OP_CMP_LT_K       = 0x58,
    OP_CMP_LT_KK      = 0x59,

    // Arrays
    OP_NEW_ARRAY      = 0x60,
    OP_INDEX_GET      = 0x61,
    OP_INDEX_SET      = 0x62,
    OP_LENGTH         = 0x63,

    // Type casts
    OP_CAST_INT       = 0x70,
    OP_CAST_FLOAT     = 0x71,
    OP_CAST_STRING    = 0x72,
    OP_CAST_BOOL      = 0x73,

    // Builtin dispatch
    OP_CALL_BUILTIN   = 0x90,

    // Result types
    OP_MAKE_OK        = 0xB0,
    OP_MAKE_ERROR     = 0xB1,
    OP_CHECK_RESULT   = 0xB2,
    OP_JUMP_IF_ERROR  = 0xB3,
    OP_ERROR_HALT     = 0xB4,

    // Tier 1 — semantic operations
    OP_CALL_IC        = 0x80,
    OP_FIELD_GET      = 0x81,
    OP_FIELD_SET      = 0x82,
    OP_ITER_INIT      = 0x83,
    OP_ITER_NEXT      = 0x84,

    // Tier 2 — superinstructions
    OP_ADD_REGS       = 0xA0,
    OP_LOOP_TEST      = 0xA1,
    OP_RETURN_REG     = 0xA2,
    OP_CALL_STORE     = 0xA3,
    OP_STRUCT_COPY    = 0xA4,
    OP_BULK_ZERO      = 0xA5,
    OP_SLICE          = 0xA6,
    OP_INC_REG        = 0xA7,
    OP_DEC_REG        = 0xA8,
    OP_CMP_JUMP_EQ    = 0xA9,

    // Quickened type variants (VM-internal)
    OP_ADD_INT        = 0xC0,
    OP_ADD_FLOAT      = 0xC1,
    OP_SUB_INT        = 0xC2,
    OP_SUB_FLOAT      = 0xC3,
    OP_MUL_INT        = 0xC4,
    OP_MUL_FLOAT      = 0xC5,
    OP_DIV_INT        = 0xC6,
    OP_DIV_FLOAT      = 0xC7,
    OP_CMP_LT_INT     = 0xC8,
    OP_CMP_LT_FLOAT   = 0xC9,
    OP_CMP_EQ_INT     = 0xCA,
    OP_CMP_EQ_FLOAT   = 0xCB,
    OP_LOAD_CONST_INT   = 0xCC,
    OP_LOAD_CONST_FLOAT = 0xCD,

    OP_NOP            = 0xFF,
};

// ── Value representation ────────────────────────────────────────────────────

enum class VType : uint8_t {
    Empty  = 0,
    Int    = 1,
    Float  = 2,
    String = 3,
    Bool   = 4,
    Array  = 5,
};

struct Value {
    VType type;
    union {
        int64_t  i;
        double   f;
        bool     b;
    };
    std::string  s;                   // used when type == String
    std::vector<Value>* arr = nullptr; // used when type == Array

    Value() : type(VType::Empty), i(0) {}

    static Value make_int(int64_t v)    { Value r; r.type = VType::Int;   r.i = v; return r; }
    static Value make_float(double v)   { Value r; r.type = VType::Float; r.f = v; return r; }
    static Value make_bool(bool v)      { Value r; r.type = VType::Bool;  r.b = v; return r; }
    static Value make_empty()           { Value r; r.type = VType::Empty; r.i = 0; return r; }
    static Value make_string(const std::string& v) {
        Value r; r.type = VType::String; r.s = v; r.i = 0; return r;
    }
    static Value make_array(std::vector<Value>* v) {
        Value r; r.type = VType::Array; r.arr = v; r.i = 0; return r;
    }

    bool truthy() const {
        switch (type) {
            case VType::Empty:  return false;
            case VType::Bool:   return b;
            case VType::Int:    return i != 0;
            case VType::Float:  return f != 0.0;
            case VType::String: return !s.empty();
            case VType::Array:  return arr && !arr->empty();
        }
        return false;
    }

    int64_t as_int() const {
        switch (type) {
            case VType::Int:   return i;
            case VType::Float: return (int64_t)f;
            case VType::Bool:  return b ? 1 : 0;
            default:           return 0;
        }
    }

    double as_float() const {
        switch (type) {
            case VType::Float: return f;
            case VType::Int:   return (double)i;
            case VType::Bool:  return b ? 1.0 : 0.0;
            default:           return 0.0;
        }
    }

    std::string as_string() const {
        switch (type) {
            case VType::String: return s;
            case VType::Int: {
                char buf[32]; snprintf(buf, sizeof(buf), "%lld", (long long)i);
                return buf;
            }
            case VType::Float: {
                char buf[64]; snprintf(buf, sizeof(buf), "%f", f);
                return buf;
            }
            case VType::Bool:  return b ? "true" : "false";
            case VType::Empty: return "empty";
            case VType::Array: return "[array]";
        }
        return "";
    }
};

// ── Arithmetic helpers ──────────────────────────────────────────────────────

static inline Value val_add(const Value& a, const Value& b) {
    if (a.type == VType::Int && b.type == VType::Int)
        return Value::make_int(a.i + b.i);
    if (a.type == VType::Float || b.type == VType::Float)
        return Value::make_float(a.as_float() + b.as_float());
    if (a.type == VType::String || b.type == VType::String)
        return Value::make_string(a.as_string() + b.as_string());
    return Value::make_int(a.as_int() + b.as_int());
}

static inline Value val_sub(const Value& a, const Value& b) {
    if (a.type == VType::Float || b.type == VType::Float)
        return Value::make_float(a.as_float() - b.as_float());
    return Value::make_int(a.as_int() - b.as_int());
}

static inline Value val_mul(const Value& a, const Value& b) {
    if (a.type == VType::Float || b.type == VType::Float)
        return Value::make_float(a.as_float() * b.as_float());
    return Value::make_int(a.as_int() * b.as_int());
}

static inline Value val_div(const Value& a, const Value& b) {
    if (a.type == VType::Float || b.type == VType::Float) {
        double d = b.as_float();
        if (d == 0.0) { fprintf(stderr, "error: division by zero\n"); exit(1); }
        return Value::make_float(a.as_float() / d);
    }
    int64_t d = b.as_int();
    if (d == 0) { fprintf(stderr, "error: division by zero\n"); exit(1); }
    return Value::make_int(a.as_int() / d);
}

static inline Value val_mod(const Value& a, const Value& b) {
    if (a.type == VType::Float || b.type == VType::Float)
        return Value::make_float(fmod(a.as_float(), b.as_float()));
    int64_t d = b.as_int();
    if (d == 0) { fprintf(stderr, "error: modulo by zero\n"); exit(1); }
    return Value::make_int(a.as_int() % d);
}

static inline Value val_cmp_lt(const Value& a, const Value& b) {
    if (a.type == VType::Float || b.type == VType::Float)
        return Value::make_bool(a.as_float() < b.as_float());
    return Value::make_bool(a.as_int() < b.as_int());
}

static inline Value val_cmp_gt(const Value& a, const Value& b) {
    if (a.type == VType::Float || b.type == VType::Float)
        return Value::make_bool(a.as_float() > b.as_float());
    return Value::make_bool(a.as_int() > b.as_int());
}

static inline Value val_cmp_le(const Value& a, const Value& b) {
    if (a.type == VType::Float || b.type == VType::Float)
        return Value::make_bool(a.as_float() <= b.as_float());
    return Value::make_bool(a.as_int() <= b.as_int());
}

static inline Value val_cmp_ge(const Value& a, const Value& b) {
    if (a.type == VType::Float || b.type == VType::Float)
        return Value::make_bool(a.as_float() >= b.as_float());
    return Value::make_bool(a.as_int() >= b.as_int());
}

static inline Value val_cmp_eq(const Value& a, const Value& b) {
    // Empty comparisons: empty == empty is true, empty == anything_else is false
    if (a.type == VType::Empty || b.type == VType::Empty)
        return Value::make_bool(a.type == b.type);
    if (a.type == VType::String && b.type == VType::String)
        return Value::make_bool(a.s == b.s);
    if (a.type == VType::Float || b.type == VType::Float)
        return Value::make_bool(a.as_float() == b.as_float());
    return Value::make_bool(a.as_int() == b.as_int());
}

static inline Value val_cmp_ne(const Value& a, const Value& b) {
    Value eq = val_cmp_eq(a, b);
    return Value::make_bool(!eq.b);
}

// ── Print a value to stdout ─────────────────────────────────────────────────

static void print_value(const Value& v) {
    switch (v.type) {
        case VType::Int:    printf("%lld\n", (long long)v.i); break;
        case VType::Float:  printf("%f\n", v.f); break;
        case VType::String: printf("%s\n", v.s.c_str()); break;
        case VType::Bool:   printf("%d\n", v.b ? 1 : 0); break;
        case VType::Empty:  printf("empty\n"); break;
        case VType::Array:  printf("[array]\n"); break;
    }
}

// ── Binary reader ───────────────────────────────────────────────────────────

class Reader {
    const uint8_t* data;
    size_t size;
    size_t pos;
public:
    Reader(const uint8_t* d, size_t sz) : data(d), size(sz), pos(0) {}

    bool has(size_t n) const { return pos + n <= size; }
    size_t tell() const { return pos; }
    const uint8_t* ptr() const { return data + pos; }

    uint8_t  u8()  { uint8_t  v = data[pos]; pos += 1; return v; }
    uint16_t u16() { uint16_t v; memcpy(&v, data + pos, 2); pos += 2; return v; }
    uint32_t u32() { uint32_t v; memcpy(&v, data + pos, 4); pos += 4; return v; }
    int64_t  i64() { int64_t  v; memcpy(&v, data + pos, 8); pos += 8; return v; }
    double   f64() { double   v; memcpy(&v, data + pos, 8); pos += 8; return v; }

    std::string str(size_t len) {
        std::string s((const char*)data + pos, len);
        pos += len;
        return s;
    }

    void skip(size_t n) { pos += n; }
};

// ── Module structures ───────────────────────────────────────────────────────

struct FuncEntry {
    std::string name;
    uint8_t  param_count;
    uint8_t  register_count;
    uint32_t instr_offset;     // byte offset into instruction section
    uint32_t instr_count;      // number of 32-bit instructions
    uint32_t first_block_idx;
    uint16_t block_count;
};

struct Module {
    std::vector<Value>     const_pool;
    std::vector<FuncEntry> functions;
    std::unordered_map<std::string, uint32_t> func_by_name; // name → index
    const uint8_t*         code;       // pointer to start of instruction bytes
    size_t                 code_size;  // total bytes of instructions
};

// ── Loader ──────────────────────────────────────────────────────────────────

static Module load_module(const uint8_t* data, size_t size) {
    Reader r(data, size);
    Module mod;

    // Header
    uint32_t magic = r.u32();
    if (magic != ULB_MAGIC) {
        fprintf(stderr, "error: bad magic 0x%08X (expected 0x%08X)\n", magic, ULB_MAGIC);
        exit(1);
    }
    uint16_t ver_maj = r.u16();
    uint16_t ver_min = r.u16();
    (void)ver_min;
    if (ver_maj != ULB_VER_MAJ) {
        fprintf(stderr, "error: unsupported version %d.%d\n", ver_maj >> 8, ver_maj & 0xFF);
        exit(1);
    }

    uint32_t const_count = r.u32();
    uint32_t func_count  = r.u32();
    uint32_t block_count = r.u32();
    uint32_t edge_count  = r.u32();

    // Constant pool
    mod.const_pool.reserve(const_count);
    for (uint32_t i = 0; i < const_count; i++) {
        uint8_t tag = r.u8();
        switch (tag) {
            case 0x01: mod.const_pool.push_back(Value::make_int(r.i64()));   break;
            case 0x02: mod.const_pool.push_back(Value::make_float(r.f64())); break;
            case 0x03: {
                uint32_t len = r.u32();
                mod.const_pool.push_back(Value::make_string(r.str(len)));
                break;
            }
            case 0x04: mod.const_pool.push_back(Value::make_bool(r.u8() != 0)); break;
            case 0x05: mod.const_pool.push_back(Value::make_empty()); break;
            default:
                fprintf(stderr, "error: unknown constant tag 0x%02X at pool index %u\n", tag, i);
                exit(1);
        }
    }

    // Function table
    mod.functions.resize(func_count);
    for (uint32_t i = 0; i < func_count; i++) {
        FuncEntry& fn = mod.functions[i];
        uint32_t name_len = r.u32();
        fn.name            = r.str(name_len);
        fn.param_count     = r.u8();
        fn.register_count  = r.u8();
        fn.instr_offset    = r.u32();
        fn.instr_count     = r.u32();
        fn.first_block_idx = r.u32();
        fn.block_count     = r.u16();
        mod.func_by_name[fn.name] = i;
    }

    // Block metadata — skip for now (used by JIT, not interpreter)
    r.skip((size_t)block_count * 16);

    // Control flow edges — skip for now
    r.skip((size_t)edge_count * 12);

    // Remaining bytes are the instruction stream
    mod.code      = r.ptr();
    mod.code_size = size - r.tell();

    return mod;
}

// ── Memory modes ────────────────────────────────────────────────────────────

enum class MemMode { Default, Arena, GC, Refcount };

// Arena allocator — bump allocator, bulk free on exit
struct Arena {
    std::vector<char> memory;
    size_t offset = 0;
    std::vector<std::vector<Value>*> tracked_arrays;

    Arena(size_t capacity = 64 * 1024 * 1024) : memory(capacity) {}

    void* alloc(size_t size) {
        size = (size + 7) & ~7;
        if (offset + size > memory.size()) {
            fprintf(stderr, "VM arena out of memory\n");
            exit(1);
        }
        void* ptr = memory.data() + offset;
        offset += size;
        return ptr;
    }

    std::vector<Value>* alloc_array(size_t n) {
        auto* v = new std::vector<Value>(n);
        tracked_arrays.push_back(v);
        return v;
    }

    void reset() {
        offset = 0;
        for (auto* a : tracked_arrays) delete a;
        tracked_arrays.clear();
    }

    ~Arena() {
        for (auto* a : tracked_arrays) delete a;
    }
};

// Simple mark-and-sweep GC for VM
struct GCObject {
    bool marked = false;
    std::vector<Value>* data = nullptr;
    GCObject(std::vector<Value>* d) : data(d) {}
    ~GCObject() { delete data; }
};

// Forward declare Frame for GC::collect signature
struct Frame;

class GC {
    std::vector<GCObject*> objects;
    size_t alloc_count = 0;
    static constexpr size_t GC_THRESHOLD = 100;

public:
    std::vector<Value>* alloc_array(size_t n) {
        auto* data = new std::vector<Value>(n);
        objects.push_back(new GCObject(data));
        alloc_count++;
        return data;
    }

    bool should_collect() const { return alloc_count >= GC_THRESHOLD; }

    void mark_value(const Value& v) {
        if (v.type == VType::Array && v.arr) {
            for (auto* obj : objects) {
                if (obj->data == v.arr && !obj->marked) {
                    obj->marked = true;
                    for (const auto& elem : *v.arr)
                        mark_value(elem);
                    break;
                }
            }
        }
    }

    void collect(const std::vector<Value>& globals, const std::vector<Frame>& stack);

    ~GC() {
        for (auto* obj : objects) delete obj;
    }
};

// Reference counting for VM heap values
struct HeapValue {
    int refcount;
    std::vector<Value>* data;
    HeapValue(std::vector<Value>* d) : refcount(1), data(d) {}
    ~HeapValue() { delete data; }
};

class RefCount {
    std::unordered_map<std::vector<Value>*, HeapValue*> heap_map;
public:
    std::vector<Value>* alloc_array(size_t n) {
        auto* data = new std::vector<Value>(n);
        heap_map[data] = new HeapValue(data);
        return data;
    }

    void retain(const Value& v) {
        if (v.type == VType::Array && v.arr) {
            auto it = heap_map.find(v.arr);
            if (it != heap_map.end()) it->second->refcount++;
        }
    }

    void release(const Value& v) {
        if (v.type == VType::Array && v.arr) {
            auto it = heap_map.find(v.arr);
            if (it != heap_map.end()) {
                if (--it->second->refcount <= 0) {
                    delete it->second;
                    heap_map.erase(it);
                }
            }
        }
    }

    ~RefCount() {
        for (auto& [k, hv] : heap_map) delete hv;
    }
};

// ── Call frame ──────────────────────────────────────────────────────────────

struct Frame {
    const FuncEntry* func;
    Value            regs[256];
    uint32_t         pc;       // instruction index within this function
    uint8_t          ret_reg;  // caller's register to receive return value
};

// GC::collect implementation (needs Frame to be defined)
void GC::collect(const std::vector<Value>& globals, const std::vector<Frame>& stack) {
    // Mark phase — scan globals and all registers on the call stack
    for (const auto& g : globals) mark_value(g);
    for (const auto& frame : stack) {
        for (int i = 0; i < 256; i++) mark_value(frame.regs[i]);
    }
    // Sweep phase — delete unreachable objects
    auto it = objects.begin();
    while (it != objects.end()) {
        if (!(*it)->marked) {
            delete *it;
            it = objects.erase(it);
        } else {
            (*it)->marked = false;
            ++it;
        }
    }
    alloc_count = 0;
}

// ── VM ──────────────────────────────────────────────────────────────────────

class VM {
    Module&              mod;
    std::vector<Frame>   call_stack;
    std::vector<Value>   globals;
    // Default array pool (used when neither arena nor GC is active)
    std::vector<std::vector<Value>*> array_pool;
    // Memory mode
    MemMode mem_mode;
    Arena*    arena;
    GC*       gc;
    RefCount* rc;

public:
    VM(Module& m, MemMode mode = MemMode::Default)
        : mod(m), mem_mode(mode), arena(nullptr), gc(nullptr), rc(nullptr)
    {
        globals.resize(256);
        call_stack.reserve(256); // prevent reallocation from invalidating frame refs
        if (mode == MemMode::Arena)    arena = new Arena();
        if (mode == MemMode::GC)       gc = new GC();
        if (mode == MemMode::Refcount) rc = new RefCount();
    }

    ~VM() {
        for (auto* a : array_pool) delete a;
        delete arena;
        delete gc;
        delete rc;
    }

    int run() {
        auto it = mod.func_by_name.find("main");
        if (it == mod.func_by_name.end()) {
            fprintf(stderr, "error: no main() function in module\n");
            return 1;
        }
        return call_function(it->second, nullptr, 0, 0);
    }

private:

    // Decode helpers — read from the 32-bit instruction word
    static uint8_t  dec_op(uint32_t w)  { return (uint8_t)(w & 0xFF); }
    static uint8_t  dec_rA(uint32_t w)  { return (uint8_t)((w >> 8) & 0xFF); }
    static uint8_t  dec_rB(uint32_t w)  { return (uint8_t)((w >> 16) & 0xFF); }
    static uint8_t  dec_rC(uint32_t w)  { return (uint8_t)((w >> 24) & 0xFF); }
    static uint16_t dec_imm16(uint32_t w) { return (uint16_t)((w >> 16) & 0xFFFF); }
    static int16_t  dec_simm16(uint32_t w) { return (int16_t)dec_imm16(w); }
    static uint32_t dec_imm24(uint32_t w) { return (w >> 8) & 0x00FFFFFF; }
    static int32_t  dec_simm24(uint32_t w) {
        uint32_t raw = dec_imm24(w);
        if (raw & 0x800000) raw |= 0xFF000000; // sign extend
        return (int32_t)raw;
    }
    // For Form B with split fields: rB in byte 2, imm8 in byte 3
    static uint8_t dec_imm8(uint32_t w) { return (uint8_t)((w >> 24) & 0xFF); }

    // Read a 32-bit instruction word from the code section
    uint32_t fetch(const FuncEntry& fn, uint32_t pc) {
        uint32_t byte_off = fn.instr_offset + pc * 4;
        uint32_t w;
        memcpy(&w, mod.code + byte_off, 4);
        return w;
    }

    std::vector<Value>* alloc_array(size_t n) {
        if (arena) return arena->alloc_array(n);
        if (gc) {
            auto* v = gc->alloc_array(n);
            if (gc->should_collect())
                gc->collect(globals, call_stack);
            return v;
        }
        if (rc) return rc->alloc_array(n);
        auto* v = new std::vector<Value>(n);
        array_pool.push_back(v);
        return v;
    }

    int call_function(uint32_t func_idx, const Value* args, uint8_t arg_count, uint8_t ret_reg) {
        const FuncEntry& fn = mod.functions[func_idx];

        // Copy args before emplace_back — args may point into a frame's regs,
        // and emplace_back could invalidate that pointer if reallocation occurs.
        uint8_t n = std::min(arg_count, fn.param_count);
        Value arg_copy[16]; // max 16 function args
        for (uint8_t i = 0; i < n && i < 16; i++) {
            arg_copy[i] = args[i];
        }

        call_stack.emplace_back();
        Frame& frame = call_stack.back();
        frame.func    = &fn;
        frame.pc      = 0;
        frame.ret_reg = ret_reg;

        // Registers are already default-constructed by emplace_back
        // (Value() sets type=Empty, i=0, s="", arr=nullptr)

        // Bind arguments from our safe copy
        for (uint8_t i = 0; i < n; i++) {
            frame.regs[i] = arg_copy[i];
        }

        int exit_code = dispatch();
        // Refcount: release all array values in this frame's registers
        if (rc) {
            Frame& dying = call_stack.back();
            for (int i = 0; i < 256; i++) {
                rc->release(dying.regs[i]);
            }
        }
        call_stack.pop_back();
        return exit_code;
    }

    Value call_builtin(uint8_t id, Value* args, uint8_t argc) {
        switch (id) {
            case 0: // str_len
                if (argc >= 1 && args[0].type == VType::String)
                    return Value::make_int((int64_t)args[0].s.size());
                return Value::make_int(0);
            case 1: // str_equals
                if (argc >= 2)
                    return Value::make_bool(args[0].as_string() == args[1].as_string());
                return Value::make_bool(false);
            case 2: // str_contains
                if (argc >= 2 && args[0].type == VType::String && args[1].type == VType::String)
                    return Value::make_bool(args[0].s.find(args[1].s) != std::string::npos);
                return Value::make_bool(false);
            case 3: { // str_starts_with
                if (argc >= 2 && args[0].type == VType::String && args[1].type == VType::String) {
                    const auto& s = args[0].s; const auto& p = args[1].s;
                    return Value::make_bool(s.size() >= p.size() && s.compare(0, p.size(), p) == 0);
                }
                return Value::make_bool(false);
            }
            case 4: { // str_ends_with
                if (argc >= 2 && args[0].type == VType::String && args[1].type == VType::String) {
                    const auto& s = args[0].s; const auto& x = args[1].s;
                    return Value::make_bool(s.size() >= x.size() &&
                        s.compare(s.size() - x.size(), x.size(), x) == 0);
                }
                return Value::make_bool(false);
            }
            case 5: { // str_upper
                if (argc >= 1 && args[0].type == VType::String) {
                    std::string r = args[0].s;
                    for (auto& c : r) c = toupper((unsigned char)c);
                    return Value::make_string(r);
                }
                return Value::make_string("");
            }
            case 6: { // str_lower
                if (argc >= 1 && args[0].type == VType::String) {
                    std::string r = args[0].s;
                    for (auto& c : r) c = tolower((unsigned char)c);
                    return Value::make_string(r);
                }
                return Value::make_string("");
            }
            case 7: { // str_trim
                if (argc >= 1 && args[0].type == VType::String) {
                    const std::string& s = args[0].s;
                    size_t start = s.find_first_not_of(" \t\n\r");
                    if (start == std::string::npos) return Value::make_string("");
                    size_t end = s.find_last_not_of(" \t\n\r");
                    return Value::make_string(s.substr(start, end - start + 1));
                }
                return Value::make_string("");
            }
            case 8: // str_concat
                if (argc >= 2)
                    return Value::make_string(args[0].as_string() + args[1].as_string());
                return Value::make_string("");
            case 9: // strlen (C compat)
                if (argc >= 1 && args[0].type == VType::String)
                    return Value::make_int((int64_t)args[0].s.size());
                return Value::make_int(0);
            case 10: { // strstr — returns found substring or empty
                if (argc >= 2 && args[0].type == VType::String && args[1].type == VType::String) {
                    auto pos = args[0].s.find(args[1].s);
                    if (pos != std::string::npos)
                        return Value::make_string(args[0].s.substr(pos));
                    return Value::make_empty();
                }
                return Value::make_empty();
            }
            // Array builtins (arg[0]=array, arg[1]=size, ...)
            case 20: { // array_get(arr, n, idx)
                if (argc >= 3 && args[0].type == VType::Array && args[0].arr) {
                    int64_t idx = args[2].as_int();
                    auto& arr = *args[0].arr;
                    if (idx >= 0 && (size_t)idx < arr.size()) return arr[idx];
                }
                return Value::make_int(0);
            }
            case 21: { // array_set(arr, n, idx, val)
                if (argc >= 4 && args[0].type == VType::Array && args[0].arr) {
                    int64_t idx = args[2].as_int();
                    auto& arr = *args[0].arr;
                    if (idx >= 0 && (size_t)idx < arr.size()) arr[idx] = args[3];
                }
                return Value::make_int(0);
            }
            case 22: { // array_contains(arr, n, val)
                if (argc >= 3 && args[0].type == VType::Array && args[0].arr) {
                    int64_t target = args[2].as_int();
                    for (const auto& v : *args[0].arr)
                        if (v.as_int() == target) return Value::make_bool(true);
                }
                return Value::make_bool(false);
            }
            case 23: { // array_index_of(arr, n, val)
                if (argc >= 3 && args[0].type == VType::Array && args[0].arr) {
                    int64_t target = args[2].as_int();
                    auto& arr = *args[0].arr;
                    for (size_t i = 0; i < arr.size(); i++)
                        if (arr[i].as_int() == target) return Value::make_int((int64_t)i);
                }
                return Value::make_int(-1);
            }
            case 24: { // array_sum(arr, n)
                if (argc >= 1 && args[0].type == VType::Array && args[0].arr) {
                    int64_t sum = 0;
                    for (const auto& v : *args[0].arr) sum += v.as_int();
                    return Value::make_int(sum);
                }
                return Value::make_int(0);
            }
            case 25: { // array_min(arr, n)
                if (argc >= 1 && args[0].type == VType::Array && args[0].arr && !args[0].arr->empty()) {
                    int64_t mn = (*args[0].arr)[0].as_int();
                    for (const auto& v : *args[0].arr) { int64_t x = v.as_int(); if (x < mn) mn = x; }
                    return Value::make_int(mn);
                }
                return Value::make_int(0);
            }
            case 26: { // array_max(arr, n)
                if (argc >= 1 && args[0].type == VType::Array && args[0].arr && !args[0].arr->empty()) {
                    int64_t mx = (*args[0].arr)[0].as_int();
                    for (const auto& v : *args[0].arr) { int64_t x = v.as_int(); if (x > mx) mx = x; }
                    return Value::make_int(mx);
                }
                return Value::make_int(0);
            }
            case 27: { // array_reverse(arr, n)
                if (argc >= 1 && args[0].type == VType::Array && args[0].arr)
                    std::reverse(args[0].arr->begin(), args[0].arr->end());
                return Value::make_int(0);
            }
            case 28: { // array_sort(arr, n)
                if (argc >= 1 && args[0].type == VType::Array && args[0].arr) {
                    std::sort(args[0].arr->begin(), args[0].arr->end(),
                        [](const Value& a, const Value& b) { return a.as_int() < b.as_int(); });
                }
                return Value::make_int(0);
            }
            case 29: { // array_count(arr, n, val)
                if (argc >= 3 && args[0].type == VType::Array && args[0].arr) {
                    int64_t target = args[2].as_int();
                    int64_t count = 0;
                    for (const auto& v : *args[0].arr)
                        if (v.as_int() == target) count++;
                    return Value::make_int(count);
                }
                return Value::make_int(0);
            }
            case 40: // absval
                if (argc >= 1) {
                    if (args[0].type == VType::Float)
                        return Value::make_float(fabs(args[0].f));
                    return Value::make_int(llabs(args[0].as_int()));
                }
                return Value::make_int(0);
            case 41: // sqrt
            case 44: // math_sqrt
                if (argc >= 1) return Value::make_float(sqrt(args[0].as_float()));
                return Value::make_float(0.0);
            case 42: // pow
            case 45: // math_pow
                if (argc >= 2) return Value::make_float(::pow(args[0].as_float(), args[1].as_float()));
                return Value::make_float(0.0);
            case 43: // fmod
                if (argc >= 2) return Value::make_float(fmod(args[0].as_float(), args[1].as_float()));
                return Value::make_float(0.0);
            case 46: // math_abs
                if (argc >= 1) return Value::make_float(fabs(args[0].as_float()));
                return Value::make_float(0.0);
            case 47: // math_floor
                if (argc >= 1) return Value::make_float(floor(args[0].as_float()));
                return Value::make_float(0.0);
            case 48: // math_ceil
                if (argc >= 1) return Value::make_float(ceil(args[0].as_float()));
                return Value::make_float(0.0);
            case 49: // math_min
                if (argc >= 2) return Value::make_float(fmin(args[0].as_float(), args[1].as_float()));
                return Value::make_float(0.0);
            case 50: // math_max
                if (argc >= 2) return Value::make_float(fmax(args[0].as_float(), args[1].as_float()));
                return Value::make_float(0.0);
            case 51: // math_pi
                return Value::make_float(3.14159265358979323846);
            case 52: // math_e
                return Value::make_float(2.71828182845904523536);
            case 53: { // str_char_at(s, i)
                if (argc >= 2 && args[0].type == VType::String) {
                    int64_t idx = args[1].as_int();
                    const auto& s = args[0].s;
                    if (idx >= 0 && (size_t)idx < s.size())
                        return Value::make_string(std::string(1, s[idx]));
                    return Value::make_string("");
                }
                return Value::make_string("");
            }
            case 54: { // str_substr(s, start, len)
                if (argc >= 3 && args[0].type == VType::String) {
                    int64_t start = args[1].as_int();
                    int64_t len = args[2].as_int();
                    const auto& s = args[0].s;
                    if (start < 0) start = 0;
                    if (start >= (int64_t)s.size()) return Value::make_string("");
                    if (len < 0) len = 0;
                    return Value::make_string(s.substr(start, len));
                }
                return Value::make_string("");
            }
            case 55: { // str_index_of(s, sub)
                if (argc >= 2 && args[0].type == VType::String && args[1].type == VType::String) {
                    auto pos = args[0].s.find(args[1].s);
                    if (pos != std::string::npos) return Value::make_int((int64_t)pos);
                    return Value::make_int(-1);
                }
                return Value::make_int(-1);
            }
            default:
                fprintf(stderr, "error: unknown builtin %d\n", id);
                return Value::make_int(0);
        }
    }

    int dispatch() {
        Frame& F = call_stack.back();
        const FuncEntry& fn = *F.func;
        Value* R = F.regs;

        while (F.pc < fn.instr_count) {
            uint32_t w = fetch(fn, F.pc);
            uint8_t op = dec_op(w);
            F.pc++;

            switch (op) {

            // ── Hot path ────────────────────────────────────────────────────

            case OP_MOV: {
                uint8_t a = dec_rA(w), b = dec_rB(w);
                if (rc) { rc->release(R[a]); }
                R[a] = R[b];
                if (rc) { rc->retain(R[a]); }
            } break;

            case OP_LOAD_CONST: {
                uint8_t a = dec_rA(w);
                uint16_t idx = dec_imm16(w);
                if (idx < mod.const_pool.size())
                    R[a] = mod.const_pool[idx];
            } break;

            case OP_ADD: {
                uint8_t a = dec_rA(w), b = dec_rB(w), c = dec_rC(w);
                R[a] = val_add(R[b], R[c]);
            } break;

            case OP_SUB: {
                uint8_t a = dec_rA(w), b = dec_rB(w), c = dec_rC(w);
                R[a] = val_sub(R[b], R[c]);
            } break;

            case OP_MUL: {
                uint8_t a = dec_rA(w), b = dec_rB(w), c = dec_rC(w);
                R[a] = val_mul(R[b], R[c]);
            } break;

            case OP_DIV: {
                uint8_t a = dec_rA(w), b = dec_rB(w), c = dec_rC(w);
                R[a] = val_div(R[b], R[c]);
            } break;

            case OP_CMP_LT: {
                uint8_t a = dec_rA(w), b = dec_rB(w), c = dec_rC(w);
                R[a] = val_cmp_lt(R[b], R[c]);
            } break;

            case OP_JUMP_IF_FALSE: {
                uint8_t a = dec_rA(w);
                int16_t off = dec_simm16(w);
                if (!R[a].truthy()) F.pc += off;
            } break;

            case OP_CALL: {
                uint8_t a = dec_rA(w), b = dec_rB(w), c = dec_rC(w);
                // b = function index, c = arg count, args start at rA
                if (b < mod.functions.size()) {
                    call_function(b, &R[a], c, a);
                }
                // After call, result is written to R[a] via return handling
            } break;

            case OP_CALL_BUILTIN: {
                uint8_t a = dec_rA(w), b = dec_rB(w), c = dec_rC(w);
                // b = builtin ID, c = arg count, args start at R[a]
                R[a] = call_builtin(b, &R[a], c);
            } break;

            case OP_RETURN: {
                uint8_t a = dec_rA(w);
                Value ret_val = R[a];
                if (call_stack.size() <= 1) {
                    // Top-level main return — exit code
                    return (int)ret_val.as_int();
                }
                // Store return value into caller's register
                Frame& caller = call_stack[call_stack.size() - 2];
                caller.regs[F.ret_reg] = ret_val;
                return 0;
            }

            case OP_PRINT: {
                uint8_t a = dec_rA(w);
                print_value(R[a]);
            } break;

            // ── Arithmetic/logic ────────────────────────────────────────────

            case OP_MOD: {
                uint8_t a = dec_rA(w), b = dec_rB(w), c = dec_rC(w);
                R[a] = val_mod(R[b], R[c]);
            } break;

            case OP_NEG: {
                uint8_t a = dec_rA(w), b = dec_rB(w);
                if (R[b].type == VType::Float) R[a] = Value::make_float(-R[b].f);
                else R[a] = Value::make_int(-R[b].as_int());
            } break;

            case OP_NOT: {
                uint8_t a = dec_rA(w), b = dec_rB(w);
                R[a] = Value::make_bool(!R[b].truthy());
            } break;

            case OP_AND: {
                uint8_t a = dec_rA(w), b = dec_rB(w), c = dec_rC(w);
                R[a] = Value::make_bool(R[b].truthy() && R[c].truthy());
            } break;

            case OP_OR: {
                uint8_t a = dec_rA(w), b = dec_rB(w), c = dec_rC(w);
                R[a] = Value::make_bool(R[b].truthy() || R[c].truthy());
            } break;

            case OP_ADD_IMM: {
                uint8_t a = dec_rA(w), b = dec_rB(w), imm = dec_imm8(w);
                R[a] = Value::make_int(R[b].as_int() + (int64_t)imm);
            } break;

            case OP_SUB_IMM: {
                uint8_t a = dec_rA(w), b = dec_rB(w), imm = dec_imm8(w);
                R[a] = Value::make_int(R[b].as_int() - (int64_t)imm);
            } break;

            // ── Comparison ──────────────────────────────────────────────────

            case OP_CMP_EQ: {
                uint8_t a = dec_rA(w), b = dec_rB(w), c = dec_rC(w);
                R[a] = val_cmp_eq(R[b], R[c]);
            } break;

            case OP_CMP_NE: {
                uint8_t a = dec_rA(w), b = dec_rB(w), c = dec_rC(w);
                R[a] = val_cmp_ne(R[b], R[c]);
            } break;

            case OP_CMP_GT: {
                uint8_t a = dec_rA(w), b = dec_rB(w), c = dec_rC(w);
                R[a] = val_cmp_gt(R[b], R[c]);
            } break;

            case OP_CMP_LE: {
                uint8_t a = dec_rA(w), b = dec_rB(w), c = dec_rC(w);
                R[a] = val_cmp_le(R[b], R[c]);
            } break;

            case OP_CMP_GE: {
                uint8_t a = dec_rA(w), b = dec_rB(w), c = dec_rC(w);
                R[a] = val_cmp_ge(R[b], R[c]);
            } break;

            // ── Control flow ────────────────────────────────────────────────

            case OP_JUMP: {
                int32_t off = dec_simm24(w);
                F.pc += off;
            } break;

            case OP_JUMP_IF_TRUE: {
                uint8_t a = dec_rA(w);
                int16_t off = dec_simm16(w);
                if (R[a].truthy()) F.pc += off;
            } break;

            case OP_RETURN_NONE: {
                Value ret_val = Value::make_empty();
                if (call_stack.size() <= 1) return 0;
                Frame& caller = call_stack[call_stack.size() - 2];
                caller.regs[F.ret_reg] = ret_val;
                return 0;
            }

            // ── Globals ─────────────────────────────────────────────────────

            case OP_LOAD_GLOBAL: {
                uint8_t a = dec_rA(w);
                uint16_t idx = dec_imm16(w);
                if (idx < globals.size()) R[a] = globals[idx];
            } break;

            case OP_STORE_GLOBAL: {
                uint8_t a = dec_rA(w);
                uint16_t idx = dec_imm16(w);
                if (idx >= globals.size()) globals.resize(idx + 1);
                globals[idx] = R[a];
            } break;

            // ── Struct/array access ─────────────────────────────────────────

            case OP_LOAD_STRUCT_FIELD:
            case OP_LOAD_ARRAY_SEQ:
            case OP_LOAD_ARRAY_STRIDE: {
                // Treat all as generic index get for now
                uint8_t a = dec_rA(w), b = dec_rB(w), c = dec_rC(w);
                if (R[b].type == VType::Array && R[b].arr) {
                    int64_t idx = R[c].as_int();
                    auto& arr = *R[b].arr;
                    if (idx >= 0 && (size_t)idx < arr.size())
                        R[a] = arr[idx];
                }
            } break;

            case OP_STORE_STRUCT_FIELD: {
                uint8_t a = dec_rA(w), b = dec_rB(w), imm = dec_imm8(w);
                if (R[a].type == VType::Array && R[a].arr) {
                    auto& arr = *R[a].arr;
                    if ((size_t)imm < arr.size())
                        arr[imm] = R[b];
                }
            } break;

            // ── Kill bit variants — same semantics as base ops ──────────────

            case OP_ADD_K:
            case OP_ADD_KK: {
                uint8_t a = dec_rA(w), b = dec_rB(w), c = dec_rC(w);
                R[a] = val_add(R[b], R[c]);
            } break;

            case OP_SUB_K:
            case OP_SUB_KK: {
                uint8_t a = dec_rA(w), b = dec_rB(w), c = dec_rC(w);
                R[a] = val_sub(R[b], R[c]);
            } break;

            case OP_MUL_K:
            case OP_MUL_KK: {
                uint8_t a = dec_rA(w), b = dec_rB(w), c = dec_rC(w);
                R[a] = val_mul(R[b], R[c]);
            } break;

            case OP_DIV_K:
            case OP_DIV_KK: {
                uint8_t a = dec_rA(w), b = dec_rB(w), c = dec_rC(w);
                R[a] = val_div(R[b], R[c]);
            } break;

            case OP_CMP_LT_K:
            case OP_CMP_LT_KK: {
                uint8_t a = dec_rA(w), b = dec_rB(w), c = dec_rC(w);
                R[a] = val_cmp_lt(R[b], R[c]);
            } break;

            // ── Arrays ──────────────────────────────────────────────────────

            case OP_NEW_ARRAY: {
                uint8_t a = dec_rA(w), b = dec_rB(w), c = dec_rC(w);
                int64_t count = R[c].as_int();
                if (count < 0) count = 0;
                auto* arr = alloc_array((size_t)count);
                R[a] = Value::make_array(arr);
            } break;

            case OP_INDEX_GET: {
                uint8_t a = dec_rA(w), b = dec_rB(w), c = dec_rC(w);
                if (R[b].type == VType::Array && R[b].arr) {
                    int64_t idx = R[c].as_int();
                    auto& arr = *R[b].arr;
                    if (idx < 0 || (size_t)idx >= arr.size()) {
                        fprintf(stderr, "error: array index %lld out of bounds (size %lld)\n",
                                (long long)idx, (long long)arr.size());
                        exit(1);
                    }
                    R[a] = arr[idx];
                }
            } break;

            case OP_INDEX_SET: {
                uint8_t a = dec_rA(w), b = dec_rB(w), c = dec_rC(w);
                if (R[a].type == VType::Array && R[a].arr) {
                    int64_t idx = R[b].as_int();
                    auto& arr = *R[a].arr;
                    if (idx < 0 || (size_t)idx >= arr.size()) {
                        fprintf(stderr, "error: array index %lld out of bounds (size %lld)\n",
                                (long long)idx, (long long)arr.size());
                        exit(1);
                    }
                    arr[idx] = R[c];
                }
            } break;

            case OP_LENGTH: {
                uint8_t a = dec_rA(w), b = dec_rB(w);
                if (R[b].type == VType::Array && R[b].arr)
                    R[a] = Value::make_int((int64_t)R[b].arr->size());
                else if (R[b].type == VType::String)
                    R[a] = Value::make_int((int64_t)R[b].s.size());
                else
                    R[a] = Value::make_int(0);
            } break;

            // ── Type casts ──────────────────────────────────────────────────

            case OP_CAST_INT: {
                uint8_t a = dec_rA(w), b = dec_rB(w);
                R[a] = Value::make_int(R[b].as_int());
            } break;

            case OP_CAST_FLOAT: {
                uint8_t a = dec_rA(w), b = dec_rB(w);
                R[a] = Value::make_float(R[b].as_float());
            } break;

            case OP_CAST_STRING: {
                uint8_t a = dec_rA(w), b = dec_rB(w);
                R[a] = Value::make_string(R[b].as_string());
            } break;

            case OP_CAST_BOOL: {
                uint8_t a = dec_rA(w), b = dec_rB(w);
                R[a] = Value::make_bool(R[b].truthy());
            } break;

            // ── Tier 1 — semantic operations ────────────────────────────────

            case OP_CALL_IC: {
                // Same as CALL for interpreter — IC is JIT-only
                uint8_t a = dec_rA(w), b = dec_rB(w);
                // ic_slot in imm8 ignored
                if (b < mod.functions.size()) {
                    call_function(b, &R[a], mod.functions[b].param_count, a);
                }
            } break;

            case OP_FIELD_GET: {
                // rA = rB.field[ic_slot] — treat as array index
                uint8_t a = dec_rA(w), b = dec_rB(w), idx = dec_imm8(w);
                if (R[b].type == VType::Array && R[b].arr && (size_t)idx < R[b].arr->size())
                    R[a] = (*R[b].arr)[idx];
            } break;

            case OP_FIELD_SET: {
                uint8_t a = dec_rA(w), b = dec_rB(w), idx = dec_imm8(w);
                if (R[a].type == VType::Array && R[a].arr && (size_t)idx < R[a].arr->size())
                    (*R[a].arr)[idx] = R[b];
            } break;

            case OP_ITER_INIT: {
                // rA = iterator state (just store the source array + index 0)
                uint8_t a = dec_rA(w), b = dec_rB(w);
                // Store the source in rA, use a separate counter — simplified:
                // We pack the array ref into rA and rely on ITER_NEXT to use it
                R[a] = R[b];
            } break;

            case OP_ITER_NEXT: {
                // Not fully implementable without an iteration index;
                // would need an extended value type. Placeholder.
                uint8_t a = dec_rA(w);
                (void)a;
            } break;

            // ── Tier 2 — superinstructions ──────────────────────────────────

            case OP_ADD_REGS: {
                uint8_t a = dec_rA(w), b = dec_rB(w), c = dec_rC(w);
                // int-only fast path
                R[a] = Value::make_int(R[b].as_int() + R[c].as_int());
            } break;

            case OP_LOOP_TEST: {
                uint8_t a = dec_rA(w), b = dec_rB(w), imm = dec_imm8(w);
                if (!(R[a].as_int() < R[b].as_int()))
                    F.pc += (int8_t)imm;
            } break;

            case OP_RETURN_REG: {
                uint8_t a = dec_rA(w);
                Value ret_val = R[a];
                if (call_stack.size() <= 1) return (int)ret_val.as_int();
                Frame& caller = call_stack[call_stack.size() - 2];
                caller.regs[F.ret_reg] = ret_val;
                return 0;
            }

            case OP_CALL_STORE: {
                uint8_t a = dec_rA(w), b = dec_rB(w), c = dec_rC(w);
                if (b < mod.functions.size()) {
                    call_function(b, &R[a], c, a);
                }
            } break;

            case OP_STRUCT_COPY: {
                uint8_t a = dec_rA(w), b = dec_rB(w), imm = dec_imm8(w);
                for (uint8_t n = 0; n < imm; n++)
                    R[a + n] = R[b + n];
            } break;

            case OP_BULK_ZERO: {
                uint8_t a = dec_rA(w);
                uint16_t count = dec_imm16(w);
                for (uint16_t n = 0; n < count && (a + n) < 256; n++)
                    R[a + n] = Value::make_int(0);
            } break;

            case OP_SLICE: {
                // Simplified — not fully supported
                uint8_t a = dec_rA(w);
                R[a] = Value::make_empty();
            } break;

            case OP_INC_REG: {
                uint8_t a = dec_rA(w);
                R[a] = Value::make_int(R[a].as_int() + 1);
            } break;

            case OP_DEC_REG: {
                uint8_t a = dec_rA(w);
                R[a] = Value::make_int(R[a].as_int() - 1);
            } break;

            case OP_CMP_JUMP_EQ: {
                uint8_t a = dec_rA(w), b = dec_rB(w), imm = dec_imm8(w);
                if (R[a].as_int() == R[b].as_int())
                    F.pc += (int8_t)imm;
            } break;

            // ── Quickened type variants ──────────────────────────────────────

            case OP_ADD_INT: {
                uint8_t a = dec_rA(w), b = dec_rB(w), c = dec_rC(w);
                R[a] = Value::make_int(R[b].i + R[c].i);
            } break;

            case OP_ADD_FLOAT: {
                uint8_t a = dec_rA(w), b = dec_rB(w), c = dec_rC(w);
                R[a] = Value::make_float(R[b].f + R[c].f);
            } break;

            case OP_SUB_INT: {
                uint8_t a = dec_rA(w), b = dec_rB(w), c = dec_rC(w);
                R[a] = Value::make_int(R[b].i - R[c].i);
            } break;

            case OP_SUB_FLOAT: {
                uint8_t a = dec_rA(w), b = dec_rB(w), c = dec_rC(w);
                R[a] = Value::make_float(R[b].f - R[c].f);
            } break;

            case OP_MUL_INT: {
                uint8_t a = dec_rA(w), b = dec_rB(w), c = dec_rC(w);
                R[a] = Value::make_int(R[b].i * R[c].i);
            } break;

            case OP_MUL_FLOAT: {
                uint8_t a = dec_rA(w), b = dec_rB(w), c = dec_rC(w);
                R[a] = Value::make_float(R[b].f * R[c].f);
            } break;

            case OP_DIV_INT: {
                uint8_t a = dec_rA(w), b = dec_rB(w), c = dec_rC(w);
                if (R[c].i == 0) { fprintf(stderr, "error: division by zero\n"); exit(1); }
                R[a] = Value::make_int(R[b].i / R[c].i);
            } break;

            case OP_DIV_FLOAT: {
                uint8_t a = dec_rA(w), b = dec_rB(w), c = dec_rC(w);
                if (R[c].f == 0.0) { fprintf(stderr, "error: division by zero\n"); exit(1); }
                R[a] = Value::make_float(R[b].f / R[c].f);
            } break;

            case OP_CMP_LT_INT: {
                uint8_t a = dec_rA(w), b = dec_rB(w), c = dec_rC(w);
                R[a] = Value::make_bool(R[b].i < R[c].i);
            } break;

            case OP_CMP_LT_FLOAT: {
                uint8_t a = dec_rA(w), b = dec_rB(w), c = dec_rC(w);
                R[a] = Value::make_bool(R[b].f < R[c].f);
            } break;

            case OP_CMP_EQ_INT: {
                uint8_t a = dec_rA(w), b = dec_rB(w), c = dec_rC(w);
                R[a] = Value::make_bool(R[b].i == R[c].i);
            } break;

            case OP_CMP_EQ_FLOAT: {
                uint8_t a = dec_rA(w), b = dec_rB(w), c = dec_rC(w);
                R[a] = Value::make_bool(R[b].f == R[c].f);
            } break;

            case OP_LOAD_CONST_INT: {
                uint8_t a = dec_rA(w);
                uint16_t idx = dec_imm16(w);
                if (idx < mod.const_pool.size())
                    R[a] = Value::make_int(mod.const_pool[idx].i);
            } break;

            case OP_LOAD_CONST_FLOAT: {
                uint8_t a = dec_rA(w);
                uint16_t idx = dec_imm16(w);
                if (idx < mod.const_pool.size())
                    R[a] = Value::make_float(mod.const_pool[idx].f);
            } break;

            // ── Result types ──────────────────────────────────────────────

            case OP_MAKE_OK: {
                // rA = result array [0, rB]
                uint8_t a = dec_rA(w), b = dec_rB(w);
                auto* arr = alloc_array(2);
                (*arr)[0] = Value::make_int(0);  // tag 0 = ok
                (*arr)[1] = R[b];
                R[a] = Value::make_array(arr);
            } break;

            case OP_MAKE_ERROR: {
                // rA = result array [1, rB]
                uint8_t a = dec_rA(w), b = dec_rB(w);
                auto* arr = alloc_array(2);
                (*arr)[0] = Value::make_int(1);  // tag 1 = error
                (*arr)[1] = R[b];
                R[a] = Value::make_array(arr);
            } break;

            case OP_CHECK_RESULT: {
                // rA = tag, rB = value, rC = result array
                uint8_t a = dec_rA(w), b = dec_rB(w), c = dec_rC(w);
                if (R[c].type == VType::Array && R[c].arr && R[c].arr->size() >= 2) {
                    R[a] = (*R[c].arr)[0];  // tag
                    R[b] = (*R[c].arr)[1];  // value
                } else {
                    // Not a result — treat as ok with the raw value
                    R[a] = Value::make_int(0);
                    R[b] = R[c];
                }
            } break;

            case OP_JUMP_IF_ERROR: {
                // rA = tag register, imm16 = offset
                uint8_t a = dec_rA(w);
                int16_t off = dec_simm16(w);
                if (R[a].as_int() != 0) F.pc += off;
            } break;

            case OP_ERROR_HALT: {
                // rA = error message register — print to stderr and exit(1)
                uint8_t a = dec_rA(w);
                fprintf(stderr, "error: %s\n", R[a].as_string().c_str());
                return 1;
            }

            case OP_NOP:
                break;

            default:
                fprintf(stderr, "error: unknown opcode 0x%02X at pc=%u in %s\n",
                        op, F.pc - 1, fn.name.c_str());
                return 1;

            } // switch
        } // while

        // Fell off end of function — implicit return 0 for main, empty for others
        if (call_stack.size() <= 1) return 0;
        Frame& caller = call_stack[call_stack.size() - 2];
        caller.regs[F.ret_reg] = Value::make_empty();
        return 0;
    }
};

// ── Main ────────────────────────────────────────────────────────────────────

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "usage: ulvm [--arena|--gc|--refcount] <program.ulb>\n");
        return 1;
    }

    // Parse flags
    MemMode mode = MemMode::Default;
    const char* filename = nullptr;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--arena") == 0)           mode = MemMode::Arena;
        else if (strcmp(argv[i], "--gc") == 0)          mode = MemMode::GC;
        else if (strcmp(argv[i], "--refcount") == 0)    mode = MemMode::Refcount;
        else                                            filename = argv[i];
    }
    if (!filename) {
        fprintf(stderr, "usage: ulvm [--arena|--gc|--refcount] <program.ulb>\n");
        return 1;
    }

    // Read the .ulb file
    FILE* fp = fopen(filename, "rb");
    if (!fp) {
        fprintf(stderr, "error: cannot open '%s'\n", filename);
        return 1;
    }
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    std::vector<uint8_t> buf(file_size);
    size_t read = fread(buf.data(), 1, file_size, fp);
    fclose(fp);

    if ((long)read != file_size) {
        fprintf(stderr, "error: short read on '%s'\n", filename);
        return 1;
    }

    // Load and execute
    Module mod = load_module(buf.data(), buf.size());
    VM vm(mod, mode);
    return vm.run();
}
