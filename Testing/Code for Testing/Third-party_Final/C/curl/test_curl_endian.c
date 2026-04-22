/*
 * test_curl_endian.c
 * Extracted from curl lib/curl_endian.c — endian conversion functions.
 * Uses array indexing on int arrays (no pointer arithmetic).
 * Exercises bit operations (shift + OR) for endian byte assembly.
 *
 * Note: restricted to values that fit in signed int to avoid
 * signed/unsigned print mismatch in UL-generated code.
 * Tests 16-bit values and 32-bit values that don't overflow int.
 */
#include <stdio.h>

/*
 * read16_le — read 2 bytes as little-endian 16-bit value
 */
static int read16_le(const int *buf)
{
    return (buf[0]) | (buf[1] << 8);
}

/*
 * read16_be — read 2 bytes as big-endian 16-bit value
 */
static int read16_be(const int *buf)
{
    return (buf[0] << 8) | (buf[1]);
}

/*
 * read32_le — read 4 bytes as little-endian 32-bit value
 * (only safe for small values that fit in signed int)
 */
static int read32_le(const int *buf)
{
    return (buf[0]) | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24);
}

/*
 * read32_be — read 4 bytes as big-endian 32-bit value
 * (only safe for small values that fit in signed int)
 */
static int read32_be(const int *buf)
{
    return (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | (buf[3]);
}

int main(void)
{
    /* Use small values only to avoid signed int overflow */
    int data1[4] = {1, 2, 3, 4};
    int data2[4] = {5, 0, 0, 0};
    int data3[4] = {0, 0, 0, 0};
    int data4[4] = {127, 127, 0, 0};

    printf("=== Endian Read Tests ===\n");

    printf("LE16 [01,02] = %d\n", read16_le(data1));
    printf("LE16 [05,00] = %d\n", read16_le(data2));
    printf("LE16 [00,00] = %d\n", read16_le(data3));
    printf("LE16 [7F,7F] = %d\n", read16_le(data4));

    printf("BE16 [01,02] = %d\n", read16_be(data1));
    printf("BE16 [05,00] = %d\n", read16_be(data2));
    printf("BE16 [00,00] = %d\n", read16_be(data3));
    printf("BE16 [7F,7F] = %d\n", read16_be(data4));

    /* 32-bit: use only small non-overflowing values */
    int data5[4] = {1, 0, 0, 0};    /* LE=1, BE=16777216 */
    int data6[4] = {0, 0, 0, 1};    /* LE=16777216, BE=1 */
    int data7[4] = {1, 2, 3, 4};    /* LE=67305985, BE=16909060 */
    int data8[4] = {16, 32, 48, 64};/* non-trivial safe values */

    printf("LE32 [01,00,00,00] = %d\n", read32_le(data5));
    printf("LE32 [00,00,00,01] = %d\n", read32_le(data6));
    printf("LE32 [01,02,03,04] = %d\n", read32_le(data7));
    printf("LE32 [10,20,30,40] = %d\n", read32_le(data8));

    printf("BE32 [01,00,00,00] = %d\n", read32_be(data5));
    printf("BE32 [00,00,00,01] = %d\n", read32_be(data6));
    printf("BE32 [01,02,03,04] = %d\n", read32_be(data7));
    printf("BE32 [10,20,30,40] = %d\n", read32_be(data8));

    return 0;
}
