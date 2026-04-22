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

typedef struct {
    uint32_t h[8];
    uint32_t w[16];
    int round;
    int status;
} sha256_ctx;

uint32_t K[] = {1116352408, 1899447441, 3049323471LL, 3921009573LL, 961987163, 1508970993, 2453635748LL, 2870763221LL, 3624381080LL, 310598401, 607225278, 1426881987, 1925078388, 2162078206LL, 2614888103LL, 3248222580LL};
uint32_t rotr(uint32_t x, int n);
uint32_t sigma0(uint32_t x);
uint32_t sigma1(uint32_t x);
uint32_t ch(uint32_t e, uint32_t f, uint32_t g);
uint32_t maj(uint32_t a, uint32_t b, uint32_t c);
int sha256_init(sha256_ctx* ctx);
int sha256_load_block(sha256_ctx* ctx, uint32_t w0, uint32_t w1, uint32_t w2, uint32_t w3, uint32_t w4, uint32_t w5, uint32_t w6, uint32_t w7);
int sha256_do_round(sha256_ctx* ctx, int r);
int classify_round(int r);
uint32_t get_k(int i);
int main(void);

uint32_t rotr(uint32_t x, int n)
{
    return ((x >> n) | (x << (32 - n)));
}

uint32_t sigma0(uint32_t x)
{
    return ((rotr(x, 2) ^ rotr(x, 13)) ^ rotr(x, 22));
}

uint32_t sigma1(uint32_t x)
{
    return ((rotr(x, 6) ^ rotr(x, 11)) ^ rotr(x, 25));
}

uint32_t ch(uint32_t e, uint32_t f, uint32_t g)
{
    return ((e & f) ^ ((~e) & g));
}

uint32_t maj(uint32_t a, uint32_t b, uint32_t c)
{
    return (((a & b) ^ (a & c)) ^ (b & c));
}

int sha256_init(sha256_ctx* ctx)
{
    ctx->h[0] = 1779033703;
    ctx->h[1] = 3144134277LL;
    ctx->h[2] = 1013904242;
    ctx->h[3] = 2773480762LL;
    ctx->h[4] = 1359893119;
    ctx->h[5] = 2600822924LL;
    ctx->h[6] = 528734635;
    ctx->h[7] = 1541459225;
    ctx->round = 0;
    ctx->status = 0;
    return 0;
}

int sha256_load_block(sha256_ctx* ctx, uint32_t w0, uint32_t w1, uint32_t w2, uint32_t w3, uint32_t w4, uint32_t w5, uint32_t w6, uint32_t w7)
{
    if ((ctx->status != 0)) {
        return ctx->status;
    }
    ctx->w[0] = w0;
    ctx->w[1] = w1;
    ctx->w[2] = w2;
    ctx->w[3] = w3;
    ctx->w[4] = w4;
    ctx->w[5] = w5;
    ctx->w[6] = w6;
    ctx->w[7] = w7;
    ctx->w[8] = 0;
    ctx->w[9] = 0;
    ctx->w[10] = 0;
    ctx->w[11] = 0;
    ctx->w[12] = 0;
    ctx->w[13] = 0;
    ctx->w[14] = 0;
    ctx->w[15] = 0;
    return 0;
}

int sha256_do_round(sha256_ctx* ctx, int r)
{
    uint32_t a = 0;
    uint32_t b = 0;
    uint32_t c = 0;
    uint32_t d = 0;
    uint32_t e = 0;
    uint32_t f = 0;
    uint32_t g = 0;
    uint32_t h = 0;
    uint32_t t1 = 0;
    uint32_t t2 = 0;
    if ((ctx->status != 0)) {
        return ctx->status;
    }
    if (((r < 0) || (r >= 16))) {
        ctx->status = (-1);
        return (-1);
    }
    a = ctx->h[0];
    b = ctx->h[1];
    c = ctx->h[2];
    d = ctx->h[3];
    e = ctx->h[4];
    f = ctx->h[5];
    g = ctx->h[6];
    h = ctx->h[7];
    t1 = ((((h + sigma1(e)) + ch(e, f, g)) + K[r]) + ctx->w[r]);
    t2 = (sigma0(a) + maj(a, b, c));
    ctx->h[7] = g;
    ctx->h[6] = f;
    ctx->h[5] = e;
    ctx->h[4] = (d + t1);
    ctx->h[3] = c;
    ctx->h[2] = b;
    ctx->h[1] = a;
    ctx->h[0] = (t1 + t2);
    ctx->round = (r + 1);
    return 0;
}

int classify_round(int r)
{
    if (((r % 4) == 0)) {
        return 10;
    } else {
        if (((r % 4) == 1)) {
            return 20;
        } else {
            if (((r % 4) == 2)) {
                return 30;
            } else {
                if (((r % 4) == 3)) {
                    return 40;
                } else {
                    return (-1);
                }
            }
        }
    }
}

uint32_t get_k(int i)
{
    if (((i < 0) || (i >= 16))) {
        return 0;
    }
    return K[i];
}

int main(void)
{
    sha256_ctx ctx = {0};
    int ret = 0;
    printf("%s\n", "=== 1. SHA-256 init ===");
    ret = sha256_init((&ctx));
    __auto_type _tern_1 = "FAIL";
    if ((ret == 0)) {
        _tern_1 = "PASS";
    }
    static char _cast_buf_0[64];
    snprintf(_cast_buf_0, sizeof(_cast_buf_0), "%d", ret);
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("init ret=", _cast_buf_0), " "), _tern_1));
    __auto_type _tern_2 = "FAIL";
    if ((ctx.h[0] == 1779033703)) {
        _tern_2 = "PASS";
    }
    static char _cast_buf_1[64];
    snprintf(_cast_buf_1, sizeof(_cast_buf_1), "%d", ctx.h[0]);
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("h[0]=", _cast_buf_1), " "), _tern_2));
    __auto_type _tern_3 = "FAIL";
    if ((ctx.h[7] == 1541459225)) {
        _tern_3 = "PASS";
    }
    static char _cast_buf_2[64];
    snprintf(_cast_buf_2, sizeof(_cast_buf_2), "%d", ctx.h[7]);
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("h[7]=", _cast_buf_2), " "), _tern_3));
    __auto_type _tern_4 = "FAIL";
    if ((ctx.round == 0)) {
        _tern_4 = "PASS";
    }
    static char _cast_buf_3[64];
    snprintf(_cast_buf_3, sizeof(_cast_buf_3), "%d", ctx.round);
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("round=", _cast_buf_3), " "), _tern_4));
    __auto_type _tern_5 = "FAIL";
    if ((ctx.status == 0)) {
        _tern_5 = "PASS";
    }
    static char _cast_buf_4[64];
    snprintf(_cast_buf_4, sizeof(_cast_buf_4), "%d", ctx.status);
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("status=", _cast_buf_4), " "), _tern_5));
    printf("%s\n", "\n=== 2. Load block ===");
    ret = sha256_load_block((&ctx), 1633837952, 0, 0, 0, 0, 0, 0, 0);
    __auto_type _tern_6 = "FAIL";
    if ((ret == 0)) {
        _tern_6 = "PASS";
    }
    static char _cast_buf_5[64];
    snprintf(_cast_buf_5, sizeof(_cast_buf_5), "%d", ret);
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("load ret=", _cast_buf_5), " "), _tern_6));
    __auto_type _tern_7 = "FAIL";
    if ((ctx.w[0] == 1633837952)) {
        _tern_7 = "PASS";
    }
    static char _cast_buf_6[64];
    snprintf(_cast_buf_6, sizeof(_cast_buf_6), "%d", ctx.w[0]);
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("w[0]=", _cast_buf_6), " "), _tern_7));
    __auto_type _tern_8 = "FAIL";
    if ((ctx.w[7] == 0)) {
        _tern_8 = "PASS";
    }
    static char _cast_buf_7[64];
    snprintf(_cast_buf_7, sizeof(_cast_buf_7), "%d", ctx.w[7]);
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("w[7]=", _cast_buf_7), " "), _tern_8));
    printf("%s\n", "\n=== 3. One SHA-256 round ===");
    ret = sha256_do_round((&ctx), 0);
    __auto_type _tern_9 = "FAIL";
    if ((ret == 0)) {
        _tern_9 = "PASS";
    }
    static char _cast_buf_8[64];
    snprintf(_cast_buf_8, sizeof(_cast_buf_8), "%d", ret);
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("round ret=", _cast_buf_8), " "), _tern_9));
    __auto_type _tern_10 = "FAIL";
    if ((ctx.round == 1)) {
        _tern_10 = "PASS";
    }
    static char _cast_buf_9[64];
    snprintf(_cast_buf_9, sizeof(_cast_buf_9), "%d", ctx.round);
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("round count=", _cast_buf_9), " "), _tern_10));
    __auto_type _tern_11 = "FAIL";
    if ((ctx.h[0] != 1779033703)) {
        _tern_11 = "PASS";
    }
    printf("%s\n", __ul_strcat("h[0] changed: ", _tern_11));
    __auto_type _tern_12 = "FAIL";
    if ((ctx.h[1] == 1779033703)) {
        _tern_12 = "PASS";
    }
    printf("%s\n", __ul_strcat("h[1]=old h[0]: ", _tern_12));
    printf("%s\n", "\n=== 4. Error propagation ===");
    ret = sha256_do_round((&ctx), 99);
    __auto_type _tern_13 = "FAIL";
    if ((ret == (-1))) {
        _tern_13 = "PASS";
    }
    static char _cast_buf_10[64];
    snprintf(_cast_buf_10, sizeof(_cast_buf_10), "%d", ret);
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("bad round ret=", _cast_buf_10), " "), _tern_13));
    __auto_type _tern_14 = "FAIL";
    if ((ctx.status == (-1))) {
        _tern_14 = "PASS";
    }
    static char _cast_buf_11[64];
    snprintf(_cast_buf_11, sizeof(_cast_buf_11), "%d", ctx.status);
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("status=", _cast_buf_11), " "), _tern_14));
    ret = sha256_load_block((&ctx), 0, 0, 0, 0, 0, 0, 0, 0);
    __auto_type _tern_15 = "FAIL";
    if ((ret == (-1))) {
        _tern_15 = "PASS";
    }
    static char _cast_buf_12[64];
    snprintf(_cast_buf_12, sizeof(_cast_buf_12), "%d", ret);
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("blocked load ret=", _cast_buf_12), " "), _tern_15));
    printf("%s\n", "\n=== 5. Switch/case classification ===");
    __auto_type _tern_16 = "FAIL";
    if ((classify_round(0) == 10)) {
        _tern_16 = "PASS";
    }
    static char _cast_buf_13[64];
    snprintf(_cast_buf_13, sizeof(_cast_buf_13), "%d", classify_round(0));
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("classify(0)=", _cast_buf_13), " "), _tern_16));
    __auto_type _tern_17 = "FAIL";
    if ((classify_round(1) == 20)) {
        _tern_17 = "PASS";
    }
    static char _cast_buf_14[64];
    snprintf(_cast_buf_14, sizeof(_cast_buf_14), "%d", classify_round(1));
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("classify(1)=", _cast_buf_14), " "), _tern_17));
    __auto_type _tern_18 = "FAIL";
    if ((classify_round(5) == 20)) {
        _tern_18 = "PASS";
    }
    static char _cast_buf_15[64];
    snprintf(_cast_buf_15, sizeof(_cast_buf_15), "%d", classify_round(5));
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("classify(5)=", _cast_buf_15), " "), _tern_18));
    __auto_type _tern_19 = "FAIL";
    if ((classify_round(7) == 40)) {
        _tern_19 = "PASS";
    }
    static char _cast_buf_16[64];
    snprintf(_cast_buf_16, sizeof(_cast_buf_16), "%d", classify_round(7));
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("classify(7)=", _cast_buf_16), " "), _tern_19));
    printf("%s\n", "\n=== 6. K constant table ===");
    __auto_type _tern_20 = "FAIL";
    if ((get_k(0) == 1116352408)) {
        _tern_20 = "PASS";
    }
    static char _cast_buf_17[64];
    snprintf(_cast_buf_17, sizeof(_cast_buf_17), "%u", get_k(0));
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("K[0]=", _cast_buf_17), " "), _tern_20));
    __auto_type _tern_21 = "FAIL";
    if ((get_k(7) == 2870763221LL)) {
        _tern_21 = "PASS";
    }
    static char _cast_buf_18[64];
    snprintf(_cast_buf_18, sizeof(_cast_buf_18), "%u", get_k(7));
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("K[7]=", _cast_buf_18), " "), _tern_21));
    __auto_type _tern_22 = "FAIL";
    if ((get_k((-1)) == 0)) {
        _tern_22 = "PASS";
    }
    static char _cast_buf_19[64];
    snprintf(_cast_buf_19, sizeof(_cast_buf_19), "%u", get_k((-1)));
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("K[-1]=", _cast_buf_19), " "), _tern_22));
    __auto_type _tern_23 = "FAIL";
    if ((get_k(16) == 0)) {
        _tern_23 = "PASS";
    }
    static char _cast_buf_20[64];
    snprintf(_cast_buf_20, sizeof(_cast_buf_20), "%u", get_k(16));
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("K[16]=", _cast_buf_20), " "), _tern_23));
    printf("%s\n", "\n=== 7. Determinism check ===");
    sha256_ctx c1 = {0};
    sha256_ctx c2 = {0};
    sha256_init((&c1));
    sha256_init((&c2));
    sha256_load_block((&c1), 1633837952, 0, 0, 0, 0, 0, 0, 0);
    sha256_load_block((&c2), 1633837952, 0, 0, 0, 0, 0, 0, 0);
    sha256_do_round((&c1), 0);
    sha256_do_round((&c2), 0);
    __auto_type _tern_24 = "FAIL";
    if ((c1.h[0] == c2.h[0])) {
        _tern_24 = "PASS";
    }
    printf("%s\n", __ul_strcat("two-ctx match: ", _tern_24));
    sha256_do_round((&c1), 1);
    sha256_do_round((&c2), 1);
    __auto_type _tern_25 = "FAIL";
    if ((c1.h[0] == c2.h[0])) {
        _tern_25 = "PASS";
    }
    printf("%s\n", __ul_strcat("after round 1: ", _tern_25));
    return 0;
}


