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
static void __ul_print_float(double v) {
    char _buf[64]; snprintf(_buf, sizeof(_buf), "%.10g", v);
    int _has_dot = 0; for (int _i = 0; _buf[_i]; _i++) if (_buf[_i] == '.' || _buf[_i] == 'e') { _has_dot = 1; break; }
    if (!_has_dot) { int _l = (int)strlen(_buf); _buf[_l] = '.'; _buf[_l+1] = '0'; _buf[_l+2] = 0; }
    printf("%s\n", _buf); }
#include <stdlib.h>

typedef struct {
    uint8_t* buf;
    uint8_t* head;
    uint8_t* tail;
    int size_c;
} ringbuf_t;

typedef struct { uint8_t* data; int len; int cap; } __ul_list_uint8;
static void __ul_list_uint8_append(__ul_list_uint8* l, uint8_t val) {
    if (l->len >= l->cap) { l->cap = l->cap ? l->cap * 2 : 8; l->data = (uint8_t*)realloc(l->data, l->cap * sizeof(uint8_t)); }
    l->data[l->len++] = val;
}
static uint8_t __ul_list_uint8_pop(__ul_list_uint8* l) { return l->data[--l->len]; }
static uint8_t __ul_list_uint8_drop(__ul_list_uint8* l, int i) {
    uint8_t val = l->data[i];
    memmove(&l->data[i], &l->data[i+1], (l->len - i - 1) * sizeof(uint8_t));
    l->len--; return val;
}
static void __ul_list_uint8_insert(__ul_list_uint8* l, int i, uint8_t val) {
    if (l->len >= l->cap) { l->cap = l->cap ? l->cap * 2 : 8; l->data = (uint8_t*)realloc(l->data, l->cap * sizeof(uint8_t)); }
    memmove(&l->data[i+1], &l->data[i], (l->len - i) * sizeof(uint8_t));
    l->data[i] = val; l->len++;
}
static void __ul_list_uint8_remove(__ul_list_uint8* l, uint8_t val) {
    for (int i = 0; i < l->len; i++) { if (l->data[i] == val) {
        memmove(&l->data[i], &l->data[i+1], (l->len - i - 1) * sizeof(uint8_t)); l->len--; return; } }
}
static void __ul_list_uint8_clear(__ul_list_uint8* l) { l->len = 0; }
static void __ul_list_uint8_sort(__ul_list_uint8* l) { qsort(l->data, l->len, sizeof(uint8_t), __ul_cmp_int); }
static void __ul_list_uint8_reverse(__ul_list_uint8* l) {
    for (int i = 0, j = l->len - 1; i < j; i++, j--) { uint8_t tmp = l->data[i]; l->data[i] = l->data[j]; l->data[j] = tmp; }
}
static int __ul_list_uint8_contains(__ul_list_uint8* l, uint8_t val) {
    for (int i = 0; i < l->len; i++) if (l->data[i] == val) return 1; return 0;
}

int ringbuf_buffer_size(ringbuf_t* rb);
uint8_t ringbuf_end(ringbuf_t* rb);
uint8_t ringbuf_nextp(ringbuf_t* rb, uint8_t* p);
void ringbuf_free_sim(ringbuf_t* rb);
int main(void);

int ringbuf_buffer_size(ringbuf_t* rb)
{
    return rb->size_c;
}

uint8_t ringbuf_end(ringbuf_t* rb)
{
    return (rb->buf + ringbuf_buffer_size(rb));
}

uint8_t ringbuf_nextp(ringbuf_t* rb, uint8_t* p)
{
    return (rb->buf + ((p - rb->buf) % ringbuf_buffer_size(rb)));
}

void ringbuf_free_sim(ringbuf_t* rb)
{
    (*rb) = 0;
}

int main(void)
{
    __ul_list_uint8 storage = {NULL, 0, 0};
    ringbuf_t rb = {0};
    rb.buf = storage;
    rb.head = storage;
    rb.tail = storage;
    rb.size_c = (4 + 1);
    uint8_t* end_c = ringbuf_end((&rb));
    static char _cast_buf_0[64];
    snprintf(_cast_buf_0, sizeof(_cast_buf_0), "%d", ((void)rb.buf));
    printf("%s\n", __ul_strcat(__ul_strcat("buf=%p size=", _cast_buf_0), " end=%p"));
    int i = 0;
    i = 0;
    while ((i < rb.size_c)) {
        uint8_t* p = (rb.buf + i);
        if ((p < end_c)) {
            uint8_t* next = ringbuf_nextp((&rb), p);
            int expected_offset = ((int)((i + 1) % rb.size_c));
            int actual_offset = ((int)(next - rb.buf));
            __auto_type _tern_1 = "FAIL";
            if ((actual_offset == expected_offset)) {
                _tern_1 = "OK";
            }
            static char _cast_buf_1[64];
            snprintf(_cast_buf_1, sizeof(_cast_buf_1), "%d", ((int)i));
            static char _cast_buf_2[64];
            snprintf(_cast_buf_2, sizeof(_cast_buf_2), "%d", actual_offset);
            static char _cast_buf_3[64];
            snprintf(_cast_buf_3, sizeof(_cast_buf_3), "%d", expected_offset);
            printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat("nextp(offset=", _cast_buf_1), ") -> offset="), _cast_buf_2), " (expected="), _cast_buf_3), ") "), _tern_1));
        }
        i = (i + 1);
    }
    ringbuf_t* rbp = ((ringbuf_t)__ul_malloc(sizeof(ringbuf_t)));
    rbp->size_c = (4 + 1);
    rbp->buf = ((uint8_t)__ul_malloc(rbp->size_c));
    rbp->head = rbp->buf;
    rbp->tail = rbp->buf;
    __auto_type _tern_2 = "null";
    if (rbp) {
        _tern_2 = "non-null";
    }
    printf("%s\n", __ul_strcat("before free: rbp=", _tern_2));
    ringbuf_free_sim((&rbp));
    __auto_type _tern_3 = "non-null";
    if ((rbp == 0)) {
        _tern_3 = "null";
    }
    printf("%s\n", __ul_strcat("after free: rbp=", _tern_3));
    return 0;
}


