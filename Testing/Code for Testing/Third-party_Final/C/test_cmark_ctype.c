/* Test harness for cmark_ctype functions — exercises isspace, ispunct, isdigit, isalpha */
#include <stdio.h>

/* Forward declarations of the functions under test */
int cmark_isspace(int c);
int cmark_ispunct(int c);
int cmark_isdigit(int c);
int cmark_isalpha(int c);

int main(void) {
    /* spaces */
    printf("isspace(tab)=%d\n", cmark_isspace('\t'));
    printf("isspace(newline)=%d\n", cmark_isspace('\n'));
    printf("isspace(space)=%d\n", cmark_isspace(' '));
    printf("isspace(A)=%d\n", cmark_isspace('A'));

    /* punctuation */
    printf("ispunct(!)=%d\n", cmark_ispunct('!'));
    printf("ispunct(.)=%d\n", cmark_ispunct('.'));
    printf("ispunct(a)=%d\n", cmark_ispunct('a'));
    printf("ispunct(1)=%d\n", cmark_ispunct('1'));

    /* digits */
    printf("isdigit(0)=%d\n", cmark_isdigit('0'));
    printf("isdigit(9)=%d\n", cmark_isdigit('9'));
    printf("isdigit(a)=%d\n", cmark_isdigit('a'));

    /* alpha */
    printf("isalpha(a)=%d\n", cmark_isalpha('a'));
    printf("isalpha(Z)=%d\n", cmark_isalpha('Z'));
    printf("isalpha(5)=%d\n", cmark_isalpha('5'));

    return 0;
}
