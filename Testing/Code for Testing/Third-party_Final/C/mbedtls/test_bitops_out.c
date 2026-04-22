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

uint8_t FSb[] = {99, 124, 119, 123, 242, 107, 111, 197, 48, 1, 103, 43, 254, 215, 171, 118, 202, 130, 201, 125, 250, 89, 71, 240, 173, 212, 162, 175, 156, 164, 114, 192, 183, 253, 147, 38, 54, 63, 247, 204, 52, 165, 229, 241, 113, 216, 49, 21, 4, 199, 35, 195, 24, 150, 5, 154, 7, 18, 128, 226, 235, 39, 178, 117, 9, 131, 44, 26, 27, 110, 90, 160, 82, 59, 214, 179, 41, 227, 47, 132, 83, 209, 0, 237, 32, 252, 177, 91, 106, 203, 190, 57, 74, 76, 88, 207, 208, 239, 170, 251, 67, 77, 51, 133, 69, 249, 2, 127, 80, 60, 159, 168, 81, 163, 64, 143, 146, 157, 56, 245, 188, 182, 218, 33, 16, 255, 243, 210, 205, 12, 19, 236, 95, 151, 68, 23, 196, 167, 126, 61, 100, 93, 25, 115, 96, 129, 79, 220, 34, 42, 144, 136, 70, 238, 184, 20, 222, 94, 11, 219, 224, 50, 58, 10, 73, 6, 36, 92, 194, 211, 172, 98, 145, 149, 228, 121, 231, 200, 55, 109, 141, 213, 78, 169, 108, 86, 244, 234, 101, 122, 174, 8, 186, 120, 37, 46, 28, 166, 180, 198, 232, 221, 116, 31, 75, 189, 139, 138, 112, 62, 181, 102, 72, 3, 246, 14, 97, 53, 87, 185, 134, 193, 29, 158, 225, 248, 152, 17, 105, 217, 142, 148, 155, 30, 135, 233, 206, 85, 40, 223, 140, 161, 137, 13, 191, 230, 66, 104, 65, 153, 45, 15, 176, 84, 187, 22};
uint32_t rotr32(uint32_t x, int n);
uint32_t rotl32(uint32_t x, int n);
uint32_t sigma0(uint32_t x);
uint32_t sigma1(uint32_t x);
uint32_t gamma0(uint32_t x);
uint32_t gamma1(uint32_t x);
uint32_t SubWord(uint32_t x);
int fsb_is_bijective(void);
uint32_t xor_checksum(int lo, int hi);
int ct_memcmp_words(uint32_t a, uint32_t b);
int ct_count_matches(int n, uint32_t val);
uint32_t sched_step(uint32_t w0, uint32_t w2, uint32_t w7, uint32_t w15);
uint32_t msg_sched_expand(uint32_t w0);
uint32_t sha_ch(uint32_t e, uint32_t f, uint32_t g);
uint32_t sha_maj(uint32_t a, uint32_t b, uint32_t c);
uint32_t sha_round(uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t e, uint32_t f, uint32_t g, uint32_t h, uint32_t w, uint32_t k);
int main(void);

uint32_t rotr32(uint32_t x, int n)
{
    return ((x >> n) | (x << (32 - n)));
}

uint32_t rotl32(uint32_t x, int n)
{
    return ((x << n) | (x >> (32 - n)));
}

uint32_t sigma0(uint32_t x)
{
    return ((rotr32(x, 2) ^ rotr32(x, 13)) ^ rotr32(x, 22));
}

uint32_t sigma1(uint32_t x)
{
    return ((rotr32(x, 6) ^ rotr32(x, 11)) ^ rotr32(x, 25));
}

uint32_t gamma0(uint32_t x)
{
    return ((rotr32(x, 7) ^ rotr32(x, 18)) ^ (x >> 3));
}

uint32_t gamma1(uint32_t x)
{
    return ((rotr32(x, 17) ^ rotr32(x, 19)) ^ (x >> 10));
}

uint32_t SubWord(uint32_t x)
{
    return (((((int)FSb[(x & 255)]) | (((int)FSb[((x >> 8) & 255)]) << 8)) | (((int)FSb[((x >> 16) & 255)]) << 16)) | (((int)FSb[((x >> 24) & 255)]) << 24));
}

int fsb_is_bijective(void)
{
    int counts[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int i = 0;
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
    i = 0;
    while ((i < 256)) {
        if ((counts[i] != 1)) {
            return 0;
        }
        i = (i + 1);
    }
    return 1;
}

uint32_t xor_checksum(int lo, int hi)
{
    int i = 0;
    uint32_t acc = 0;
    i = lo;
    while ((i <= hi)) {
        acc = (acc ^ ((int)FSb[i]));
        acc = rotl32(acc, 1);
        i = (i + 1);
    }
    return acc;
}

int ct_memcmp_words(uint32_t a, uint32_t b)
{
    uint32_t diff_c = (a ^ b);
    return ((int)diff_c);
}

int ct_count_matches(int n, uint32_t val)
{
    int i = 0;
    int count = 0;
    uint32_t x = val;
    i = 0;
    while ((i < n)) {
        if (((x ^ val) == 0)) {
            count = (count + 1);
        }
        x = val;
        i = (i + 1);
    }
    return count;
}

uint32_t sched_step(uint32_t w0, uint32_t w2, uint32_t w7, uint32_t w15)
{
    return (((gamma1(w2) + w7) + gamma0(w15)) + w0);
}

uint32_t msg_sched_expand(uint32_t w0)
{
    uint32_t w16 = sched_step(w0, gamma1(w0), gamma0(w0), w0);
    return w16;
}

uint32_t sha_ch(uint32_t e, uint32_t f, uint32_t g)
{
    return ((e & f) ^ ((~e) & g));
}

uint32_t sha_maj(uint32_t a, uint32_t b, uint32_t c)
{
    return (((a & b) ^ (a & c)) ^ (b & c));
}

uint32_t sha_round(uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t e, uint32_t f, uint32_t g, uint32_t h, uint32_t w, uint32_t k)
{
    uint32_t t1 = ((((h + sigma1(e)) + sha_ch(e, f, g)) + k) + w);
    uint32_t t2 = (sigma0(a) + sha_maj(a, b, c));
    return (t1 + t2);
}

int main(void)
{
    uint32_t x = 0;
    printf("%s\n", "=== 1. Bit Rotation ===");
    x = 305419896;
    __auto_type _tern_1 = "FAIL";
    if ((rotr32(x, 4) == 2166572391LL)) {
        _tern_1 = "PASS";
    }
    static char _cast_buf_0[64];
    snprintf(_cast_buf_0, sizeof(_cast_buf_0), "%u", rotr32(x, 4));
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("rotr32(305419896,4)=", _cast_buf_0), " "), _tern_1));
    __auto_type _tern_2 = "FAIL";
    if ((rotl32(x, 4) == 591751041)) {
        _tern_2 = "PASS";
    }
    static char _cast_buf_1[64];
    snprintf(_cast_buf_1, sizeof(_cast_buf_1), "%u", rotl32(x, 4));
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("rotl32(305419896,4)=", _cast_buf_1), " "), _tern_2));
    __auto_type _tern_3 = "FAIL";
    if ((rotr32(rotl32(x, 7), 7) == x)) {
        _tern_3 = "PASS";
    }
    printf("%s\n", __ul_strcat("rotr32(rotl32(x,7),7)==x: ", _tern_3));
    printf("%s\n", "\n=== 2a. SHA-256 sigma functions ===");
    x = 2882400001LL;
    __auto_type _tern_4 = "FAIL";
    if ((sigma0(x) == 625091328)) {
        _tern_4 = "PASS";
    }
    static char _cast_buf_2[64];
    snprintf(_cast_buf_2, sizeof(_cast_buf_2), "%u", sigma0(x));
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("sigma0=", _cast_buf_2), " "), _tern_4));
    __auto_type _tern_5 = "FAIL";
    if ((sigma1(x) == 7196372)) {
        _tern_5 = "PASS";
    }
    static char _cast_buf_3[64];
    snprintf(_cast_buf_3, sizeof(_cast_buf_3), "%u", sigma1(x));
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("sigma1=", _cast_buf_3), " "), _tern_5));
    __auto_type _tern_6 = "FAIL";
    if ((gamma0(x) == 1844333773)) {
        _tern_6 = "PASS";
    }
    static char _cast_buf_4[64];
    snprintf(_cast_buf_4, sizeof(_cast_buf_4), "%u", gamma0(x));
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("gamma0=", _cast_buf_4), " "), _tern_6));
    __auto_type _tern_7 = "FAIL";
    if ((gamma1(x) == 1246368740)) {
        _tern_7 = "PASS";
    }
    static char _cast_buf_5[64];
    snprintf(_cast_buf_5, sizeof(_cast_buf_5), "%u", gamma1(x));
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("gamma1=", _cast_buf_5), " "), _tern_7));
    __auto_type _tern_8 = "FAIL";
    if ((sigma0(x) == sigma0(x))) {
        _tern_8 = "PASS";
    }
    printf("%s\n", __ul_strcat("sigma0 deterministic: ", _tern_8));
    printf("%s\n", "\n=== 2b. AES S-box lookup ===");
    __auto_type _tern_9 = "FAIL";
    if ((FSb[0] == 99)) {
        _tern_9 = "PASS";
    }
    static char _cast_buf_6[64];
    snprintf(_cast_buf_6, sizeof(_cast_buf_6), "%u", FSb[0]);
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("FSb[0]=", _cast_buf_6), " "), _tern_9));
    __auto_type _tern_10 = "FAIL";
    if ((FSb[1] == 124)) {
        _tern_10 = "PASS";
    }
    static char _cast_buf_7[64];
    snprintf(_cast_buf_7, sizeof(_cast_buf_7), "%u", FSb[1]);
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("FSb[1]=", _cast_buf_7), " "), _tern_10));
    __auto_type _tern_11 = "FAIL";
    if ((FSb[255] == 22)) {
        _tern_11 = "PASS";
    }
    static char _cast_buf_8[64];
    snprintf(_cast_buf_8, sizeof(_cast_buf_8), "%u", FSb[255]);
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("FSb[255]=", _cast_buf_8), " "), _tern_11));
    __auto_type _tern_12 = "FAIL";
    if ((SubWord(66051) == 1669101435)) {
        _tern_12 = "PASS";
    }
    static char _cast_buf_9[64];
    snprintf(_cast_buf_9, sizeof(_cast_buf_9), "%u", SubWord(66051));
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("SubWord(66051)=", _cast_buf_9), " "), _tern_12));
    __auto_type _tern_13 = "FAIL";
    if (fsb_is_bijective()) {
        _tern_13 = "PASS";
    }
    printf("%s\n", __ul_strcat("FSb bijective: ", _tern_13));
    printf("%s\n", "\n=== 2c. XOR checksum ===");
    __auto_type _tern_14 = "FAIL";
    if ((xor_checksum(0, 3) == 1274)) {
        _tern_14 = "PASS";
    }
    static char _cast_buf_10[64];
    snprintf(_cast_buf_10, sizeof(_cast_buf_10), "%u", xor_checksum(0, 3));
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("xor_checksum(0,3)=", _cast_buf_10), " "), _tern_14));
    __auto_type _tern_15 = "FAIL";
    if ((xor_checksum(0, 255) == xor_checksum(0, 255))) {
        _tern_15 = "PASS";
    }
    printf("%s\n", __ul_strcat("xor_checksum deterministic: ", _tern_15));
    printf("%s\n", "\n=== 3. Constant-time compare ===");
    __auto_type _tern_16 = "FAIL";
    if ((ct_memcmp_words(57005, 57005) == 0)) {
        _tern_16 = "PASS";
    }
    static char _cast_buf_11[64];
    snprintf(_cast_buf_11, sizeof(_cast_buf_11), "%d", ct_memcmp_words(57005, 57005));
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("ct_memcmp_words(57005,57005)=", _cast_buf_11), " "), _tern_16));
    __auto_type _tern_17 = "FAIL";
    if ((ct_memcmp_words(57005, 48879) != 0)) {
        _tern_17 = "PASS";
    }
    printf("%s\n", __ul_strcat("ct_memcmp_words(57005,48879)!=0: ", _tern_17));
    __auto_type _tern_18 = "FAIL";
    if ((ct_count_matches(8, 51966) == 8)) {
        _tern_18 = "PASS";
    }
    static char _cast_buf_12[64];
    snprintf(_cast_buf_12, sizeof(_cast_buf_12), "%d", ct_count_matches(8, 51966));
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("ct_count_matches(8,51966)=", _cast_buf_12), " "), _tern_18));
    printf("%s\n", "\n=== 4. SHA-256 message schedule ===");
    x = 1633837952;
    __auto_type _tern_19 = "FAIL";
    if ((msg_sched_expand(x) == 3357343958LL)) {
        _tern_19 = "PASS";
    }
    static char _cast_buf_13[64];
    snprintf(_cast_buf_13, sizeof(_cast_buf_13), "%u", msg_sched_expand(x));
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("msg_sched_expand=", _cast_buf_13), " "), _tern_19));
    __auto_type _tern_20 = "FAIL";
    if ((msg_sched_expand(x) == msg_sched_expand(x))) {
        _tern_20 = "PASS";
    }
    printf("%s\n", __ul_strcat("sched deterministic: ", _tern_20));
    printf("%s\n", "\n=== 5. SHA-256 Ch/Maj/round ===");
    uint32_t a = 1779033703;
    uint32_t b = 3144134277LL;
    uint32_t c = 1013904242;
    uint32_t d = 2773480762LL;
    uint32_t e = 1359893119;
    uint32_t f = 2600822924LL;
    uint32_t g = 528734635;
    uint32_t h = 1541325730;
    uint32_t w = 1633837952;
    uint32_t k = 1116352408;
    uint32_t new_a = sha_round(a, b, c, d, e, f, g, h, w, k);
    __auto_type _tern_21 = "FAIL";
    if ((sha_ch(e, f, g) == 528861580)) {
        _tern_21 = "PASS";
    }
    static char _cast_buf_14[64];
    snprintf(_cast_buf_14, sizeof(_cast_buf_14), "%u", sha_ch(e, f, g));
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("sha_ch=", _cast_buf_14), " "), _tern_21));
    __auto_type _tern_22 = "FAIL";
    if ((sha_maj(a, b, c) == 980412007)) {
        _tern_22 = "PASS";
    }
    static char _cast_buf_15[64];
    snprintf(_cast_buf_15, sizeof(_cast_buf_15), "%u", sha_maj(a, b, c));
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("sha_maj=", _cast_buf_15), " "), _tern_22));
    __auto_type _tern_23 = "FAIL";
    if ((new_a == 1567154774)) {
        _tern_23 = "PASS";
    }
    static char _cast_buf_16[64];
    snprintf(_cast_buf_16, sizeof(_cast_buf_16), "%u", new_a);
    printf("%s\n", __ul_strcat(__ul_strcat(__ul_strcat("sha_round=", _cast_buf_16), " "), _tern_23));
    __auto_type _tern_24 = "FAIL";
    if ((sha_round(a, b, c, d, e, f, g, h, w, k) == new_a)) {
        _tern_24 = "PASS";
    }
    printf("%s\n", __ul_strcat("sha_round deterministic: ", _tern_24));
    return 0;
}


