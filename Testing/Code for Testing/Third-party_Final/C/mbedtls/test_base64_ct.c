/*
 * test_base64_ct.c
 * Extracted from mbedtls/tf-psa-crypto/utilities/base64.c
 *
 * Stress: constant-time masking, bitwise OR accumulation, signed return codes,
 *         const correctness (const unsigned char * param), security patterns.
 *
 * Patterns tested:
 *   - mbedtls_ct_uchar_in_range_if: bitwise mask via -(cond) trick
 *   - mbedtls_ct_base64_enc_char:   OR accumulation, unsigned char arithmetic
 *   - mbedtls_ct_base64_dec_value:  signed char return, +1/-1 sentinel
 */

#include <stdio.h>
#include <string.h>

/* ---- Stripped-down constant-time primitives (from constant_time_impl.h) ---- */

/* Returns mask=0xFF if lo <= c <= hi, else 0x00.
 * Uses -(unsigned)(c-lo <= hi-lo) bitmask trick — no branches. */
static unsigned char mbedtls_ct_uchar_in_range_if(unsigned char lo,
                                                   unsigned char hi,
                                                   unsigned char c,
                                                   unsigned char val)
{
    unsigned char in_range = (unsigned char)(1u + hi - lo) > (unsigned char)(c - lo) ? 0xFF : 0x00;
    return (unsigned char)(in_range & val);
}

/* ---- Functions extracted from base64.c ---- */

/*
 * Map a 6-bit value (0..63) to its base64 character.
 * Uses no branches — constant-time for security.
 */
static unsigned char mbedtls_ct_base64_enc_char(unsigned char value)
{
    unsigned char digit = 0;
    digit |= mbedtls_ct_uchar_in_range_if(0,  25, value, (unsigned char)('A' + value));
    digit |= mbedtls_ct_uchar_in_range_if(26, 51, value, (unsigned char)('a' + value - 26));
    digit |= mbedtls_ct_uchar_in_range_if(52, 61, value, (unsigned char)('0' + value - 52));
    digit |= mbedtls_ct_uchar_in_range_if(62, 62, value, '+');
    digit |= mbedtls_ct_uchar_in_range_if(63, 63, value, '/');
    return digit;
}

/*
 * Map a base64 character to its 6-bit value, or -1 if invalid.
 * Returns val+1 internally (0 = not in range), then subtracts 1 at end.
 */
static signed char mbedtls_ct_base64_dec_value(unsigned char c)
{
    unsigned char val = 0;
    val |= mbedtls_ct_uchar_in_range_if('A', 'Z', c, (unsigned char)(c - 'A' +  0 + 1));
    val |= mbedtls_ct_uchar_in_range_if('a', 'z', c, (unsigned char)(c - 'a' + 26 + 1));
    val |= mbedtls_ct_uchar_in_range_if('0', '9', c, (unsigned char)(c - '0' + 52 + 1));
    val |= mbedtls_ct_uchar_in_range_if('+', '+', c, (unsigned char)(c - '+' + 62 + 1));
    val |= mbedtls_ct_uchar_in_range_if('/', '/', c, (unsigned char)(c - '/' + 63 + 1));
    return (signed char)(val - 1);
}

/*
 * Error codes (from mbedtls/private/error_common.h pattern)
 */
#define MBEDTLS_ERR_BASE64_BUFFER_TOO_SMALL    -0x0028
#define MBEDTLS_ERR_BASE64_INVALID_CHARACTER   -0x002A

/*
 * Encode src[0..slen-1] to base64 into dst.
 * Returns 0 on success, MBEDTLS_ERR_BASE64_BUFFER_TOO_SMALL if dst too small.
 * *olen receives the number of bytes written (including NUL terminator).
 *
 * Patterns: pointer arithmetic (*p++), error code propagation,
 *           conditional return paths, struct field access via pointer.
 */
static int mbedtls_base64_encode(unsigned char *dst, int dlen, int *olen,
                                 const unsigned char *src, int slen)
{
    int i, n;
    int C1, C2, C3;
    unsigned char *p;

    if (slen == 0) {
        *olen = 0;
        return 0;
    }

    /* Number of 4-char groups needed */
    n = (slen / 3) + (slen % 3 != 0);

    /* Check for overflow */
    if (n > (0x7FFFFFFF - 1) / 4) {
        *olen = 0x7FFFFFFF;
        return MBEDTLS_ERR_BASE64_BUFFER_TOO_SMALL;
    }

    n *= 4;

    if (dlen < n + 1 || dst == 0) {
        *olen = n + 1;
        return MBEDTLS_ERR_BASE64_BUFFER_TOO_SMALL;
    }

    n = (slen / 3) * 3;

    for (i = 0, p = dst; i < n; i += 3) {
        C1 = *src++;
        C2 = *src++;
        C3 = *src++;

        *p++ = mbedtls_ct_base64_enc_char((unsigned char)((C1 >> 2) & 0x3F));
        *p++ = mbedtls_ct_base64_enc_char((unsigned char)((((C1 & 3) << 4) + (C2 >> 4)) & 0x3F));
        *p++ = mbedtls_ct_base64_enc_char((unsigned char)((((C2 & 15) << 2) + (C3 >> 6)) & 0x3F));
        *p++ = mbedtls_ct_base64_enc_char((unsigned char)(C3 & 0x3F));
    }

    if (i < slen) {
        C1 = *src++;
        C2 = ((i + 1) < slen) ? (int)*src++ : 0;

        *p++ = mbedtls_ct_base64_enc_char((unsigned char)((C1 >> 2) & 0x3F));
        *p++ = mbedtls_ct_base64_enc_char((unsigned char)((((C1 & 3) << 4) + (C2 >> 4)) & 0x3F));

        if ((i + 1) < slen) {
            *p++ = mbedtls_ct_base64_enc_char((unsigned char)(((C2 & 15) << 2) & 0x3F));
        } else {
            *p++ = '=';
        }
        *p++ = '=';
    }

    *olen = (int)(p - dst);
    *p = 0;

    return 0;
}

/* ---- Test harness ---- */

int main(void)
{
    int i, ret;
    signed char dec;

    /* Test 1: enc_char — spot check all 64 valid values */
    printf("=== mbedtls_ct_base64_enc_char ===\n");
    static const char *alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    int pass = 1;
    for (i = 0; i < 64; i++) {
        unsigned char got = mbedtls_ct_base64_enc_char((unsigned char)i);
        if (got != (unsigned char)alphabet[i]) {
            printf("FAIL enc_char(%d): got %c expected %c\n", i, got, alphabet[i]);
            pass = 0;
        }
    }
    printf("enc_char 0..63: %s\n", pass ? "PASS" : "FAIL");

    /* Test 2: dec_value — round-trip for all 64 */
    printf("\n=== mbedtls_ct_base64_dec_value ===\n");
    pass = 1;
    for (i = 0; i < 64; i++) {
        unsigned char ch = (unsigned char)alphabet[i];
        dec = mbedtls_ct_base64_dec_value(ch);
        if ((int)dec != i) {
            printf("FAIL dec_value('%c'=%d): got %d expected %d\n", ch, ch, dec, i);
            pass = 0;
        }
    }
    printf("dec_value round-trip: %s\n", pass ? "PASS" : "FAIL");

    /* Test 3: dec_value invalid chars */
    printf("\n=== dec_value invalid chars ===\n");
    static const unsigned char invalid[] = { '!', '@', '#', ' ', '\n', 0x00, 0x80, 0xFF };
    pass = 1;
    for (i = 0; i < (int)sizeof(invalid); i++) {
        dec = mbedtls_ct_base64_dec_value(invalid[i]);
        if (dec != -1) {
            printf("FAIL dec_value(0x%02X): got %d expected -1\n", invalid[i], dec);
            pass = 0;
        }
    }
    printf("dec_value invalid: %s\n", pass ? "PASS" : "FAIL");

    /* Test 4: encode — standard test vector from RFC 4648 */
    printf("\n=== mbedtls_base64_encode ===\n");
    {
        static const unsigned char src[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05 };
        unsigned char dst[32];
        int olen = 0;
        ret = mbedtls_base64_encode(dst, sizeof(dst), &olen, src, 6);
        printf("encode ret=%d olen=%d out=%s\n", ret, olen, dst);
        /* Expected: AAECAwQF */
        printf("encode 6 bytes: %s\n",
               (ret == 0 && olen == 8 && strcmp((char*)dst, "AAECAwQF") == 0) ? "PASS" : "FAIL");
    }

    /* Test 5: encode empty */
    {
        unsigned char dst[8];
        int olen = 0;
        ret = mbedtls_base64_encode(dst, sizeof(dst), &olen, (const unsigned char*)"", 0);
        printf("encode empty: ret=%d olen=%d %s\n", ret, olen,
               (ret == 0 && olen == 0) ? "PASS" : "FAIL");
    }

    /* Test 6: encode buffer too small */
    {
        unsigned char dst[4];
        int olen = 0;
        ret = mbedtls_base64_encode(dst, 4, &olen, (const unsigned char*)"Hello", 5);
        printf("encode too-small: ret=%d olen=%d %s\n", ret, olen,
               (ret == MBEDTLS_ERR_BASE64_BUFFER_TOO_SMALL) ? "PASS" : "FAIL");
    }

    /* Test 7: encode padded (1-byte remaining) */
    {
        unsigned char dst[32];
        int olen = 0;
        ret = mbedtls_base64_encode(dst, sizeof(dst), &olen,
                                    (const unsigned char*)"Man", 3);
        printf("encode 'Man': ret=%d olen=%d out=%s %s\n", ret, olen, dst,
               (ret == 0 && strcmp((char*)dst, "TWFu") == 0) ? "PASS" : "FAIL");
    }

    return 0;
}
