// @dr python_compat = true
// @dr pyimport_0 = collections__OrderedDict
// @dr pyimport_1 = compat__Mapping
// @dr pyimport_2 = compat__MutableMapping

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#define __ul_malloc(size) malloc(size)
#define __ul_free(ptr)    free(ptr)
static char* str_char_at(const char* s, int i) {
    char* r = (char*)__ul_malloc(2); if (i >= 0 && i < (int)strlen(s)) { r[0] = s[i]; r[1] = 0; } else { r[0] = 0; } return r; }
static char* __ul_char_from_code(int code) {
    char* r = (char*)__ul_malloc(2); r[0] = (char)code; r[1] = 0; return r; }
static char* __ul_strcat(const char* a, const char* b) {
    int la = (int)strlen(a), lb = (int)strlen(b);
    char* r = (char*)__ul_malloc(la + lb + 1); memcpy(r, a, la); memcpy(r + la, b, lb); r[la+lb] = 0; return r; }
static int __ul_cmp_int(const void* a, const void* b) { return (*(int*)a - *(int*)b); }
static int __ul_cmp_float(const void* a, const void* b) { float fa=*(float*)a, fb=*(float*)b; return (fa>fb)-(fa<fb); }
static int __ul_cmp_double(const void* a, const void* b) { double da=*(double*)a, db=*(double*)b; return (da>db)-(da<db); }
static int __ul_cmp_string(const void* a, const void* b) { return strcmp(*(char**)a, *(char**)b); }

typedef struct MutableMapping MutableMapping;
typedef struct {
} MutableMapping_VTable;

struct MutableMapping {
    MutableMapping_VTable* _vtable;
    const char* __type;
};

typedef struct dict dict;
typedef struct {
} dict_VTable;

struct dict {
    dict_VTable* _vtable;
    const char* __type;
};

typedef struct CaseInsensitiveDict CaseInsensitiveDict;
typedef struct {
    void (*__init__)(CaseInsensitiveDict*, int data);
    void (*__setitem__)(CaseInsensitiveDict*, int key, int value);
    void (*__getitem__)(CaseInsensitiveDict*, int key);
    void (*__delitem__)(CaseInsensitiveDict*, int key);
    void (*__iter__)(CaseInsensitiveDict*);
    void (*__len__)(CaseInsensitiveDict*);
    void (*lower_items)(CaseInsensitiveDict*);
    void (*__eq__)(CaseInsensitiveDict*, int other);
    void (*copy)(CaseInsensitiveDict*);
    void (*__repr__)(CaseInsensitiveDict*);
} CaseInsensitiveDict_VTable;

struct CaseInsensitiveDict {
    MutableMapping _base;
    CaseInsensitiveDict_VTable* _vtable;
    const char* __type;
    int _store;
};

typedef struct LookupDict LookupDict;
typedef struct {
    void (*__init__)(LookupDict*, int name);
    void (*__repr__)(LookupDict*);
    void (*__getitem__)(LookupDict*, int key);
    void (*get)(LookupDict*, int key, int _py_default);
} LookupDict_VTable;

struct LookupDict {
    dict _base;
    LookupDict_VTable* _vtable;
    const char* __type;
    int name;
};

void CaseInsensitiveDict___init__(CaseInsensitiveDict* self, int data);
void CaseInsensitiveDict___setitem__(CaseInsensitiveDict* self, int key, int value);
void CaseInsensitiveDict___getitem__(CaseInsensitiveDict* self, int key);
void CaseInsensitiveDict___delitem__(CaseInsensitiveDict* self, int key);
void CaseInsensitiveDict___iter__(CaseInsensitiveDict* self);
void CaseInsensitiveDict___len__(CaseInsensitiveDict* self);
void CaseInsensitiveDict_lower_items(CaseInsensitiveDict* self);
void CaseInsensitiveDict___eq__(CaseInsensitiveDict* self, int other);
void CaseInsensitiveDict_copy(CaseInsensitiveDict* self);
void CaseInsensitiveDict___repr__(CaseInsensitiveDict* self);
void LookupDict___init__(LookupDict* self, int name);
void LookupDict___repr__(LookupDict* self);
void LookupDict___getitem__(LookupDict* self, int key);
void LookupDict_get(LookupDict* self, int key, int _py_default);
static MutableMapping_VTable _MutableMapping_vtable;
static dict_VTable _dict_vtable;
static CaseInsensitiveDict_VTable _CaseInsensitiveDict_vtable;
static LookupDict_VTable _LookupDict_vtable;

void CaseInsensitiveDict___init__(CaseInsensitiveDict* self, int data)
{
    self->_store = OrderedDict();
    if ((data == NULL)) {
        map _dict0 = map_new();
        data = _dict0;
    }
    self._vtable->update(&self, data);
}

void CaseInsensitiveDict___setitem__(CaseInsensitiveDict* self, int key, int value)
{
    self->_store[key._vtable->lower(&key)] = /* unknown expr ArrayLiteral */;
}

void CaseInsensitiveDict___getitem__(CaseInsensitiveDict* self, int key)
{
    return self->_store[key._vtable->lower(&key)][1];
}

void CaseInsensitiveDict___delitem__(CaseInsensitiveDict* self, int key)
{
}

void CaseInsensitiveDict___iter__(CaseInsensitiveDict* self)
{
    return 0;
}

void CaseInsensitiveDict___len__(CaseInsensitiveDict* self)
{
    return self->_store._vtable->len(&self->_store);
}

void CaseInsensitiveDict_lower_items(CaseInsensitiveDict* self)
{
    return 0;
}

void CaseInsensitiveDict___eq__(CaseInsensitiveDict* self, int other)
{
    if (isinstance(other, Mapping)) {
        other = CaseInsensitiveDict(other);
    } else {
        return NotImplemented;
    }
    return (dict(self._vtable->lower_items(&self)) == dict(other._vtable->lower_items(&other)));
}

void CaseInsensitiveDict_copy(CaseInsensitiveDict* self)
{
    return CaseInsensitiveDict(self->_store._vtable->values(&self->_store));
}

void CaseInsensitiveDict___repr__(CaseInsensitiveDict* self)
{
    static char _cast_buf_0[64];
    snprintf(_cast_buf_0, sizeof(_cast_buf_0), "%d", dict(self._vtable->items(&self)));
    return _cast_buf_0;
}

void LookupDict___init__(LookupDict* self, int name)
{
    self->name = self->name;
    super()._vtable->__init__(&super());
}

void LookupDict___repr__(LookupDict* self)
{
    static char _cast_buf_1[64];
    snprintf(_cast_buf_1, sizeof(_cast_buf_1), "%d", self->name);
    return __ul_strcat(__ul_strcat("<lookup '", _cast_buf_1), "'>");
}

void LookupDict___getitem__(LookupDict* self, int key)
{
    return self->_base.__dict__._vtable->get(&self->_base.__dict__, key, NULL);
}

void LookupDict_get(LookupDict* self, int key, int _py_default)
{
    return self->_base.__dict__._vtable->get(&self->_base.__dict__, key, _py_default);
}

static MutableMapping_VTable _MutableMapping_vtable = {};
static dict_VTable _dict_vtable = {};
static CaseInsensitiveDict_VTable _CaseInsensitiveDict_vtable = {(void*)CaseInsensitiveDict___init__, (void*)CaseInsensitiveDict___setitem__, (void*)CaseInsensitiveDict___getitem__, (void*)CaseInsensitiveDict___delitem__, (void*)CaseInsensitiveDict___iter__, (void*)CaseInsensitiveDict___len__, (void*)CaseInsensitiveDict_lower_items, (void*)CaseInsensitiveDict___eq__, (void*)CaseInsensitiveDict_copy, (void*)CaseInsensitiveDict___repr__};
static LookupDict_VTable _LookupDict_vtable = {(void*)LookupDict___init__, (void*)LookupDict___repr__, (void*)LookupDict___getitem__, (void*)LookupDict_get};

