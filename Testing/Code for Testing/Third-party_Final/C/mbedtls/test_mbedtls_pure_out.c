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
    uint32_t state[4];
    uint32_t total[2];
    int last_ret;
} md_ctx_t;

typedef struct {
    char* name;
    int output_size;
    int* init_fn;
    int* chain_fn;
} md_dispatch_t;

typedef struct {
    md_ctx_t inner;
    md_ctx_t outer;
    int mode;
} hmac_ctx_t;

uint8_t FSb[] = {99, 124, 119, 123, 242, 107, 111, 197, 48, 1, 103, 43, 254, 215, 171, 118, 202, 130, 201, 125, 250, 89, 71, 240, 173, 212, 162, 175, 156, 164, 114, 192, 183, 253, 147, 38, 54, 63, 247, 204, 52, 165, 229, 241, 113, 216, 49, 21, 4, 199, 35, 195, 24, 150, 5, 154, 7, 18, 128, 226, 235, 39, 178, 117, 9, 131, 44, 26, 27, 110, 90, 160, 82, 59, 214, 179, 41, 227, 47, 132, 83, 209, 0, 237, 32, 252, 177, 91, 106, 203, 190, 57, 74, 76, 88, 207, 208, 239, 170, 251, 67, 77, 51, 133, 69, 249, 2, 127, 80, 60, 159, 168, 81, 163, 64, 143, 146, 157, 56, 245, 188, 182, 218, 33, 16, 255, 243, 210, 205, 12, 19, 236, 95, 151, 68, 23, 196, 167, 126, 61, 100, 93, 25, 115, 96, 129, 79, 220, 34, 42, 144, 136, 70, 238, 184, 20, 222, 94, 11, 219, 224, 50, 58, 10, 73, 6, 36, 92, 194, 211, 172, 98, 145, 149, 228, 121, 231, 200, 55, 109, 141, 213, 78, 169, 108, 86, 244, 234, 101, 122, 174, 8, 186, 120, 37, 46, 28, 166, 180, 198, 232, 221, 116, 31, 75, 189, 139, 138, 112, 62, 181, 102, 72, 3, 246, 14, 97, 53, 87, 185, 134, 193, 29, 158, 225, 248, 152, 17, 105, 217, 142, 148, 155, 30, 135, 233, 206, 85, 40, 223, 140, 161, 137, 13, 191, 230, 66, 104, 65, 153, 45, 15, 176, 84, 187, 22};
md_dispatch_t md5_dispatch = {0};
uint8_t ct_in_range(uint8_t lo, uint8_t hi, uint8_t c, uint8_t val);
uint8_t b64_enc_char(uint8_t v);
int b64_dec_value(uint8_t c);
uint32_t SubWord(uint32_t x);
int md_init(md_ctx_t* ctx);
int md_chain(md_ctx_t* ctx, int step);
int hmac_init(hmac_ctx_t* ctx, int mode);
int get_nr_rounds(int keybits);
uint32_t sbox_checksum(int lo, int hi);
int main(void);

uint8_t ct_in_range(uint8_t lo, uint8_t hi, uint8_t c, uint8_t val)
{
    __auto_type _tern_1 = 0;
    if (((int)(((int)((1 + hi) - lo)) > ((int)(c - lo))))) {
        _tern_1 = 255;
    }
    uint8_t in_range = _tern_1;
    return ((int)(in_range & val));
}

uint8_t b64_enc_char(uint8_t v)
{
    uint8_t d = 0;
    d = ((int)(d | ct_in_range(0, 25, v, ((int)(65 + v)))));
    d = ((int)(d | ct_in_range(26, 51, v, ((int)((97 + v) - 26)))));
    d = ((int)(d | ct_in_range(52, 61, v, ((int)((48 + v) - 52)))));
    d = ((int)(d | ct_in_range(62, 62, v, 43)));
    d = ((int)(d | ct_in_range(63, 63, v, 47)));
    return d;
}

int b64_dec_value(uint8_t c)
{
    uint8_t val = 0;
    val = ((int)(val | ct_in_range(65, 90, c, ((int)(((c - 65) + 0) + 1)))));
    val = ((int)(val | ct_in_range(97, 122, c, ((int)(((c - 97) + 26) + 1)))));
    val = ((int)(val | ct_in_range(48, 57, c, ((int)(((c - 48) + 52) + 1)))));
    val = ((int)(val | ct_in_range(43, 43, c, ((int)(((c - 43) + 62) + 1)))));
    val = ((int)(val | ct_in_range(47, 47, c, ((int)(((c - 47) + 63) + 1)))));
    return (((int)val) - 1);
}

uint32_t SubWord(uint32_t x)
{
    return (((((int)FSb[(x & 255)]) | (((int)FSb[((x >> 8) & 255)]) << 8)) | (((int)FSb[((x >> 16) & 255)]) << 16)) | (((int)FSb[((x >> 24) & 255)]) << 24));
}

int md_init(md_ctx_t* ctx)
{
    ctx->state[0] = 1732584193;
    ctx->state[1] = 4023233417LL;
    ctx->state[2] = 2562383102LL;
    ctx->state[3] = 271733878;
    ctx->total[0] = 0;
    ctx->total[1] = 0;
    ctx->last_ret = 0;
    return 0;
}

int md_chain(md_ctx_t* ctx, int step)
{
    int ret = (-110);
    ret = md_init(ctx);
    if ((ret != 0)) {
        return ret;
    }
    if ((step > 0)) {
        ctx->state[0] = (ctx->state[0] ^ ((int)step));
        ret = 0;
        if ((ret != 0)) {
            return ret;
        }
    }
    if ((step < 0)) {
        ret = (-110);
        return ret;
    }
    ctx->last_ret = ret;
    return ret;
}

int hmac_init(hmac_ctx_t* ctx, int mode)
{
    int ret = (-110);
    ctx->mode = mode;
    ret = md_init((&ctx->inner));
    if ((ret != 0)) {
        return ret;
    }
    ret = md_init((&ctx->outer));
    if ((ret != 0)) {
        return ret;
    }
    ctx->inner.state[0] = (ctx->inner.state[0] ^ 909522486);
    ctx->outer.state[0] = (ctx->outer.state[0] ^ 1549556828);
    return 0;
}

int get_nr_rounds(int keybits)
{
    int nr = 0;
    if ((keybits == 128)) {
        nr = 10;
    } else {
        if ((keybits == 192)) {
            nr = 12;
        } else {
            if ((keybits == 256)) {
                nr = 14;
            } else {
                return (-1);
            }
        }
    }
    return nr;
}

uint32_t sbox_checksum(int lo, int hi)
{
    int i = 0;
    uint32_t acc = 0;
    i = lo;
    while ((i <= hi)) {
        acc = (acc ^ ((int)FSb[i]));
        acc = ((acc << 1) | (acc >> 31));
        i = (i + 1);
    }
    return acc;
}

int main(void)
{
    int i = 0;
    int pass = 0;
    printf("%s\n", "=== S1: base64 enc/dec ===");
    char* alph = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    pass = 1;
    i = 0;
    while ((i < 64)) {
        uint8_t enc = b64_enc_char(((int)i));
        int dec = b64_dec_value(enc);
        if (((enc != ((int)str_char_at(alph, i))) || (dec != i))) {
            static char _cast_buf_0[64];
            snprintf(_cast_buf_0, sizeof(_cast_buf_0), "%d", i);
            static char _cast_buf_1[64];
            snprintf(_cast_buf_1, sizeof(_cast_buf_1), "%u", enc);
            static char _cast_buf_2[64];
            snprintf(_cast_buf_2, sizeof(_cast_buf_2), "%u", enc);
            static char _cast_buf_3[64];
            snprintf(_cast_buf_3, sizeof(_cast_buf_3), "%d", dec);
            printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat("FAIL i=", _cast_buf_0), " enc="), _cast_buf_1), "("), _cast_buf_2), ") dec="), _cast_buf_3));
            pass = 0;
        }
        i = (i + 1);
    }
    __auto_type _tern_2 = "FAIL";
    if (pass) {
        _tern_2 = "PASS";
    }
    printf("%s\n", __ul_strcat("enc/dec round-trip 0..63: ", _tern_2));
    __auto_type _tern_3 = "FAIL";
    if ((b64_enc_char(0) == 65)) {
        _tern_3 = "PASS";
    }
    static char _cast_buf_4[64];
    snprintf(_cast_buf_4, sizeof(_cast_buf_4), "%u", b64_enc_char(0));
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("b64_enc_char(0)=", _cast_buf_4), " "), _tern_3));
    __auto_type _tern_4 = "FAIL";
    if ((b64_enc_char(63) == 47)) {
        _tern_4 = "PASS";
    }
    static char _cast_buf_5[64];
    snprintf(_cast_buf_5, sizeof(_cast_buf_5), "%u", b64_enc_char(63));
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("b64_enc_char(63)=", _cast_buf_5), " "), _tern_4));
    __auto_type _tern_5 = "FAIL";
    if ((b64_dec_value(65) == 0)) {
        _tern_5 = "PASS";
    }
    static char _cast_buf_6[64];
    snprintf(_cast_buf_6, sizeof(_cast_buf_6), "%d", b64_dec_value(65));
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("b64_dec_value('A')=", _cast_buf_6), " "), _tern_5));
    __auto_type _tern_6 = "FAIL";
    if ((b64_dec_value(47) == 63)) {
        _tern_6 = "PASS";
    }
    static char _cast_buf_7[64];
    snprintf(_cast_buf_7, sizeof(_cast_buf_7), "%d", b64_dec_value(47));
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("b64_dec_value('/')=", _cast_buf_7), " "), _tern_6));
    __auto_type _tern_7 = "FAIL";
    if ((b64_dec_value(33) == (-1))) {
        _tern_7 = "PASS";
    }
    static char _cast_buf_8[64];
    snprintf(_cast_buf_8, sizeof(_cast_buf_8), "%d", b64_dec_value(33));
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("b64_dec_value('!')=", _cast_buf_8), " "), _tern_7));
    printf("%s\n", "\n=== S2: AES FSb lookup ===");
    __auto_type _tern_8 = "FAIL";
    if ((FSb[0] == 99)) {
        _tern_8 = "PASS";
    }
    static char _cast_buf_9[64];
    snprintf(_cast_buf_9, sizeof(_cast_buf_9), "%u", FSb[0]);
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("FSb[0]=0x", _cast_buf_9), " "), _tern_8));
    __auto_type _tern_9 = "FAIL";
    if ((FSb[1] == 124)) {
        _tern_9 = "PASS";
    }
    static char _cast_buf_10[64];
    snprintf(_cast_buf_10, sizeof(_cast_buf_10), "%u", FSb[1]);
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("FSb[1]=0x", _cast_buf_10), " "), _tern_9));
    __auto_type _tern_10 = "FAIL";
    if ((FSb[255] == 22)) {
        _tern_10 = "PASS";
    }
    static char _cast_buf_11[64];
    snprintf(_cast_buf_11, sizeof(_cast_buf_11), "%u", FSb[255]);
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("FSb[255]=0x", _cast_buf_11), " "), _tern_10));
    uint32_t sw = SubWord(66051);
    __auto_type _tern_11 = "FAIL";
    if ((sw == 1669101435)) {
        _tern_11 = "PASS";
    }
    static char _cast_buf_12[64];
    snprintf(_cast_buf_12, sizeof(_cast_buf_12), "%u", sw);
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("SubWord(0x00010203)=0x", _cast_buf_12), " "), _tern_11));
    int counts[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    i = 0;
    while ((i < 256)) {
        counts[i] = 0;
        i = (i + 1);
    }
    i = 0;
    while ((i < 256)) {
        counts[FSb[i]] = (counts[FSb[i]] + 1);
        i = (i + 1);
    }
    pass = 1;
    int _brk_0 = 0;
    i = 0;
    while (((i < 256) && (!_brk_0))) {
        if ((counts[i] != 1)) {
            pass = 0;
            _brk_0 = 1;
        }
        if ((!_brk_0)) {
            i = (i + 1);
        }
    }
    __auto_type _tern_12 = "FAIL";
    if (pass) {
        _tern_12 = "PASS";
    }
    printf("%s\n", __ul_strcat("FSb bijective: ", _tern_12));
    printf("%s\n", "\n=== S3+4: MD5 ctx + error propagation ===");
    md_ctx_t ctx = {0};
    int ret = md_init((&ctx));
    __auto_type _tern_13 = "FAIL";
    if ((ret == 0)) {
        _tern_13 = "PASS";
    }
    static char _cast_buf_13[64];
    snprintf(_cast_buf_13, sizeof(_cast_buf_13), "%d", ret);
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("md_init ret=", _cast_buf_13), " "), _tern_13));
    __auto_type _tern_14 = "FAIL";
    if ((ctx.state[0] == 1732584193)) {
        _tern_14 = "PASS";
    }
    static char _cast_buf_14[64];
    snprintf(_cast_buf_14, sizeof(_cast_buf_14), "%d", ctx.state[0]);
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("state[0]=0x", _cast_buf_14), " "), _tern_14));
    __auto_type _tern_15 = "FAIL";
    if ((ctx.state[3] == 271733878)) {
        _tern_15 = "PASS";
    }
    static char _cast_buf_15[64];
    snprintf(_cast_buf_15, sizeof(_cast_buf_15), "%d", ctx.state[3]);
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("state[3]=0x", _cast_buf_15), " "), _tern_15));
    ret = md_chain((&ctx), 5);
    __auto_type _tern_16 = "FAIL";
    if ((ret == 0)) {
        _tern_16 = "PASS";
    }
    static char _cast_buf_16[64];
    snprintf(_cast_buf_16, sizeof(_cast_buf_16), "%d", ret);
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("md_chain(5) ret=", _cast_buf_16), " "), _tern_16));
    __auto_type _tern_17 = "FAIL";
    if ((ctx.state[0] == 1732584196)) {
        _tern_17 = "PASS";
    }
    static char _cast_buf_17[64];
    snprintf(_cast_buf_17, sizeof(_cast_buf_17), "%d", ctx.state[0]);
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("chain state[0]=0x", _cast_buf_17), " "), _tern_17));
    ret = md_chain((&ctx), (-1));
    __auto_type _tern_18 = "FAIL";
    if ((ret == (-110))) {
        _tern_18 = "PASS";
    }
    static char _cast_buf_18[64];
    snprintf(_cast_buf_18, sizeof(_cast_buf_18), "%d", ret);
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("md_chain(-1) ret=", _cast_buf_18), " "), _tern_18));
    printf("%s\n", "\n=== S5: vtable dispatch ===");
    md_dispatch_t ops = (&md5_dispatch);
    memset(&ctx, 0, sizeof(ctx));
    static char _cast_buf_19[64];
    snprintf(_cast_buf_19, sizeof(_cast_buf_19), "%d", ops.output_size);
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("vtable name=", ops.name), " output_size="), _cast_buf_19));
    ret = ((ops.init_fn)((&ctx)));
    __auto_type _tern_19 = "FAIL";
    if ((ret == 0)) {
        _tern_19 = "PASS";
    }
    static char _cast_buf_20[64];
    snprintf(_cast_buf_20, sizeof(_cast_buf_20), "%d", ret);
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("vtable init_fn ret=", _cast_buf_20), " "), _tern_19));
    __auto_type _tern_20 = "FAIL";
    if ((ctx.state[1] == 4023233417LL)) {
        _tern_20 = "PASS";
    }
    static char _cast_buf_21[64];
    snprintf(_cast_buf_21, sizeof(_cast_buf_21), "%d", ctx.state[1]);
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("vtable state[1]=0x", _cast_buf_21), " "), _tern_20));
    ret = ((ops.chain_fn)((&ctx), 3));
    __auto_type _tern_21 = "FAIL";
    if ((ret == 0)) {
        _tern_21 = "PASS";
    }
    static char _cast_buf_22[64];
    snprintf(_cast_buf_22, sizeof(_cast_buf_22), "%d", ret);
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("vtable chain_fn(3) ret=", _cast_buf_22), " "), _tern_21));
    printf("%s\n", "\n=== S6: nested struct (HMAC) ===");
    hmac_ctx_t hctx = {0};
    ret = hmac_init((&hctx), 1);
    __auto_type _tern_22 = "FAIL";
    if ((ret == 0)) {
        _tern_22 = "PASS";
    }
    static char _cast_buf_23[64];
    snprintf(_cast_buf_23, sizeof(_cast_buf_23), "%d", ret);
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("hmac_init ret=", _cast_buf_23), " "), _tern_22));
    __auto_type _tern_23 = "FAIL";
    if ((hctx.inner.state[0] == 1366496567)) {
        _tern_23 = "PASS";
    }
    static char _cast_buf_24[64];
    snprintf(_cast_buf_24, sizeof(_cast_buf_24), "%d", hctx.inner.state[0]);
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("inner.state[0]=0x", _cast_buf_24), " "), _tern_23));
    __auto_type _tern_24 = "FAIL";
    if ((hctx.outer.state[0] == 991526749)) {
        _tern_24 = "PASS";
    }
    static char _cast_buf_25[64];
    snprintf(_cast_buf_25, sizeof(_cast_buf_25), "%d", hctx.outer.state[0]);
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("outer.state[0]=0x", _cast_buf_25), " "), _tern_24));
    __auto_type _tern_25 = "FAIL";
    if ((hctx.mode == 1)) {
        _tern_25 = "PASS";
    }
    static char _cast_buf_26[64];
    snprintf(_cast_buf_26, sizeof(_cast_buf_26), "%d", hctx.mode);
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("mode=", _cast_buf_26), " "), _tern_25));
    printf("%s\n", "\n=== S7: switch/case (key rounds) ===");
    __auto_type _tern_26 = "FAIL";
    if ((get_nr_rounds(128) == 10)) {
        _tern_26 = "PASS";
    }
    static char _cast_buf_27[64];
    snprintf(_cast_buf_27, sizeof(_cast_buf_27), "%d", get_nr_rounds(128));
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("get_nr_rounds(128)=", _cast_buf_27), " "), _tern_26));
    __auto_type _tern_27 = "FAIL";
    if ((get_nr_rounds(192) == 12)) {
        _tern_27 = "PASS";
    }
    static char _cast_buf_28[64];
    snprintf(_cast_buf_28, sizeof(_cast_buf_28), "%d", get_nr_rounds(192));
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("get_nr_rounds(192)=", _cast_buf_28), " "), _tern_27));
    __auto_type _tern_28 = "FAIL";
    if ((get_nr_rounds(256) == 14)) {
        _tern_28 = "PASS";
    }
    static char _cast_buf_29[64];
    snprintf(_cast_buf_29, sizeof(_cast_buf_29), "%d", get_nr_rounds(256));
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("get_nr_rounds(256)=", _cast_buf_29), " "), _tern_28));
    __auto_type _tern_29 = "FAIL";
    if ((get_nr_rounds(64) == (-1))) {
        _tern_29 = "PASS";
    }
    static char _cast_buf_30[64];
    snprintf(_cast_buf_30, sizeof(_cast_buf_30), "%d", get_nr_rounds(64));
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("get_nr_rounds(64)=", _cast_buf_30), " "), _tern_29));
    printf("%s\n", "\n=== S8: const array checksum ===");
    uint32_t cs = sbox_checksum(0, 255);
    static char _cast_buf_31[64];
    snprintf(_cast_buf_31, sizeof(_cast_buf_31), "%u", cs);
    printf("%s\n", __ul_strcat("sbox_checksum(0,255)=0x", _cast_buf_31));
    uint32_t ref = sbox_checksum(0, 255);
    __auto_type _tern_30 = "FAIL";
    if ((cs == ref)) {
        _tern_30 = "PASS";
    }
    printf("%s\n", __ul_strcat("sbox_checksum deterministic: ", _tern_30));
    uint32_t cs4 = sbox_checksum(0, 3);
    static char _cast_buf_32[64];
    snprintf(_cast_buf_32, sizeof(_cast_buf_32), "%u", cs4);
    printf("%s\n", __ul_strcat("sbox_checksum(0,3)=0x", _cast_buf_32));
    uint32_t cs4_ref = sbox_checksum(0, 3);
    __auto_type _tern_31 = "FAIL";
    if ((cs4 == cs4_ref)) {
        _tern_31 = "PASS";
    }
    printf("%s\n", __ul_strcat("sbox_checksum(0,3) deterministic: ", _tern_31));
    return 0;
}


