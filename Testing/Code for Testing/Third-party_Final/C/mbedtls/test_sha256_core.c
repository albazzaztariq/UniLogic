/*
 * test_sha256_core.c
 * Clean SHA-256 core operations — no pointers, no goto, no function pointers.
 * Tests: structs with array fields, switch/case, error propagation, bitwise ops.
 *
 * Patterns exercised (c2ul friendly):
 *   1. Struct with fixed-size array fields (sha_state with h[8], w[16])
 *   2. Struct field assignment
 *   3. Error code propagation (init → process chain)
 *   4. switch/case dispatch
 *   5. Large constant table lookup (K constants)
 *   6. Bitwise/rotate operations
 *   7. Index-based array access (no pointer arithmetic)
 */

#include <stdio.h>

typedef unsigned int  uint32_t;
typedef unsigned char uint8_t;

/* ================================================================
 * SHA-256 round constants (first 16)
 * ================================================================ */
static const uint32_t K[16] = {
    0x428A2F98u, 0x71374491u, 0xB5C0FBCFu, 0xE9B5DBA5u,
    0x3956C25Bu, 0x59F111F1u, 0x923F82A4u, 0xAB1C5ED5u,
    0xD807AA98u, 0x12835B01u, 0x243185BEu, 0x550C7DC3u,
    0x72BE5D74u, 0x80DEB1FEu, 0x9BDC06A7u, 0xC19BF174u
};

/* ================================================================
 * Bit rotation helpers
 * ================================================================ */
static uint32_t rotr(uint32_t x, int n)
{
    return (x >> n) | (x << (32 - n));
}

static uint32_t sigma0(uint32_t x)
{
    return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22);
}

static uint32_t sigma1(uint32_t x)
{
    return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25);
}

static uint32_t ch(uint32_t e, uint32_t f, uint32_t g)
{
    return (e & f) ^ (~e & g);
}

static uint32_t maj(uint32_t a, uint32_t b, uint32_t c)
{
    return (a & b) ^ (a & c) ^ (b & c);
}

/* ================================================================
 * SHA-256 state struct — fixed-size array fields
 * ================================================================ */
typedef struct {
    uint32_t h[8];   /* hash state */
    uint32_t w[16];  /* message schedule (first 16 words) */
    int      round;  /* current round */
    int      status; /* 0=OK, -1=error */
} sha256_ctx;

/* Initialize with SHA-256 initial hash values */
static int sha256_init(sha256_ctx *ctx)
{
    ctx->h[0] = 0x6A09E667u;
    ctx->h[1] = 0xBB67AE85u;
    ctx->h[2] = 0x3C6EF372u;
    ctx->h[3] = 0xA54FF53Au;
    ctx->h[4] = 0x510E527Fu;
    ctx->h[5] = 0x9B05688Cu;
    ctx->h[6] = 0x1F83D9ABu;
    ctx->h[7] = 0x5BE0CD19u;
    ctx->round = 0;
    ctx->status = 0;
    return 0;
}

/* Load 16 message words from 4-byte index-based access */
static int sha256_load_block(sha256_ctx *ctx,
                              uint32_t w0, uint32_t w1, uint32_t w2, uint32_t w3,
                              uint32_t w4, uint32_t w5, uint32_t w6, uint32_t w7)
{
    if (ctx->status != 0) return ctx->status;
    ctx->w[0] = w0; ctx->w[1] = w1; ctx->w[2] = w2;  ctx->w[3] = w3;
    ctx->w[4] = w4; ctx->w[5] = w5; ctx->w[6] = w6;  ctx->w[7] = w7;
    ctx->w[8]  = 0; ctx->w[9]  = 0; ctx->w[10] = 0;  ctx->w[11] = 0;
    ctx->w[12] = 0; ctx->w[13] = 0; ctx->w[14] = 0;  ctx->w[15] = 0;
    return 0;
}

/* Do one SHA-256 round using working variables a..h */
static int sha256_do_round(sha256_ctx *ctx, int r)
{
    uint32_t a, b, c, d, e, f, g, h;
    uint32_t t1, t2;

    if (ctx->status != 0) return ctx->status;
    if (r < 0 || r >= 16) { ctx->status = -1; return -1; }

    a = ctx->h[0]; b = ctx->h[1]; c = ctx->h[2]; d = ctx->h[3];
    e = ctx->h[4]; f = ctx->h[5]; g = ctx->h[6]; h = ctx->h[7];

    t1 = h + sigma1(e) + ch(e, f, g) + K[r] + ctx->w[r];
    t2 = sigma0(a) + maj(a, b, c);

    ctx->h[7] = g;
    ctx->h[6] = f;
    ctx->h[5] = e;
    ctx->h[4] = d + t1;
    ctx->h[3] = c;
    ctx->h[2] = b;
    ctx->h[1] = a;
    ctx->h[0] = t1 + t2;

    ctx->round = r + 1;
    return 0;
}

/* ================================================================
 * Switch/case: round type classification
 * ================================================================ */
static int classify_round(int r)
{
    switch (r % 4) {
        case 0: return 10;
        case 1: return 20;
        case 2: return 30;
        case 3: return 40;
        default: return -1;
    }
}

/* ================================================================
 * Constant table lookup: K[i] value access
 * ================================================================ */
static uint32_t get_k(int i)
{
    if (i < 0 || i >= 16) return 0;
    return K[i];
}

/* ================================================================
 * TEST HARNESS
 * ================================================================ */

int main(void)
{
    sha256_ctx ctx;
    int ret;

    printf("=== 1. SHA-256 init ===\n");
    ret = sha256_init(&ctx);
    printf("init ret=%d %s\n", ret, ret == 0 ? "PASS" : "FAIL");
    /* h[0] = 0x6A09E667 = 1779033703 */
    printf("h[0]=%u %s\n", ctx.h[0],
           ctx.h[0] == 1779033703u ? "PASS" : "FAIL");
    /* h[7] = 0x5BE0CD19 = 1541459225 (actual SHA-256 IV) */
    printf("h[7]=%u %s\n", ctx.h[7],
           ctx.h[7] == 1541459225u ? "PASS" : "FAIL");
    printf("round=%d %s\n", ctx.round, ctx.round == 0 ? "PASS" : "FAIL");
    printf("status=%d %s\n", ctx.status, ctx.status == 0 ? "PASS" : "FAIL");

    printf("\n=== 2. Load block ===\n");
    /* "abc" SHA-256 first block: 0x61626380, 0, 0, ..., 0x00000018 */
    ret = sha256_load_block(&ctx,
                             0x61626380u, 0u, 0u, 0u,
                             0u, 0u, 0u, 0u);
    printf("load ret=%d %s\n", ret, ret == 0 ? "PASS" : "FAIL");
    /* w[0] = 0x61626380 = 1633837952 */
    printf("w[0]=%u %s\n", ctx.w[0],
           ctx.w[0] == 1633837952u ? "PASS" : "FAIL");
    printf("w[7]=%u %s\n", ctx.w[7], ctx.w[7] == 0u ? "PASS" : "FAIL");

    printf("\n=== 3. One SHA-256 round ===\n");
    ret = sha256_do_round(&ctx, 0);
    printf("round ret=%d %s\n", ret, ret == 0 ? "PASS" : "FAIL");
    printf("round count=%d %s\n", ctx.round, ctx.round == 1 ? "PASS" : "FAIL");
    /* After round 0 with "abc" block: h[0] should be t1+t2 from the round */
    /* Compute: t1 = h + sigma1(e) + ch(e,f,g) + K[0] + w[0] */
    /* h=1541325730, e=1359893119, f=2600822924, g=528734635 */
    /* K[0]=0x428A2F98=1116352408, w[0]=1633837952 */
    /* Just verify it changed from initial value */
    printf("h[0] changed: %s\n",
           ctx.h[0] != 1779033703u ? "PASS" : "FAIL");
    /* h[1] should be old h[0] = 1779033703 */
    printf("h[1]=old h[0]: %s\n",
           ctx.h[1] == 1779033703u ? "PASS" : "FAIL");

    printf("\n=== 4. Error propagation ===\n");
    /* Force error by passing out-of-range round */
    ret = sha256_do_round(&ctx, 99);
    printf("bad round ret=%d %s\n", ret, ret == -1 ? "PASS" : "FAIL");
    printf("status=%d %s\n", ctx.status, ctx.status == -1 ? "PASS" : "FAIL");
    /* Status -1 should block further operations */
    ret = sha256_load_block(&ctx, 0, 0, 0, 0, 0, 0, 0, 0);
    printf("blocked load ret=%d %s\n", ret, ret == -1 ? "PASS" : "FAIL");

    printf("\n=== 5. Switch/case classification ===\n");
    printf("classify(0)=%d %s\n", classify_round(0),
           classify_round(0) == 10 ? "PASS" : "FAIL");
    printf("classify(1)=%d %s\n", classify_round(1),
           classify_round(1) == 20 ? "PASS" : "FAIL");
    printf("classify(5)=%d %s\n", classify_round(5),
           classify_round(5) == 20 ? "PASS" : "FAIL");
    printf("classify(7)=%d %s\n", classify_round(7),
           classify_round(7) == 40 ? "PASS" : "FAIL");

    printf("\n=== 6. K constant table ===\n");
    /* K[0]=0x428A2F98=1116352408 */
    printf("K[0]=%u %s\n", get_k(0),
           get_k(0) == 1116352408u ? "PASS" : "FAIL");
    /* K[7]=0xAB1C5ED5=2870763221 */
    printf("K[7]=%u %s\n", get_k(7),
           get_k(7) == 2870763221u ? "PASS" : "FAIL");
    printf("K[-1]=%u %s\n", get_k(-1), get_k(-1) == 0u ? "PASS" : "FAIL");
    printf("K[16]=%u %s\n", get_k(16), get_k(16) == 0u ? "PASS" : "FAIL");

    printf("\n=== 7. Determinism check ===\n");
    {
        sha256_ctx c1, c2;
        sha256_init(&c1); sha256_init(&c2);
        sha256_load_block(&c1, 0x61626380u, 0, 0, 0, 0, 0, 0, 0);
        sha256_load_block(&c2, 0x61626380u, 0, 0, 0, 0, 0, 0, 0);
        sha256_do_round(&c1, 0);
        sha256_do_round(&c2, 0);
        printf("two-ctx match: %s\n",
               c1.h[0] == c2.h[0] ? "PASS" : "FAIL");
        sha256_do_round(&c1, 1);
        sha256_do_round(&c2, 1);
        printf("after round 1: %s\n",
               c1.h[0] == c2.h[0] ? "PASS" : "FAIL");
    }

    return 0;
}
