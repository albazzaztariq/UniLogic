/*
 * test_curl_hexval.c
 * Extracted from curl lib/curlx/strparse.h — hex character to value conversion.
 * Also tests the URL percent-decode inner logic from escape.c.
 * Pure integer arithmetic. No dynamic memory.
 *
 * Note: avoids %c format and char literals to work with UL transpiler.
 * All char comparisons use integer codes directly.
 */
#include <stdio.h>

/*
 * hex_char_val — convert a single hex character code to its numeric value (0-15).
 * Returns -1 for invalid input.
 * Based on curl's curlx_hexval() macro logic.
 */
static int hex_char_val(int c)
{
    if (c >= 48 && c <= 57)  return c - 48;        /* '0'-'9' */
    if (c >= 97 && c <= 102) return c - 97 + 10;   /* 'a'-'f' */
    if (c >= 65 && c <= 70)  return c - 65 + 10;   /* 'A'-'F' */
    return -1;
}

/*
 * hex_pair_val — convert two consecutive hex char codes to a byte value.
 * Returns -1 if either char is invalid.
 */
static int hex_pair_val(int hi, int lo)
{
    int h = hex_char_val(hi);
    int l = hex_char_val(lo);
    if (h < 0 || l < 0) return -1;
    return (h << 4) | l;
}

/*
 * is_xdigit — return 1 if c is a valid hex digit char code
 */
static int is_xdigit(int c)
{
    return (c >= 48 && c <= 57) ||   /* '0'-'9' */
           (c >= 97 && c <= 102) ||  /* 'a'-'f' */
           (c >= 65 && c <= 70);     /* 'A'-'F' */
}

/*
 * count_encoded_chars — count valid %XX sequences in a string.
 * Operates on ASCII integer codes in an array.
 */
static int count_encoded_seqs(const int *codes, int len)
{
    int count = 0;
    int i = 0;
    while (i < len) {
        /* '%' = 37 */
        if (codes[i] == 37 && (i + 2) < len &&
            is_xdigit(codes[i+1]) && is_xdigit(codes[i+2])) {
            count++;
            i += 3;
        } else {
            i++;
        }
    }
    return count;
}

int main(void)
{
    int i;

    printf("=== Hex Char Value Tests ===\n");
    /* Digits 48-57 = '0'-'9' */
    for (i = 48; i <= 57; i++)
        printf("code=%d val=%d\n", i, hex_char_val(i));
    /* Lowercase hex 97-102 = 'a'-'f' */
    for (i = 97; i <= 102; i++)
        printf("code=%d val=%d\n", i, hex_char_val(i));
    /* Uppercase hex 65-70 = 'A'-'F' */
    for (i = 65; i <= 70; i++)
        printf("code=%d val=%d\n", i, hex_char_val(i));
    /* Invalid */
    printf("code=103 val=%d\n", hex_char_val(103));  /* 'g' */
    printf("code=71 val=%d\n",  hex_char_val(71));   /* 'G' */
    printf("code=32 val=%d\n",  hex_char_val(32));   /* ' ' */

    printf("=== Hex Pair Value Tests ===\n");
    /* '2'=50, '0'=48 -> 32 (space) */
    printf("hi=50 lo=48 val=%d\n", hex_pair_val(50, 48));
    /* '4'=52, '1'=49 -> 65 ('A') */
    printf("hi=52 lo=49 val=%d\n", hex_pair_val(52, 49));
    /* 'F'=70, 'F'=70 -> 255 */
    printf("hi=70 lo=70 val=%d\n", hex_pair_val(70, 70));
    /* '0'=48, '0'=48 -> 0 */
    printf("hi=48 lo=48 val=%d\n", hex_pair_val(48, 48));
    /* 'a'=97, 'b'=98 -> 171 */
    printf("hi=97 lo=98 val=%d\n", hex_pair_val(97, 98));
    /* 'G'=71, '0'=48 -> -1 (invalid) */
    printf("hi=71 lo=48 val=%d\n", hex_pair_val(71, 48));

    printf("=== Percent-Encoded Detection ===\n");
    /* "hello%20world" = 104,101,108,108,111,37,50,48,119,111,114,108,100 */
    {
        int s1[] = {104,101,108,108,111,37,50,48,119,111,114,108,100};
        int s1len = 13;
        /* "%41%42%43" = 37,52,49,37,52,50,37,52,51 */
        int s2[] = {37,52,49,37,52,50,37,52,51};
        int s2len = 9;
        /* "no%encoding" -> 'n','o','%','e','n','c','o','d','i','n','g'
           %en: 'e'=101 not hex A-F or a-f past f, 'n'=110 not hex -> invalid */
        int s3[] = {110,111,37,101,110,99,111,100,105,110,103};
        int s3len = 11;
        /* "trail%2" -> incomplete sequence */
        int s4[] = {116,114,97,105,108,37,50};
        int s4len = 7;
        /* "%2F%7E%21" = 37,50,70,37,55,69,37,50,49 */
        int s5[] = {37,50,70,37,55,69,37,50,49};
        int s5len = 9;

        printf("s1 count=%d\n", count_encoded_seqs(s1, s1len));
        printf("s2 count=%d\n", count_encoded_seqs(s2, s2len));
        printf("s3 count=%d\n", count_encoded_seqs(s3, s3len));
        printf("s4 count=%d\n", count_encoded_seqs(s4, s4len));
        printf("s5 count=%d\n", count_encoded_seqs(s5, s5len));
    }

    return 0;
}
