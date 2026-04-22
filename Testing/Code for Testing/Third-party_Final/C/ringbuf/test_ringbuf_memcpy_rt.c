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

uint8_t ringbuf_end(ringbuf_t* rb);
int ringbuf_capacity(ringbuf_t* rb);
int ringbuf_bytes_free(ringbuf_t* rb);
int ringbuf_bytes_used(ringbuf_t* rb);
void rb_memcpy_into(ringbuf_t* dst, void* src, int count);
void rb_memcpy_from(void* dst, ringbuf_t* src, int count);
int main(void);

uint8_t ringbuf_end(ringbuf_t* rb)
{
    return (rb->buf + rb->size_c);
}

int ringbuf_capacity(ringbuf_t* rb)
{
    return (rb->size_c - 1);
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

void rb_memcpy_into(ringbuf_t* dst, void* src, int count)
{
    uint8_t* u8src = ((uint8_t)src);
    uint8_t* bufend = ringbuf_end(dst);
    int nread = 0;
    while ((nread != count)) {
        __auto_type _tern_1 = (count - nread);
        if ((((int)(bufend - dst->head)) < (count - nread))) {
            _tern_1 = ((int)(bufend - dst->head));
        }
        int n = _tern_1;
        memcpy(dst->head, (u8src + nread), n);
        dst->head = (dst->head + n);
        nread = (nread + n);
        if ((dst->head == bufend)) {
            dst->head = dst->buf;
        }
    }
    return dst->head;
}

void rb_memcpy_from(void* dst, ringbuf_t* src, int count)
{
    int bytes_used = ringbuf_bytes_used(src);
    if ((count > bytes_used)) {
        return 0;
    }
    uint8_t* u8dst = ((uint8_t)dst);
    uint8_t* bufend = ringbuf_end(src);
    int nwritten = 0;
    while ((nwritten != count)) {
        __auto_type _tern_2 = (count - nwritten);
        if ((((int)(bufend - src->tail)) < (count - nwritten))) {
            _tern_2 = ((int)(bufend - src->tail));
        }
        int n = _tern_2;
        memcpy((u8dst + nwritten), src->tail, n);
        src->tail = (src->tail + n);
        nwritten = (nwritten + n);
        if ((src->tail == bufend)) {
            src->tail = src->buf;
        }
    }
    return src->tail;
}

int main(void)
{
    __ul_list_uint8 storage = {NULL, 0, 0};
    uint8_t out[] = {0, 0, 0, 0, 0, 0, 0, 0};
    ringbuf_t rb = {0};
    rb.buf = storage;
    rb.size_c = (8 + 1);
    rb.head = storage;
    rb.tail = storage;
    uint8_t data_in[] = {65, 66, 67, 68, 69, 70};
    rb_memcpy_into((&rb), data_in, 6);
    static char _cast_buf_0[64];
    snprintf(_cast_buf_0, sizeof(_cast_buf_0), "%d", ((int)(rb.head - rb.buf)));
    static char _cast_buf_1[64];
    snprintf(_cast_buf_1, sizeof(_cast_buf_1), "%d", ((int)ringbuf_bytes_used((&rb))));
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("after write 6: head_offset=", _cast_buf_0), " used="), _cast_buf_1));
    memset(out, 0, sizeof(out));
    rb_memcpy_from(out, (&rb), 3);
    static char _cast_buf_2[64];
    snprintf(_cast_buf_2, sizeof(_cast_buf_2), "%u", out[0]);
    static char _cast_buf_3[64];
    snprintf(_cast_buf_3, sizeof(_cast_buf_3), "%u", out[1]);
    static char _cast_buf_4[64];
    snprintf(_cast_buf_4, sizeof(_cast_buf_4), "%u", out[2]);
    static char _cast_buf_5[64];
    snprintf(_cast_buf_5, sizeof(_cast_buf_5), "%d", ((int)(rb.tail - rb.buf)));
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat("read 3: [", _cast_buf_2), _cast_buf_3), _cast_buf_4), "] tail_offset="), _cast_buf_5));
    uint8_t data_in2[] = {71, 72, 73, 74, 75};
    rb_memcpy_into((&rb), data_in2, 5);
    static char _cast_buf_6[64];
    snprintf(_cast_buf_6, sizeof(_cast_buf_6), "%d", ((int)(rb.head - rb.buf)));
    static char _cast_buf_7[64];
    snprintf(_cast_buf_7, sizeof(_cast_buf_7), "%d", ((int)ringbuf_bytes_used((&rb))));
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("after write 5 (wrap): head_offset=", _cast_buf_6), " used="), _cast_buf_7));
    int used = ringbuf_bytes_used((&rb));
    memset(out, 0, sizeof(out));
    rb_memcpy_from(out, (&rb), used);
    static char _cast_buf_8[64];
    snprintf(_cast_buf_8, sizeof(_cast_buf_8), "%d", ((int)used));
    printf("%s\n", __ul_strcat(__ul_strcat("read ", _cast_buf_8), " (across wrap): "));
    int i = 0;
    i = 0;
    while ((i < used)) {
        static char _cast_buf_9[64];
        snprintf(_cast_buf_9, sizeof(_cast_buf_9), "%u", out[i]);
        printf("%s\n", _cast_buf_9);
        i = (i + 1);
    }
    printf("%s\n", "");
    void* ret = rb_memcpy_from(out, (&rb), 1);
    __auto_type _tern_3 = "no";
    if ((ret == 0)) {
        _tern_3 = "yes";
    }
    printf("%s\n", __ul_strcat("underflow read returns null: ", _tern_3));
    return 0;
}


