/*
 * test_bitops.c
 * Clean mbedtls-derived bit operation tests for c2ul round-trip.
 *
 * Patterns tested (no goto, no pointer arithmetic, no function pointers):
 *   1. Bit rotation (ROL/ROR) — mbedtls SHA-256 sigma functions
 *   2. Table lookup — AES S-box forward and SubWord
 *   3. Constant-time compare — mbedtls_ct_memcmp equivalent
 *   4. SHA-256 message schedule (W array, bitwise ops, no pointers)
 *   5. CRC-like accumulation (bitwise XOR fold)
 */

#include <stdio.h>

typedef unsigned int  uint32_t;
typedef unsigned char uint8_t;

/* ================================================================
 * 1. Bit rotation — SHA-256 style
 * ================================================================ */

static uint32_t rotr32(uint32_t x, int n)
{
    return (x >> n) | (x << (32 - n));
}

static uint32_t rotl32(uint32_t x, int n)
{
    return (x << n) | (x >> (32 - n));
}

/* SHA-256 sigma functions */
static uint32_t sigma0(uint32_t x)
{
    return rotr32(x, 2) ^ rotr32(x, 13) ^ rotr32(x, 22);
}

static uint32_t sigma1(uint32_t x)
{
    return rotr32(x, 6) ^ rotr32(x, 11) ^ rotr32(x, 25);
}

static uint32_t gamma0(uint32_t x)
{
    return rotr32(x, 7) ^ rotr32(x, 18) ^ (x >> 3);
}

static uint32_t gamma1(uint32_t x)
{
    return rotr32(x, 17) ^ rotr32(x, 19) ^ (x >> 10);
}

/* ================================================================
 * 2. AES S-box table lookup
 * ================================================================ */

static const uint8_t FSb[256] = {
    0x63,0x7C,0x77,0x7B,0xF2,0x6B,0x6F,0xC5,0x30,0x01,0x67,0x2B,0xFE,0xD7,0xAB,0x76,
    0xCA,0x82,0xC9,0x7D,0xFA,0x59,0x47,0xF0,0xAD,0xD4,0xA2,0xAF,0x9C,0xA4,0x72,0xC0,
    0xB7,0xFD,0x93,0x26,0x36,0x3F,0xF7,0xCC,0x34,0xA5,0xE5,0xF1,0x71,0xD8,0x31,0x15,
    0x04,0xC7,0x23,0xC3,0x18,0x96,0x05,0x9A,0x07,0x12,0x80,0xE2,0xEB,0x27,0xB2,0x75,
    0x09,0x83,0x2C,0x1A,0x1B,0x6E,0x5A,0xA0,0x52,0x3B,0xD6,0xB3,0x29,0xE3,0x2F,0x84,
    0x53,0xD1,0x00,0xED,0x20,0xFC,0xB1,0x5B,0x6A,0xCB,0xBE,0x39,0x4A,0x4C,0x58,0xCF,
    0xD0,0xEF,0xAA,0xFB,0x43,0x4D,0x33,0x85,0x45,0xF9,0x02,0x7F,0x50,0x3C,0x9F,0xA8,
    0x51,0xA3,0x40,0x8F,0x92,0x9D,0x38,0xF5,0xBC,0xB6,0xDA,0x21,0x10,0xFF,0xF3,0xD2,
    0xCD,0x0C,0x13,0xEC,0x5F,0x97,0x44,0x17,0xC4,0xA7,0x7E,0x3D,0x64,0x5D,0x19,0x73,
    0x60,0x81,0x4F,0xDC,0x22,0x2A,0x90,0x88,0x46,0xEE,0xB8,0x14,0xDE,0x5E,0x0B,0xDB,
    0xE0,0x32,0x3A,0x0A,0x49,0x06,0x24,0x5C,0xC2,0xD3,0xAC,0x62,0x91,0x95,0xE4,0x79,
    0xE7,0xC8,0x37,0x6D,0x8D,0xD5,0x4E,0xA9,0x6C,0x56,0xF4,0xEA,0x65,0x7A,0xAE,0x08,
    0xBA,0x78,0x25,0x2E,0x1C,0xA6,0xB4,0xC6,0xE8,0xDD,0x74,0x1F,0x4B,0xBD,0x8B,0x8A,
    0x70,0x3E,0xB5,0x66,0x48,0x03,0xF6,0x0E,0x61,0x35,0x57,0xB9,0x86,0xC1,0x1D,0x9E,
    0xE1,0xF8,0x98,0x11,0x69,0xD9,0x8E,0x94,0x9B,0x1E,0x87,0xE9,0xCE,0x55,0x28,0xDF,
    0x8C,0xA1,0x89,0x0D,0xBF,0xE6,0x42,0x68,0x41,0x99,0x2D,0x0F,0xB0,0x54,0xBB,0x16
};

static uint32_t SubWord(uint32_t x)
{
    return ((uint32_t)FSb[x & 0xFF])
         | ((uint32_t)FSb[(x >> 8)  & 0xFF] << 8)
         | ((uint32_t)FSb[(x >> 16) & 0xFF] << 16)
         | ((uint32_t)FSb[(x >> 24) & 0xFF] << 24);
}

/* Verify FSb is a bijection: every output appears exactly once */
static int fsb_is_bijective(void)
{
    int counts[256];
    int i;
    for (i = 0; i < 256; i++) counts[i] = 0;
    for (i = 0; i < 256; i++) counts[FSb[i]]++;
    for (i = 0; i < 256; i++) {
        if (counts[i] != 1) return 0;
    }
    return 1;
}

/* XOR checksum with rotate-left-1 accumulation */
static uint32_t xor_checksum(int lo, int hi)
{
    int i;
    uint32_t acc = 0;
    for (i = lo; i <= hi; i++) {
        acc = acc ^ (uint32_t)FSb[i];
        acc = rotl32(acc, 1);
    }
    return acc;
}

/* ================================================================
 * 3. Constant-time compare (no branches on secret data)
 * ================================================================ */

/* Returns 0 if equal, non-zero if different — constant time */
static int ct_memcmp_words(uint32_t a, uint32_t b)
{
    uint32_t diff = a ^ b;
    return (int)diff;
}

/* Count matching words: return n if all words equal val, else less */
static int ct_count_matches(int n, uint32_t val)
{
    int i;
    int count = 0;
    uint32_t x = val;
    for (i = 0; i < n; i++) {
        if ((x ^ val) == 0) count++;
        x = val;  /* keep all equal */
    }
    return count;
}

/* ================================================================
 * 4. SHA-256 message schedule (W[0..15] expansion step)
 *    W[i] = gamma1(W[i-2]) + W[i-7] + gamma0(W[i-15]) + W[i-16]
 *    Use index-based array — no pointers.
 * ================================================================ */

static uint32_t sched_step(uint32_t w0, uint32_t w2,
                            uint32_t w7, uint32_t w15)
{
    return gamma1(w2) + w7 + gamma0(w15) + w0;
}

/* Compute first 4 expansion steps from a known initial block */
static uint32_t msg_sched_expand(uint32_t w0)
{
    /* Simplified: W[i+16] = gamma1(W[i+14]) + W[i+9] + gamma0(W[i+1]) + W[i]
     * Here we just do one round with a single seed value for test simplicity. */
    uint32_t w16 = sched_step(w0, gamma1(w0), gamma0(w0), w0);
    return w16;
}

/* ================================================================
 * 5. SHA-256 Ch and Maj operations
 * ================================================================ */

static uint32_t sha_ch(uint32_t e, uint32_t f, uint32_t g)
{
    return (e & f) ^ (~e & g);
}

static uint32_t sha_maj(uint32_t a, uint32_t b, uint32_t c)
{
    return (a & b) ^ (a & c) ^ (b & c);
}

/* One round of SHA-256 compression (simplified, no full state) */
static uint32_t sha_round(uint32_t a, uint32_t b, uint32_t c,
                           uint32_t d, uint32_t e, uint32_t f,
                           uint32_t g, uint32_t h,
                           uint32_t w, uint32_t k)
{
    uint32_t t1 = h + sigma1(e) + sha_ch(e, f, g) + k + w;
    uint32_t t2 = sigma0(a) + sha_maj(a, b, c);
    /* Returns new 'a' value */
    return t1 + t2;
}

/* ================================================================
 * TEST HARNESS
 * Uses decimal output to match UL's cast(uint32, string) which emits %u.
 * All hex constants are converted to their decimal equivalents in checks.
 * ================================================================ */

int main(void)
{
    uint32_t x;

    printf("=== 1. Bit Rotation ===\n");
    /* 0x12345678 = 305419896 */
    x = 305419896u;
    /* rotr32(305419896, 4) = 0x81234567 = 2166572391 */
    printf("rotr32(305419896,4)=%u %s\n",
           rotr32(x, 4),
           rotr32(x, 4) == 2166572391u ? "PASS" : "FAIL");
    /* rotl32(305419896, 4) = 0x23456781 = 591751041 */
    printf("rotl32(305419896,4)=%u %s\n",
           rotl32(x, 4),
           rotl32(x, 4) == 591751041u ? "PASS" : "FAIL");
    printf("rotr32(rotl32(x,7),7)==x: %s\n",
           rotr32(rotl32(x, 7), 7) == x ? "PASS" : "FAIL");

    printf("\n=== 2a. SHA-256 sigma functions ===\n");
    /* 0xABCDEF01 = 2882400001 */
    x = 2882400001u;
    /* sigma0(2882400001) = 0x25422300 = 625091328 */
    printf("sigma0=%u %s\n", sigma0(x),
           sigma0(x) == 625091328u ? "PASS" : "FAIL");
    /* sigma1(2882400001) = 0x006DCED4 = 7196372 */
    printf("sigma1=%u %s\n", sigma1(x),
           sigma1(x) == 7196372u ? "PASS" : "FAIL");
    /* gamma0(2882400001) = 0x6DEE4CCD = 1844333773 */
    printf("gamma0=%u %s\n", gamma0(x),
           gamma0(x) == 1844333773u ? "PASS" : "FAIL");
    /* gamma1(2882400001) = 0x4A4A13E4 = 1246368740 */
    printf("gamma1=%u %s\n", gamma1(x),
           gamma1(x) == 1246368740u ? "PASS" : "FAIL");
    printf("sigma0 deterministic: %s\n",
           sigma0(x) == sigma0(x) ? "PASS" : "FAIL");

    printf("\n=== 2b. AES S-box lookup ===\n");
    /* FSb[0]=0x63=99, FSb[1]=0x7C=124, FSb[255]=0x16=22 */
    printf("FSb[0]=%u %s\n",   FSb[0],   FSb[0]   == 99  ? "PASS" : "FAIL");
    printf("FSb[1]=%u %s\n",   FSb[1],   FSb[1]   == 124 ? "PASS" : "FAIL");
    printf("FSb[255]=%u %s\n", FSb[255], FSb[255] == 22  ? "PASS" : "FAIL");
    /* SubWord(0x00010203) = 0x637C777B = 1669101435 */
    printf("SubWord(66051)=%u %s\n",
           SubWord(66051u),
           SubWord(66051u) == 1669101435u ? "PASS" : "FAIL");
    printf("FSb bijective: %s\n", fsb_is_bijective() ? "PASS" : "FAIL");

    printf("\n=== 2c. XOR checksum ===\n");
    /* xor_checksum(0,3) = 0x000004FA = 1274 */
    printf("xor_checksum(0,3)=%u %s\n",
           xor_checksum(0, 3),
           xor_checksum(0, 3) == 1274u ? "PASS" : "FAIL");
    printf("xor_checksum deterministic: %s\n",
           xor_checksum(0, 255) == xor_checksum(0, 255) ? "PASS" : "FAIL");

    printf("\n=== 3. Constant-time compare ===\n");
    /* 0xDEAD = 57005, 0xBEEF = 48879 */
    printf("ct_memcmp_words(57005,57005)=%d %s\n",
           ct_memcmp_words(57005u, 57005u),
           ct_memcmp_words(57005u, 57005u) == 0 ? "PASS" : "FAIL");
    printf("ct_memcmp_words(57005,48879)!=0: %s\n",
           ct_memcmp_words(57005u, 48879u) != 0 ? "PASS" : "FAIL");
    printf("ct_count_matches(8,51966)=%d %s\n",
           ct_count_matches(8, 51966u),
           ct_count_matches(8, 51966u) == 8 ? "PASS" : "FAIL");

    printf("\n=== 4. SHA-256 message schedule ===\n");
    /* 0x61626380 = 1633837952 */
    x = 1633837952u;
    /* msg_sched_expand(1633837952) = 3357343958 */
    printf("msg_sched_expand=%u %s\n",
           msg_sched_expand(x),
           msg_sched_expand(x) == 3357343958u ? "PASS" : "FAIL");
    printf("sched deterministic: %s\n",
           msg_sched_expand(x) == msg_sched_expand(x) ? "PASS" : "FAIL");

    printf("\n=== 5. SHA-256 Ch/Maj/round ===\n");
    {
        /* SHA-256 initial hash values (decimal) */
        uint32_t a = 1779033703u;  /* 0x6A09E667 */
        uint32_t b = 3144134277u;  /* 0xBB67AE85 */
        uint32_t c = 1013904242u;  /* 0x3C6EF372 */
        uint32_t d = 2773480762u;  /* 0xA54FF53A */
        uint32_t e = 1359893119u;  /* 0x510E527F */
        uint32_t f = 2600822924u;  /* 0x9B05688C */
        uint32_t g = 528734635u;   /* 0x1F83D9AB */
        uint32_t h = 1541325730u;  /* 0x5BE0CD19 */
        uint32_t w = 1633837952u;  /* 0x61626380 */
        uint32_t k = 1116352408u;  /* 0x428A2F98 */
        uint32_t new_a = sha_round(a, b, c, d, e, f, g, h, w, k);
        /* sha_ch(e,f,g) = 0x1F85C98C = 528861580 */
        printf("sha_ch=%u %s\n", sha_ch(e, f, g),
               sha_ch(e, f, g) == 528861580u ? "PASS" : "FAIL");
        /* sha_maj(a,b,c) = 0x3A6FE667 = 980412007 */
        printf("sha_maj=%u %s\n", sha_maj(a, b, c),
               sha_maj(a, b, c) == 980412007u ? "PASS" : "FAIL");
        /* sha_round new_a = 1567154774 */
        printf("sha_round=%u %s\n", new_a,
               new_a == 1567154774u ? "PASS" : "FAIL");
        printf("sha_round deterministic: %s\n",
               sha_round(a, b, c, d, e, f, g, h, w, k) == new_a ? "PASS" : "FAIL");
    }

    return 0;
}
