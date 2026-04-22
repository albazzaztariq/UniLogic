/*
 * Extracted from jgm/cmark — src/utf8.c
 * cmark_utf8proc_is_space(): pure integer logic, no structs/pointers.
 */
#include <stdio.h>

/* Verbatim from cmark/src/utf8.c (matches anything in the Zs class, plus LF, CR, TAB, FF) */
int cmark_utf8proc_is_space(int uc) {
    return (uc == 9 || uc == 10 || uc == 12 || uc == 13 || uc == 32 ||
            uc == 160 || uc == 5760 || (uc >= 8192 && uc <= 8202) || uc == 8239 ||
            uc == 8287 || uc == 12288);
}

int main(void) {
    /* Test space characters */
    printf("is_space(9)  = %d\n",  cmark_utf8proc_is_space(9));   /* TAB */
    printf("is_space(10) = %d\n",  cmark_utf8proc_is_space(10));  /* LF */
    printf("is_space(12) = %d\n",  cmark_utf8proc_is_space(12));  /* FF */
    printf("is_space(13) = %d\n",  cmark_utf8proc_is_space(13));  /* CR */
    printf("is_space(32) = %d\n",  cmark_utf8proc_is_space(32));  /* SPACE */
    printf("is_space(160) = %d\n", cmark_utf8proc_is_space(160)); /* NBSP */
    printf("is_space(5760) = %d\n",cmark_utf8proc_is_space(5760));/* OGHAM SPACE */
    printf("is_space(8192) = %d\n",cmark_utf8proc_is_space(8192));/* EN QUAD */
    printf("is_space(8202) = %d\n",cmark_utf8proc_is_space(8202));/* HAIR SPACE */
    printf("is_space(8239) = %d\n",cmark_utf8proc_is_space(8239));/* NARROW NBSP */
    printf("is_space(8287) = %d\n",cmark_utf8proc_is_space(8287));/* MEDIUM MATH */
    printf("is_space(12288) = %d\n",cmark_utf8proc_is_space(12288));/* IDEOGRAPHIC SPACE */
    /* Test non-space characters */
    printf("is_space(65) = %d\n",  cmark_utf8proc_is_space(65));  /* 'A' */
    printf("is_space(0) = %d\n",   cmark_utf8proc_is_space(0));   /* NUL */
    printf("is_space(8191) = %d\n",cmark_utf8proc_is_space(8191));/* just below range */
    printf("is_space(8203) = %d\n",cmark_utf8proc_is_space(8203));/* just above range */
    return 0;
}
