/*
 * test_md5_ctx.c
 * Extracted from mbedtls/tf-psa-crypto/drivers/builtin/src/md5.c
 *
 * Stress: struct nesting (context struct with array fields), error code propagation
 *         (int ret = ERR; if (ret != 0) goto exit;), const array params,
 *         function pointer table (mbedtls_md_info_t vtable pattern).
 *
 * Patterns tested:
 *   - mbedtls_md5_context: nested struct with uint32_t arrays
 *   - mbedtls_md5_starts: init via struct field assignment
 *   - mbedtls_internal_md5_process: heavy bitwise ops on struct fields
 *   - mbedtls_md5_update: fill/copy logic, error propagation with goto
 *   - mbedtls_md5_finish: padding, multi-step struct mutation
 *   - mbedtls_md_info_t: function pointer table (vtable) pattern
 */

#include <stdio.h>
#include <string.h>

/* ---- Types (from mbedtls/private/md5.h pattern) ---- */

typedef unsigned int  uint32_t;
typedef unsigned char uint8_t;

/*
 * MD5 context struct — nested array fields.
 * c2ul stress: struct with fixed-size array members.
 */
typedef struct mbedtls_md5_context {
    uint32_t total[2];    /* number of bytes processed */
    uint32_t state[4];    /* intermediate digest state */
    uint8_t  buffer[64];  /* data block being processed */
} mbedtls_md5_context;

/* Error code pattern from mbedtls */
#define MBEDTLS_ERR_ERROR_CORRUPTION_DETECTED  -0x006E

/* ---- Byte-access macros (from mbedtls/private/platform_util.h) ---- */

#define MBEDTLS_GET_UINT32_LE(data, offset) \
    (((uint32_t)(data)[(offset)])           \
   | ((uint32_t)(data)[(offset) + 1] <<  8) \
   | ((uint32_t)(data)[(offset) + 2] << 16) \
   | ((uint32_t)(data)[(offset) + 3] << 24))

#define MBEDTLS_PUT_UINT32_LE(n, b, i)       \
    do {                                      \
        (b)[(i)    ] = (uint8_t)((n)      );  \
        (b)[(i) + 1] = (uint8_t)((n) >>  8); \
        (b)[(i) + 2] = (uint8_t)((n) >> 16); \
        (b)[(i) + 3] = (uint8_t)((n) >> 24); \
    } while (0)

/* ---- MD5 implementation (extracted, no mbedtls build system) ---- */

static void mbedtls_md5_init(mbedtls_md5_context *ctx)
{
    memset(ctx, 0, sizeof(mbedtls_md5_context));
}

static void mbedtls_md5_free(mbedtls_md5_context *ctx)
{
    if (ctx == 0) {
        return;
    }
    memset(ctx, 0, sizeof(mbedtls_md5_context));
}

static void mbedtls_md5_clone(mbedtls_md5_context *dst,
                               const mbedtls_md5_context *src)
{
    *dst = *src;
}

static int mbedtls_md5_starts(mbedtls_md5_context *ctx)
{
    ctx->total[0] = 0;
    ctx->total[1] = 0;

    ctx->state[0] = 0x67452301;
    ctx->state[1] = 0xEFCDAB89;
    ctx->state[2] = 0x98BADCFE;
    ctx->state[3] = 0x10325476;

    return 0;
}

#define S(x, n) (((x) << (n)) | (((x) & 0xFFFFFFFF) >> (32 - (n))))

#define P(a, b, c, d, k, s, t)                         \
    do {                                                \
        (a) += F((b), (c), (d)) + local_X[(k)] + (t);  \
        (a) = S((a), (s)) + (b);                        \
    } while (0)

static int mbedtls_internal_md5_process(mbedtls_md5_context *ctx,
                                        const uint8_t data[64])
{
    uint32_t local_X[16];
    uint32_t A, B, C, D;

    local_X[ 0] = MBEDTLS_GET_UINT32_LE(data,  0);
    local_X[ 1] = MBEDTLS_GET_UINT32_LE(data,  4);
    local_X[ 2] = MBEDTLS_GET_UINT32_LE(data,  8);
    local_X[ 3] = MBEDTLS_GET_UINT32_LE(data, 12);
    local_X[ 4] = MBEDTLS_GET_UINT32_LE(data, 16);
    local_X[ 5] = MBEDTLS_GET_UINT32_LE(data, 20);
    local_X[ 6] = MBEDTLS_GET_UINT32_LE(data, 24);
    local_X[ 7] = MBEDTLS_GET_UINT32_LE(data, 28);
    local_X[ 8] = MBEDTLS_GET_UINT32_LE(data, 32);
    local_X[ 9] = MBEDTLS_GET_UINT32_LE(data, 36);
    local_X[10] = MBEDTLS_GET_UINT32_LE(data, 40);
    local_X[11] = MBEDTLS_GET_UINT32_LE(data, 44);
    local_X[12] = MBEDTLS_GET_UINT32_LE(data, 48);
    local_X[13] = MBEDTLS_GET_UINT32_LE(data, 52);
    local_X[14] = MBEDTLS_GET_UINT32_LE(data, 56);
    local_X[15] = MBEDTLS_GET_UINT32_LE(data, 60);

    A = ctx->state[0];
    B = ctx->state[1];
    C = ctx->state[2];
    D = ctx->state[3];

#undef F
#define F(x, y, z) ((z) ^ ((x) & ((y) ^ (z))))
    P(A, B, C, D,  0,  7, 0xD76AA478);
    P(D, A, B, C,  1, 12, 0xE8C7B756);
    P(C, D, A, B,  2, 17, 0x242070DB);
    P(B, C, D, A,  3, 22, 0xC1BDCEEE);
    P(A, B, C, D,  4,  7, 0xF57C0FAF);
    P(D, A, B, C,  5, 12, 0x4787C62A);
    P(C, D, A, B,  6, 17, 0xA8304613);
    P(B, C, D, A,  7, 22, 0xFD469501);
    P(A, B, C, D,  8,  7, 0x698098D8);
    P(D, A, B, C,  9, 12, 0x8B44F7AF);
    P(C, D, A, B, 10, 17, 0xFFFF5BB1);
    P(B, C, D, A, 11, 22, 0x895CD7BE);
    P(A, B, C, D, 12,  7, 0x6B901122);
    P(D, A, B, C, 13, 12, 0xFD987193);
    P(C, D, A, B, 14, 17, 0xA679438E);
    P(B, C, D, A, 15, 22, 0x49B40821);
#undef F

#define F(x, y, z) ((y) ^ ((z) & ((x) ^ (y))))
    P(A, B, C, D,  1,  5, 0xF61E2562);
    P(D, A, B, C,  6,  9, 0xC040B340);
    P(C, D, A, B, 11, 14, 0x265E5A51);
    P(B, C, D, A,  0, 20, 0xE9B6C7AA);
    P(A, B, C, D,  5,  5, 0xD62F105D);
    P(D, A, B, C, 10,  9, 0x02441453);
    P(C, D, A, B, 15, 14, 0xD8A1E681);
    P(B, C, D, A,  4, 20, 0xE7D3FBC8);
    P(A, B, C, D,  9,  5, 0x21E1CDE6);
    P(D, A, B, C, 14,  9, 0xC33707D6);
    P(C, D, A, B,  3, 14, 0xF4D50D87);
    P(B, C, D, A,  8, 20, 0x455A14ED);
    P(A, B, C, D, 13,  5, 0xA9E3E905);
    P(D, A, B, C,  2,  9, 0xFCEFA3F8);
    P(C, D, A, B,  7, 14, 0x676F02D9);
    P(B, C, D, A, 12, 20, 0x8D2A4C8A);
#undef F

#define F(x, y, z) ((x) ^ (y) ^ (z))
    P(A, B, C, D,  5,  4, 0xFFFA3942);
    P(D, A, B, C,  8, 11, 0x8771F681);
    P(C, D, A, B, 11, 16, 0x6D9D6122);
    P(B, C, D, A, 14, 23, 0xFDE5380C);
    P(A, B, C, D,  1,  4, 0xA4BEEA44);
    P(D, A, B, C,  4, 11, 0x4BDECFA9);
    P(C, D, A, B,  7, 16, 0xF6BB4B60);
    P(B, C, D, A, 10, 23, 0xBEBFBC70);
    P(A, B, C, D, 13,  4, 0x289B7EC6);
    P(D, A, B, C,  0, 11, 0xEAA127FA);
    P(C, D, A, B,  3, 16, 0xD4EF3085);
    P(B, C, D, A,  6, 23, 0x04881D05);
    P(A, B, C, D,  9,  4, 0xD9D4D039);
    P(D, A, B, C, 12, 11, 0xE6DB99E5);
    P(C, D, A, B, 15, 16, 0x1FA27CF8);
    P(B, C, D, A,  2, 23, 0xC4AC5665);
#undef F

#define F(x, y, z) ((y) ^ ((x) | ~(z)))
    P(A, B, C, D,  0,  6, 0xF4292244);
    P(D, A, B, C,  7, 10, 0x432AFF97);
    P(C, D, A, B, 14, 15, 0xAB9423A7);
    P(B, C, D, A,  5, 21, 0xFC93A039);
    P(A, B, C, D, 12,  6, 0x655B59C3);
    P(D, A, B, C,  3, 10, 0x8F0CCC92);
    P(C, D, A, B, 10, 15, 0xFFEFF47D);
    P(B, C, D, A,  1, 21, 0x85845DD1);
    P(A, B, C, D,  8,  6, 0x6FA87E4F);
    P(D, A, B, C, 15, 10, 0xFE2CE6E0);
    P(C, D, A, B,  6, 15, 0xA3014314);
    P(B, C, D, A, 13, 21, 0x4E0811A1);
    P(A, B, C, D,  4,  6, 0xF7537E82);
    P(D, A, B, C, 11, 10, 0xBD3AF235);
    P(C, D, A, B,  2, 15, 0x2AD7D2BB);
    P(B, C, D, A,  9, 21, 0xEB86D391);
#undef F

    ctx->state[0] += A;
    ctx->state[1] += B;
    ctx->state[2] += C;
    ctx->state[3] += D;

    memset(local_X, 0, sizeof(local_X));

    return 0;
}

static int mbedtls_md5_update(mbedtls_md5_context *ctx,
                               const uint8_t *input,
                               int ilen)
{
    int ret = MBEDTLS_ERR_ERROR_CORRUPTION_DETECTED;
    int fill;
    uint32_t left;

    if (ilen == 0) {
        return 0;
    }

    left = ctx->total[0] & 0x3F;
    fill = (int)(64 - left);

    ctx->total[0] += (uint32_t)ilen;
    ctx->total[0] &= 0xFFFFFFFF;

    if (ctx->total[0] < (uint32_t)ilen) {
        ctx->total[1]++;
    }

    if (left != 0 && ilen >= fill) {
        memcpy((void *)(ctx->buffer + left), input, (int)fill);
        if ((ret = mbedtls_internal_md5_process(ctx, ctx->buffer)) != 0) {
            return ret;
        }
        input += fill;
        ilen  -= fill;
        left = 0;
    }

    while (ilen >= 64) {
        if ((ret = mbedtls_internal_md5_process(ctx, input)) != 0) {
            return ret;
        }
        input += 64;
        ilen  -= 64;
    }

    if (ilen > 0) {
        memcpy((void *)(ctx->buffer + left), input, ilen);
    }

    return 0;
}

static int mbedtls_md5_finish(mbedtls_md5_context *ctx,
                               uint8_t output[16])
{
    int ret = MBEDTLS_ERR_ERROR_CORRUPTION_DETECTED;
    uint32_t used;
    uint32_t high, low;

    used = ctx->total[0] & 0x3F;

    ctx->buffer[used++] = 0x80;

    if (used <= 56) {
        memset(ctx->buffer + used, 0, (int)(56 - used));
    } else {
        memset(ctx->buffer + used, 0, (int)(64 - used));

        if ((ret = mbedtls_internal_md5_process(ctx, ctx->buffer)) != 0) {
            goto exit;
        }

        memset(ctx->buffer, 0, 56);
    }

    high = (ctx->total[0] >> 29) | (ctx->total[1] << 3);
    low  =  ctx->total[0] << 3;

    MBEDTLS_PUT_UINT32_LE(low,  ctx->buffer, 56);
    MBEDTLS_PUT_UINT32_LE(high, ctx->buffer, 60);

    if ((ret = mbedtls_internal_md5_process(ctx, ctx->buffer)) != 0) {
        goto exit;
    }

    MBEDTLS_PUT_UINT32_LE(ctx->state[0], output,  0);
    MBEDTLS_PUT_UINT32_LE(ctx->state[1], output,  4);
    MBEDTLS_PUT_UINT32_LE(ctx->state[2], output,  8);
    MBEDTLS_PUT_UINT32_LE(ctx->state[3], output, 12);

    ret = 0;

exit:
    mbedtls_md5_free(ctx);
    return ret;
}

static int mbedtls_md5(const uint8_t *input, int ilen, uint8_t output[16])
{
    int ret = MBEDTLS_ERR_ERROR_CORRUPTION_DETECTED;
    mbedtls_md5_context ctx;

    mbedtls_md5_init(&ctx);

    if ((ret = mbedtls_md5_starts(&ctx)) != 0) {
        goto exit;
    }

    if ((ret = mbedtls_md5_update(&ctx, input, ilen)) != 0) {
        goto exit;
    }

    if ((ret = mbedtls_md5_finish(&ctx, output)) != 0) {
        goto exit;
    }

exit:
    mbedtls_md5_free(&ctx);
    return ret;
}

/* ---- Function pointer table (vtable) pattern ---- */

/*
 * mbedtls_md_info_t: the canonical mbedtls vtable.
 * c2ul stress: struct with multiple function pointer fields.
 */
typedef struct {
    const char *name;
    int         digest_size;
    int         block_size;
    int       (*starts_func)(void *ctx);
    int       (*update_func)(void *ctx, const uint8_t *input, int ilen);
    int       (*finish_func)(void *ctx, uint8_t *output);
    int       (*digest_func)(const uint8_t *input, int ilen, uint8_t *output);
    void      (*ctx_alloc_func)(void *ctx);
    void      (*ctx_free_func)(void *ctx);
    void      (*clone_func)(void *dst, const void *src);
} mbedtls_md_info_t;

static int md5_starts_wrap(void *ctx)
{
    return mbedtls_md5_starts((mbedtls_md5_context *)ctx);
}

static int md5_update_wrap(void *ctx, const uint8_t *input, int ilen)
{
    return mbedtls_md5_update((mbedtls_md5_context *)ctx, input, ilen);
}

static int md5_finish_wrap(void *ctx, uint8_t *output)
{
    return mbedtls_md5_finish((mbedtls_md5_context *)ctx, output);
}

static int md5_digest_wrap(const uint8_t *input, int ilen, uint8_t *output)
{
    return mbedtls_md5(input, ilen, output);
}

static void md5_ctx_alloc(void *ctx)
{
    mbedtls_md5_init((mbedtls_md5_context *)ctx);
}

static void md5_ctx_free(void *ctx)
{
    mbedtls_md5_free((mbedtls_md5_context *)ctx);
}

static void md5_clone_wrap(void *dst, const void *src)
{
    mbedtls_md5_clone((mbedtls_md5_context *)dst,
                      (const mbedtls_md5_context *)src);
}

static const mbedtls_md_info_t mbedtls_md5_info = {
    "MD5",
    16,
    64,
    md5_starts_wrap,
    md5_update_wrap,
    md5_finish_wrap,
    md5_digest_wrap,
    md5_ctx_alloc,
    md5_ctx_free,
    md5_clone_wrap,
};

/* ---- Test harness ---- */

static void print_hex(const uint8_t *buf, int len)
{
    int i;
    for (i = 0; i < len; i++) {
        printf("%02X", buf[i]);
    }
}

int main(void)
{
    int i, ret;
    uint8_t digest[16];

    /* RFC 1321 test vectors */
    static const struct {
        const char *input;
        int len;
        const uint8_t expected[16];
    } tests[] = {
        { "", 0,
          { 0xD4,0x1D,0x8C,0xD9,0x8F,0x00,0xB2,0x04,
            0xE9,0x80,0x09,0x98,0xEC,0xF8,0x42,0x7E } },
        { "a", 1,
          { 0x0C,0xC1,0x75,0xB9,0xC0,0xF1,0xB6,0xA8,
            0x31,0xC3,0x99,0xE2,0x69,0x77,0x26,0x61 } },
        { "abc", 3,
          { 0x90,0x01,0x50,0x98,0x3C,0xD2,0x4F,0xB0,
            0xD6,0x96,0x3F,0x7D,0x28,0xE1,0x7F,0x72 } },
        { "message digest", 14,
          { 0xF9,0x6B,0x69,0x7D,0x7C,0xB7,0x93,0x8D,
            0x52,0x5A,0x2F,0x31,0xAA,0xF1,0x61,0xD0 } },
    };

    printf("=== MD5 RFC 1321 vectors ===\n");
    for (i = 0; i < 4; i++) {
        ret = mbedtls_md5((const uint8_t *)tests[i].input, tests[i].len, digest);
        int ok = (ret == 0 && memcmp(digest, tests[i].expected, 16) == 0);
        printf("MD5(\"%s\"): ", tests[i].input);
        print_hex(digest, 16);
        printf(" %s\n", ok ? "PASS" : "FAIL");
    }

    /* Test the vtable dispatch path */
    printf("\n=== MD5 vtable dispatch ===\n");
    {
        mbedtls_md5_context ctx;
        const mbedtls_md_info_t *info = &mbedtls_md5_info;

        info->ctx_alloc_func(&ctx);
        info->starts_func(&ctx);
        info->update_func(&ctx, (const uint8_t *)"abc", 3);
        info->finish_func(&ctx, digest);

        static const uint8_t abc_expected[16] = {
            0x90,0x01,0x50,0x98,0x3C,0xD2,0x4F,0xB0,
            0xD6,0x96,0x3F,0x7D,0x28,0xE1,0x7F,0x72
        };
        int ok = (memcmp(digest, abc_expected, 16) == 0);
        printf("vtable MD5(\"abc\"): ");
        print_hex(digest, 16);
        printf(" %s\n", ok ? "PASS" : "FAIL");

        printf("vtable name=%s digest_size=%d block_size=%d\n",
               info->name, info->digest_size, info->block_size);
    }

    /* Test clone */
    printf("\n=== MD5 clone ===\n");
    {
        mbedtls_md5_context ctx1, ctx2;
        uint8_t d1[16], d2[16];

        mbedtls_md5_init(&ctx1);
        mbedtls_md5_starts(&ctx1);
        mbedtls_md5_update(&ctx1, (const uint8_t *)"hello", 5);

        mbedtls_md5_clone(&ctx2, &ctx1);

        mbedtls_md5_finish(&ctx1, d1);
        mbedtls_md5_finish(&ctx2, d2);

        int ok = (memcmp(d1, d2, 16) == 0);
        printf("clone match: %s\n", ok ? "PASS" : "FAIL");
    }

    return 0;
}
