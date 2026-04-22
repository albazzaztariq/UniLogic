/* test_redis_zipint.c
 * Extracted from Redis ziplist.c: integer encoding selection logic.
 * Tests the classification of integers into their encoding buckets:
 *   IMM (0-12), INT8, INT16, INT24, INT32, INT64.
 * Pure integer comparisons - no memory allocation, no string ops.
 */
#include <stdio.h>
#include <stdint.h>
#include <limits.h>

/* Encoding constants from ziplist.c */
#define ZIP_INT_16B  (0xc0 | 1<<4)   /* 0xD0: 2 bytes */
#define ZIP_INT_32B  (0xc0 | 2<<4)   /* 0xE0: 4 bytes */
#define ZIP_INT_64B  (0xc0 | 3<<4)   /* 0xF0: 8 bytes */
#define ZIP_INT_24B  (0xc0 | 4<<4)   /* not quite right in original, kept for test */
#define ZIP_INT_8B   0xfe             /* 1 byte */
#define ZIP_INT_IMM_MIN 0xf1          /* 4 bit immediate, 0001-1101 => 0-12 */
#define ZIP_INT_IMM_MAX 0xfd

#define INT24_MIN (-8388608)
#define INT24_MAX  8388607

/* Select encoding for integer value (extracted from zipTryEncoding logic) */
static int zipIntEncoding(long long value)
{
    if (value >= 0 && value <= 12) {
        return ZIP_INT_IMM_MIN + (int)value;
    } else if (value >= -128 && value <= 127) {
        return ZIP_INT_8B;
    } else if (value >= -32768 && value <= 32767) {
        return ZIP_INT_16B;
    } else if (value >= INT24_MIN && value <= INT24_MAX) {
        return ZIP_INT_24B;
    } else if (value >= -2147483648LL && value <= 2147483647LL) {
        return ZIP_INT_32B;
    } else {
        return ZIP_INT_64B;
    }
}

static const char *encName(int enc)
{
    if (enc >= ZIP_INT_IMM_MIN && enc <= ZIP_INT_IMM_MAX) return "IMM";
    if (enc == ZIP_INT_8B)  return "INT8";
    if (enc == ZIP_INT_16B) return "INT16";
    if (enc == ZIP_INT_24B) return "INT24";
    if (enc == ZIP_INT_32B) return "INT32";
    if (enc == ZIP_INT_64B) return "INT64";
    return "UNKNOWN";
}

int main(void)
{
    long long tests[] = {
        0, 1, 12, 13,
        -1, -128, 127, -129, 128,
        -32768, 32767, -32769, 32768,
        INT24_MIN, INT24_MAX, INT24_MIN - 1, INT24_MAX + 1,
        -2147483648LL, 2147483647LL, -2147483649LL, 2147483648LL,
        -9223372036854775807LL - 1, 9223372036854775807LL
    };
    int n = (int)(sizeof(tests) / sizeof(tests[0]));
    int i;
    for (i = 0; i < n; i++) {
        int enc = zipIntEncoding(tests[i]);
        printf("%s\n", encName(enc));
    }
    return 0;
}
