/*
 * test_curl_ctype.c
 * Extracted from curl lib/curl_ctype.h — character classification macros.
 * Inlined as pure integer functions. No pointers, no malloc.
 * Tests: ISALPHA, ISDIGIT, ISXDIGIT, ISUNRESERVED, ISSPACE
 *
 * Note: uses integer-only patterns compatible with UL transpiler.
 * Uses %d format (not %x) to avoid format specifier translation issues.
 */
#include <stdio.h>

/* Direct inline of curl_ctype.h macros as functions */

static int curl_islower(int x)  { return (x >= 'a' && x <= 'z'); }
static int curl_isupper(int x)  { return (x >= 'A' && x <= 'Z'); }
static int curl_isdigit(int x)  { return (x >= '0' && x <= '9'); }
static int curl_isalpha(int x)  { return curl_islower(x) || curl_isupper(x); }
static int curl_isalnum(int x)  { return curl_isdigit(x) || curl_islower(x) || curl_isupper(x); }

static int curl_islowhexalpha(int x) { return (x >= 'a' && x <= 'f'); }
static int curl_isuphexalpha(int x)  { return (x >= 'A' && x <= 'F'); }
static int curl_isxdigit(int x)
{
    return curl_isdigit(x) || curl_islowhexalpha(x) || curl_isuphexalpha(x);
}

static int curl_isurlpuntcs(int x)
{
    return (x == '-' || x == '.' || x == '_' || x == '~');
}

static int curl_isunreserved(int x)
{
    return curl_isalnum(x) || curl_isurlpuntcs(x);
}

static int curl_isblank(int x)  { return (x == ' ' || x == '\t'); }
static int curl_isspace(int x)
{
    return curl_isblank(x) || (x >= 0x0a && x <= 0x0d);
}

/* HTTP token char: used in header parsing */
static int curl_is_token_char(int x)
{
    /* RFC 7230: token = any visible char except delimiters */
    if (x <= 0x20 || x >= 0x7f) return 0;
    switch (x) {
    case '(': case ')': case ',': case '/':
    case ':': case ';': case '<': case '=':
    case '>': case '?': case '@': case '[':
    case '\\': case ']': case '{': case '}':
    case '"':
        return 0;
    default:
        return 1;
    }
}

int main(void)
{
    int i;
    /* Test set: char codes of interest (decimal values) */
    int chars[] = {48, 57, 97, 102, 122, 65, 70, 90,
                   103, 71, 45, 46, 95, 126, 43, 47,
                   32, 9, 10, 13, 33, 35, 37, 38};
    int n = 24;

    printf("=== curl ctype checks ===\n");

    /* Use %d for char code to avoid format specifier translation issues */
    for (i = 0; i < n; i++) {
        int c1 = chars[i];
        printf("chr=%d alpha=%d digit=%d xdigit=%d alnum=%d unresvd=%d space=%d\n",
               c1,
               curl_isalpha(c1),
               curl_isdigit(c1),
               curl_isxdigit(c1),
               curl_isalnum(c1),
               curl_isunreserved(c1),
               curl_isspace(c1));
    }

    printf("=== token char check ===\n");
    int tkchars[] = {97, 90, 48, 45, 95, 40, 58, 32, 10};
    int tn = 9;
    for (i = 0; i < tn; i++) {
        int c2 = tkchars[i];
        printf("chr=%d token=%d\n", c2, curl_is_token_char(c2));
    }

    return 0;
}
