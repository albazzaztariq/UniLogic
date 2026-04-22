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
int ringbuf_capacity(ringbuf_t* rb);
int ringbuf_bytes_free(ringbuf_t* rb);
int ringbuf_bytes_used(ringbuf_t* rb);
int main(void);

int ringbuf_buffer_size(ringbuf_t* rb)
{
    return rb->size_c;
}

int ringbuf_capacity(ringbuf_t* rb)
{
    return (ringbuf_buffer_size(rb) - 1);
}

int ringbuf_bytes_free(ringbuf_t* rb)
{
    if ((rb->head >= rb->tail)) {
        return (ringbuf_capacity(rb) - (rb->head - rb->tail));
    } else {
        return ((rb->tail - rb->head) - 1);
    }
}

int ringbuf_bytes_used(ringbuf_t* rb)
{
    return (ringbuf_capacity(rb) - ringbuf_bytes_free(rb));
}

int main(void)
{
    __ul_list_uint8 storage = {NULL, 0, 0};
    ringbuf_t rb = {0};
    rb.buf = storage;
    rb.size_c = (8 + 1);
    rb.head = storage;
    rb.tail = storage;
    static char _cast_buf_0[64];
    snprintf(_cast_buf_0, sizeof(_cast_buf_0), "%d", ((int)ringbuf_bytes_free((&rb))));
    static char _cast_buf_1[64];
    snprintf(_cast_buf_1, sizeof(_cast_buf_1), "%d", ((int)ringbuf_bytes_used((&rb))));
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("empty: free=", _cast_buf_0), " used="), _cast_buf_1));
    rb.head = (storage + 3);
    rb.tail = storage;
    static char _cast_buf_2[64];
    snprintf(_cast_buf_2, sizeof(_cast_buf_2), "%d", ((int)ringbuf_bytes_free((&rb))));
    static char _cast_buf_3[64];
    snprintf(_cast_buf_3, sizeof(_cast_buf_3), "%d", ((int)ringbuf_bytes_used((&rb))));
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("head+3 tail+0: free=", _cast_buf_2), " used="), _cast_buf_3));
    rb.head = (storage + 5);
    rb.tail = (storage + 6);
    static char _cast_buf_4[64];
    snprintf(_cast_buf_4, sizeof(_cast_buf_4), "%d", ((int)ringbuf_bytes_free((&rb))));
    static char _cast_buf_5[64];
    snprintf(_cast_buf_5, sizeof(_cast_buf_5), "%d", ((int)ringbuf_bytes_used((&rb))));
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("full (tail=head+1): free=", _cast_buf_4), " used="), _cast_buf_5));
    rb.head = (storage + 2);
    rb.tail = (storage + 6);
    static char _cast_buf_6[64];
    snprintf(_cast_buf_6, sizeof(_cast_buf_6), "%d", ((int)ringbuf_bytes_free((&rb))));
    static char _cast_buf_7[64];
    snprintf(_cast_buf_7, sizeof(_cast_buf_7), "%d", ((int)ringbuf_bytes_used((&rb))));
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("wrapped head+2 tail+6: free=", _cast_buf_6), " used="), _cast_buf_7));
    rb.head = (storage + 7);
    rb.tail = (storage + 0);
    int used = ((int)(rb.head - rb.tail));
    static char _cast_buf_8[64];
    snprintf(_cast_buf_8, sizeof(_cast_buf_8), "%d", ((int)used));
    printf("%s\n", __ul_strcat("subtraction head-tail=", _cast_buf_8));
    return 0;
}


