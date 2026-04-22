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
    int nr;
    uint32_t* rk;
    uint32_t rk_buf[68];
} mbedtls_aes_context;

typedef struct {
    mbedtls_aes_context crypt;
    mbedtls_aes_context tweak;
} mbedtls_aes_xts_context;

typedef struct {
    char* name;
    int* setkey_enc;
    void* init;
    void* free;
} mbedtls_aes_ops_t;

uint8_t FSb[] = {99, 124, 119, 123, 242, 107, 111, 197, 48, 1, 103, 43, 254, 215, 171, 118, 202, 130, 201, 125, 250, 89, 71, 240, 173, 212, 162, 175, 156, 164, 114, 192, 183, 253, 147, 38, 54, 63, 247, 204, 52, 165, 229, 241, 113, 216, 49, 21, 4, 199, 35, 195, 24, 150, 5, 154, 7, 18, 128, 226, 235, 39, 178, 117, 9, 131, 44, 26, 27, 110, 90, 160, 82, 59, 214, 179, 41, 227, 47, 132, 83, 209, 0, 237, 32, 252, 177, 91, 106, 203, 190, 57, 74, 76, 88, 207, 208, 239, 170, 251, 67, 77, 51, 133, 69, 249, 2, 127, 80, 60, 159, 168, 81, 163, 64, 143, 146, 157, 56, 245, 188, 182, 218, 33, 16, 255, 243, 210, 205, 12, 19, 236, 95, 151, 68, 23, 196, 167, 126, 61, 100, 93, 25, 115, 96, 129, 79, 220, 34, 42, 144, 136, 70, 238, 184, 20, 222, 94, 11, 219, 224, 50, 58, 10, 73, 6, 36, 92, 194, 211, 172, 98, 145, 149, 228, 121, 231, 200, 55, 109, 141, 213, 78, 169, 108, 86, 244, 234, 101, 122, 174, 8, 186, 120, 37, 46, 28, 166, 180, 198, 232, 221, 116, 31, 75, 189, 139, 138, 112, 62, 181, 102, 72, 3, 246, 14, 97, 53, 87, 185, 134, 193, 29, 158, 225, 248, 152, 17, 105, 217, 142, 148, 155, 30, 135, 233, 206, 85, 40, 223, 140, 161, 137, 13, 191, 230, 66, 104, 65, 153, 45, 15, 176, 84, 187, 22};
uint8_t RSb[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint8_t Rcon[] = {0, 1, 2, 4, 8, 16, 32, 64, 128, 27, 54};
mbedtls_aes_ops_t mbedtls_aes_ops = {0};
uint8_t xtime(uint8_t x);
void aes_gen_tables(void);
uint32_t SubWord(uint32_t x);
uint32_t RotWord(uint32_t x);
void mbedtls_aes_init(mbedtls_aes_context* ctx);
void mbedtls_aes_free(mbedtls_aes_context* ctx);
int mbedtls_aes_setkey_enc(mbedtls_aes_context* ctx, uint8_t* key, int keybits);
int aes_setkey_enc_wrap(mbedtls_aes_context* ctx, uint8_t* key, int keybits);
void aes_init_wrap(mbedtls_aes_context* ctx);
void aes_free_wrap(mbedtls_aes_context* ctx);
int main(void);

uint8_t xtime(uint8_t x)
{
    __auto_type _tern_1 = 0;
    if ((x & 128)) {
        _tern_1 = 27;
    }
    return ((int)((x << 1) ^ _tern_1));
}

void aes_gen_tables(void)
{
    int i = 0;
    i = 0;
    while ((i < 256)) {
        RSb[FSb[i]] = ((int)i);
        i = (i + 1);
    }
}

uint32_t SubWord(uint32_t x)
{
    return (((((int)FSb[(x & 255)]) | (((int)FSb[((x >> 8) & 255)]) << 8)) | (((int)FSb[((x >> 16) & 255)]) << 16)) | (((int)FSb[((x >> 24) & 255)]) << 24));
}

uint32_t RotWord(uint32_t x)
{
    return ((x >> 8) | (x << 24));
}

void mbedtls_aes_init(mbedtls_aes_context* ctx)
{
    memset(ctx, 0, sizeof(mbedtls_aes_context));
}

void mbedtls_aes_free(mbedtls_aes_context* ctx)
{
    if ((ctx == 0)) {
        return;
    }
    memset(ctx, 0, sizeof(mbedtls_aes_context));
}

int mbedtls_aes_setkey_enc(mbedtls_aes_context* ctx, uint8_t* key, int keybits)
{
    int i = 0;
    uint32_t RK = 0;
    if ((((keybits != 128) && (keybits != 192)) && (keybits != 256))) {
        return (-32);
    }
    ctx->rk = ctx->rk_buf;
    RK = ctx->rk;
    if ((keybits == 128)) {
        ctx->nr = 10;
    } else {
        if ((keybits == 192)) {
            ctx->nr = 12;
        } else {
            if ((keybits == 256)) {
                ctx->nr = 14;
            } else {
                return (-32);
            }
        }
    }
    int Nk = (keybits / 32);
    i = 0;
    while ((i < Nk)) {
        RK[i] = (((((int)key[(4 * i)]) | (((int)key[((4 * i) + 1)]) << 8)) | (((int)key[((4 * i) + 2)]) << 16)) | (((int)key[((4 * i) + 3)]) << 24));
        i = (i + 1);
    }
    int Nr = ctx->nr;
    i = Nk;
    while ((i < ((Nr + 1) * 4))) {
        uint32_t temp = RK[(i - 1)];
        if (((i % Nk) == 0)) {
            temp = (SubWord(RotWord(temp)) ^ ((int)Rcon[(i / Nk)]));
        } else {
            if (((Nk > 6) && ((i % Nk) == 4))) {
                temp = SubWord(temp);
            }
        }
        RK[i] = (RK[(i - Nk)] ^ temp);
        i = (i + 1);
    }
    return 0;
}

int aes_setkey_enc_wrap(mbedtls_aes_context* ctx, uint8_t* key, int keybits)
{
    return mbedtls_aes_setkey_enc(ctx, key, keybits);
}

void aes_init_wrap(mbedtls_aes_context* ctx)
{
    mbedtls_aes_init(ctx);
}

void aes_free_wrap(mbedtls_aes_context* ctx)
{
    mbedtls_aes_free(ctx);
}

int main(void)
{
    int i = 0;
    int pass = 0;
    aes_gen_tables();
    printf("%s\n", "=== AES S-box (FSb) ===");
    __auto_type _tern_2 = "FAIL";
    if ((FSb[0] == 99)) {
        _tern_2 = "PASS";
    }
    static char _cast_buf_0[64];
    snprintf(_cast_buf_0, sizeof(_cast_buf_0), "%u", FSb[0]);
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("FSb[0]=0x", _cast_buf_0), " "), _tern_2));
    __auto_type _tern_3 = "FAIL";
    if ((FSb[255] == 22)) {
        _tern_3 = "PASS";
    }
    static char _cast_buf_1[64];
    snprintf(_cast_buf_1, sizeof(_cast_buf_1), "%u", FSb[255]);
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("FSb[0xFF]=0x", _cast_buf_1), " "), _tern_3));
    printf("%s\n", "\n=== AES Inverse S-box (RSb) ===");
    pass = 1;
    i = 0;
    while ((i < 256)) {
        if ((RSb[FSb[i]] != ((int)i))) {
            static char _cast_buf_2[64];
            snprintf(_cast_buf_2, sizeof(_cast_buf_2), "%d", i);
            static char _cast_buf_3[64];
            snprintf(_cast_buf_3, sizeof(_cast_buf_3), "%u", RSb[FSb[i]]);
            static char _cast_buf_4[64];
            snprintf(_cast_buf_4, sizeof(_cast_buf_4), "%d", i);
            printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat("FAIL RSb[FSb[", _cast_buf_2), "]] = "), _cast_buf_3), " (expected "), _cast_buf_4), ")"));
            pass = 0;
        }
        i = (i + 1);
    }
    __auto_type _tern_4 = "FAIL";
    if (pass) {
        _tern_4 = "PASS";
    }
    printf("%s\n", __ul_strcat("RSb is inverse of FSb: ", _tern_4));
    printf("%s\n", "\n=== FSb is a permutation ===");
    int counts[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    memset(counts, 0, sizeof(counts));
    i = 0;
    while ((i < 256)) {
        counts[FSb[i]] = (counts[FSb[i]] + 1);
        i = (i + 1);
    }
    pass = 1;
    i = 0;
    while ((i < 256)) {
        if ((counts[i] != 1)) {
            static char _cast_buf_5[64];
            snprintf(_cast_buf_5, sizeof(_cast_buf_5), "%d", i);
            static char _cast_buf_6[64];
            snprintf(_cast_buf_6, sizeof(_cast_buf_6), "%d", counts[i]);
            printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat("FAIL FSb not bijective: value ", _cast_buf_5), " appears "), _cast_buf_6), " times"));
            pass = 0;
        }
        i = (i + 1);
    }
    __auto_type _tern_5 = "FAIL";
    if (pass) {
        _tern_5 = "PASS";
    }
    printf("%s\n", __ul_strcat("FSb bijective: ", _tern_5));
    printf("%s\n", "\n=== SubWord ===");
    uint32_t sw = SubWord(66051);
    __auto_type _tern_6 = "FAIL";
    if ((sw == 1669101435)) {
        _tern_6 = "PASS";
    }
    static char _cast_buf_7[64];
    snprintf(_cast_buf_7, sizeof(_cast_buf_7), "%u", sw);
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("SubWord(0x00010203)=0x", _cast_buf_7), " "), _tern_6));
    printf("%s\n", "\n=== AES-128 key schedule ===");
    mbedtls_aes_context ctx = {0};
    mbedtls_aes_init((&ctx));
    uint8_t key128[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    int ret = mbedtls_aes_setkey_enc((&ctx), key128, 128);
    __auto_type _tern_7 = "FAIL";
    if ((ret == 0)) {
        _tern_7 = "PASS";
    }
    static char _cast_buf_8[64];
    snprintf(_cast_buf_8, sizeof(_cast_buf_8), "%d", ret);
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("setkey_enc 128: ret=", _cast_buf_8), " "), _tern_7));
    __auto_type _tern_8 = "FAIL";
    if ((ctx.rk[4] == 4252281558LL)) {
        _tern_8 = "PASS";
    }
    static char _cast_buf_9[64];
    snprintf(_cast_buf_9, sizeof(_cast_buf_9), "%d", ctx.rk[4]);
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("RK[4]=0x", _cast_buf_9), " "), _tern_8));
    __auto_type _tern_9 = "FAIL";
    if ((ctx.rk[7] == 4269190102LL)) {
        _tern_9 = "PASS";
    }
    static char _cast_buf_10[64];
    snprintf(_cast_buf_10, sizeof(_cast_buf_10), "%d", ctx.rk[7]);
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("RK[7]=0x", _cast_buf_10), " "), _tern_9));
    mbedtls_aes_free((&ctx));
    printf("%s\n", "\n=== Invalid key length ===");
    memset(&ctx, 0, sizeof(ctx));
    mbedtls_aes_init((&ctx));
    uint8_t bad_key[] = {0};
    ret = mbedtls_aes_setkey_enc((&ctx), bad_key, 64);
    __auto_type _tern_10 = "FAIL";
    if ((ret == (-32))) {
        _tern_10 = "PASS";
    }
    static char _cast_buf_11[64];
    snprintf(_cast_buf_11, sizeof(_cast_buf_11), "%d", ret);
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("setkey 64-bit: ret=", _cast_buf_11), " "), _tern_10));
    mbedtls_aes_free((&ctx));
    printf("%s\n", "\n=== AES XTS nested context ===");
    mbedtls_aes_xts_context xts = {0};
    memset((&xts), 0, sizeof(xts));
    mbedtls_aes_init((&xts.crypt));
    mbedtls_aes_init((&xts.tweak));
    uint8_t key1[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    uint8_t key2[] = {255, 254, 253, 252, 251, 250, 249, 248, 247, 246, 245, 244, 243, 242, 241, 240};
    int r1 = mbedtls_aes_setkey_enc((&xts.crypt), key1, 128);
    int r2 = mbedtls_aes_setkey_enc((&xts.tweak), key2, 128);
    __auto_type _tern_11 = "FAIL";
    if (((r1 == 0) && (r2 == 0))) {
        _tern_11 = "PASS";
    }
    static char _cast_buf_12[64];
    snprintf(_cast_buf_12, sizeof(_cast_buf_12), "%d", r1);
    static char _cast_buf_13[64];
    snprintf(_cast_buf_13, sizeof(_cast_buf_13), "%d", r2);
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat("XTS crypt/tweak setkey: r1=", _cast_buf_12), " r2="), _cast_buf_13), " "), _tern_11));
    __auto_type _tern_12 = "FAIL";
    if (((xts.crypt.nr == 10) && (xts.tweak.nr == 10))) {
        _tern_12 = "PASS";
    }
    static char _cast_buf_14[64];
    snprintf(_cast_buf_14, sizeof(_cast_buf_14), "%d", xts.crypt.nr);
    static char _cast_buf_15[64];
    snprintf(_cast_buf_15, sizeof(_cast_buf_15), "%d", xts.tweak.nr);
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat("XTS crypt.nr=", _cast_buf_14), " tweak.nr="), _cast_buf_15), " "), _tern_12));
    mbedtls_aes_free((&xts.crypt));
    mbedtls_aes_free((&xts.tweak));
    printf("%s\n", "\n=== AES vtable dispatch ===");
    memset(&ctx, 0, sizeof(ctx));
    mbedtls_aes_ops_t ops = (&mbedtls_aes_ops);
    ((ops.init)((&ctx)));
    printf("%s\n", __ul_strcat("vtable name=", ops.name));
    uint8_t key[] = {43, 126, 21, 22, 40, 174, 210, 166, 171, 247, 21, 136, 9, 207, 79, 60};
    ret = ((ops.setkey_enc)((&ctx), key, 128));
    __auto_type _tern_13 = "FAIL";
    if ((ret == 0)) {
        _tern_13 = "PASS";
    }
    static char _cast_buf_16[64];
    snprintf(_cast_buf_16, sizeof(_cast_buf_16), "%d", ret);
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("vtable setkey_enc: ret=", _cast_buf_16), " "), _tern_13));
    __auto_type _tern_14 = "FAIL";
    if ((ctx.rk[4] == 402586272)) {
        _tern_14 = "PASS";
    }
    static char _cast_buf_17[64];
    snprintf(_cast_buf_17, sizeof(_cast_buf_17), "%d", ctx.rk[4]);
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("vtable RK[4]=0x", _cast_buf_17), " "), _tern_14));
    ((ops.free)((&ctx)));
    return 0;
}


