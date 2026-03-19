#include <stdio.h>
#include <stdint.h>
#include <string.h>

void test_int_match(void);
void test_default(void);
void test_match_expr(void);
void test_match_negative(void);
int main(void);

void test_int_match(void)
{
    int x = 2;
    if (x == 1) {
        printf("%s\n", "one");
    } else if (x == 2) {
        printf("%s\n", "two");
    } else if (x == 3) {
        printf("%s\n", "three");
    } else {
        printf("%s\n", "other");
    }
}

void test_default(void)
{
    int y = 99;
    if (y == 1) {
        printf("%s\n", "one");
    } else if (y == 2) {
        printf("%s\n", "two");
    } else {
        printf("%s\n", "default hit");
    }
}

void test_match_expr(void)
{
    int val = (3 + 2);
    if (val == 4) {
        printf("%s\n", "four");
    } else if (val == 5) {
        printf("%s\n", "five");
    } else if (val == 6) {
        printf("%s\n", "six");
    }
}

void test_match_negative(void)
{
    int n = (-1);
    if (n == (-1)) {
        printf("%s\n", "negative one");
    } else if (n == 0) {
        printf("%s\n", "zero");
    } else if (n == 1) {
        printf("%s\n", "positive one");
    }
}

int main(void)
{
    test_int_match();
    test_default();
    test_match_expr();
    test_match_negative();
    return 0;
}

