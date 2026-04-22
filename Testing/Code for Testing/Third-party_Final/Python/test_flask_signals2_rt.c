// @dr python_compat = true

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

typedef struct Signal Signal;
typedef struct {
    void (*__init__)(Signal*, int name);
    void (*connect)(Signal*, int receiver);
    void (*send)(Signal*);
    void (*__repr__)(Signal*);
} Signal_VTable;

struct Signal {
    Signal_VTable* _vtable;
    const char* __type;
    int name;
    int _receivers;
};

typedef struct Namespace Namespace;
typedef struct {
    void (*__init__)(Namespace*);
    void (*signal)(Namespace*, int name);
} Namespace_VTable;

struct Namespace {
    Namespace_VTable* _vtable;
    const char* __type;
    int _signals;
};

__auto_type received = NULL;
void main__on_template_rendered(int data);
int main(void);
void Signal___init__(Signal* self, int name);
void Signal_connect(Signal* self, int receiver);
void Signal_send(Signal* self);
void Signal___repr__(Signal* self);
void Namespace___init__(Namespace* self);
void Namespace_signal(Namespace* self, int name);
static Signal_VTable _Signal_vtable;
static Namespace_VTable _Namespace_vtable;

void main__on_template_rendered(int data)
{
    received._vtable->append(&received, __ul_strcat("rendered:", data));
    return __ul_strcat("rendered:", data);
}

int main(void)
{
    ns = Namespace();
    template_rendered = ns._vtable->signal(&ns, "template-rendered");
    request_started = ns._vtable->signal(&ns, "request-started");
    got_request_exception = ns._vtable->signal(&ns, "got-request-exception");
    static char _cast_buf_0[64];
    snprintf(_cast_buf_0, sizeof(_cast_buf_0), "%s", "template_rendered:");
    static char _cast_buf_1[64];
    snprintf(_cast_buf_1, sizeof(_cast_buf_1), "%d", template_rendered);
    printf("%s\n", __ul_strcat((_cast_buf_0 + " "), _cast_buf_1));
    static char _cast_buf_2[64];
    snprintf(_cast_buf_2, sizeof(_cast_buf_2), "%s", "request_started:");
    static char _cast_buf_3[64];
    snprintf(_cast_buf_3, sizeof(_cast_buf_3), "%d", request_started);
    printf("%s\n", __ul_strcat((_cast_buf_2 + " "), _cast_buf_3));
    received = /* unknown expr ArrayLiteral */;
    template_rendered._vtable->connect(&template_rendered, main__on_template_rendered);
    template_rendered._vtable->send(&template_rendered, "index.html");
    static char _cast_buf_4[64];
    snprintf(_cast_buf_4, sizeof(_cast_buf_4), "%s", "received:");
    static char _cast_buf_5[64];
    snprintf(_cast_buf_5, sizeof(_cast_buf_5), "%d", received);
    printf("%s\n", __ul_strcat((_cast_buf_4 + " "), _cast_buf_5));
    static char _cast_buf_6[64];
    snprintf(_cast_buf_6, sizeof(_cast_buf_6), "%s", "same signal:");
    static char _cast_buf_7[64];
    snprintf(_cast_buf_7, sizeof(_cast_buf_7), "%d", (ns._vtable->signal(&ns, "template-rendered") == template_rendered));
    printf("%s\n", __ul_strcat((_cast_buf_6 + " "), _cast_buf_7));
    return 0;
    return 0;
}

void Signal___init__(Signal* self, int name)
{
    self->name = self->name;
    self->_receivers = /* unknown expr ArrayLiteral */;
}

void Signal_connect(Signal* self, int receiver)
{
    self->_receivers._vtable->append(&self->_receivers, receiver);
}

void Signal_send(Signal* self)
{
    results = /* unknown expr ArrayLiteral */;
    for (int _i = 0; _i < (int)(sizeof(self->_receivers)/sizeof(self->_receivers[0])); _i++) {
        int r = self->_receivers[_i];
        results._vtable->append(&results, r(_va_args));
    }
    return results;
}

void Signal___repr__(Signal* self)
{
    static char _cast_buf_8[64];
    snprintf(_cast_buf_8, sizeof(_cast_buf_8), "%d", self->name);
    return __ul_strcat(__ul_strcat("<Signal '", _cast_buf_8), "'>");
}

void Namespace___init__(Namespace* self)
{
    map _dict0 = map_new();
    self->_signals = _dict0;
}

void Namespace_signal(Namespace* self, int name)
{
    if ((!self->_signals._vtable->contains(&self->_signals, name))) {
        self->_signals[name] = Signal(name);
    }
    return self->_signals[name];
}

static Signal_VTable _Signal_vtable = {(void*)Signal___init__, (void*)Signal_connect, (void*)Signal_send, (void*)Signal___repr__};
static Namespace_VTable _Namespace_vtable = {(void*)Namespace___init__, (void*)Namespace_signal};

