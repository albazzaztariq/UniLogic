/*
 * Extracted from DaveGamble/cJSON — cJSON.c
 * parse_hex4(): parse a 4-character hex string into an unsigned int.
 * Pure integer/array logic — no pointer arithmetic, no structs.
 */
#include <stdio.h>

/* Verbatim from cJSON.c (simplified to avoid unsigned char vs int issues) */
static unsigned parse_hex4(const char *input) {
    unsigned int h = 0;
    int i = 0;
    while (i < 4) {
        char c = input[i];
        if (c >= '0' && c <= '9') {
            h += (unsigned)(c - '0');
        } else if (c >= 'A' && c <= 'F') {
            h += (unsigned)(10 + c - 'A');
        } else if (c >= 'a' && c <= 'f') {
            h += (unsigned)(10 + c - 'a');
        } else {
            return 0;
        }
        if (i < 3) {
            h <<= 4;
        }
        i++;
    }
    return h;
}

int main(void) {
    printf("parse_hex4(\"0000\") = %u\n", parse_hex4("0000"));
    printf("parse_hex4(\"000F\") = %u\n", parse_hex4("000F"));
    printf("parse_hex4(\"00FF\") = %u\n", parse_hex4("00FF"));
    printf("parse_hex4(\"0FFF\") = %u\n", parse_hex4("0FFF"));
    printf("parse_hex4(\"FFFF\") = %u\n", parse_hex4("FFFF"));
    printf("parse_hex4(\"1A2B\") = %u\n", parse_hex4("1A2B"));
    printf("parse_hex4(\"dead\") = %u\n", parse_hex4("dead"));
    printf("parse_hex4(\"BEEF\") = %u\n", parse_hex4("BEEF"));
    printf("parse_hex4(\"4E2D\") = %u\n", parse_hex4("4E2D"));
    printf("parse_hex4(\"00GG\") = %u\n", parse_hex4("00GG")); /* invalid -> 0 */
    return 0;
}
