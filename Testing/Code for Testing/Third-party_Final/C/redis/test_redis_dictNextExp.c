/* test_redis_dictNextExp.c
 * Extracted from Redis dict.c: _dictNextExp()
 * Returns the smallest power-of-2 exponent >= size.
 * Pure integer math - no malloc, no stdio beyond printf.
 */
#include <stdio.h>
#include <limits.h>

#define DICT_HT_INITIAL_EXP  2
#define DICT_HT_INITIAL_SIZE (1<<(DICT_HT_INITIAL_EXP))

static signed char dictNextExp(unsigned long size)
{
    if (size <= DICT_HT_INITIAL_SIZE) return DICT_HT_INITIAL_EXP;
    if (size >= LONG_MAX) return (8*sizeof(long)-1);

    return (signed char)(8*sizeof(long) - __builtin_clzl(size-1));
}

int main(void)
{
    printf("%d\n", (int)dictNextExp(0));
    printf("%d\n", (int)dictNextExp(1));
    printf("%d\n", (int)dictNextExp(4));
    printf("%d\n", (int)dictNextExp(5));
    printf("%d\n", (int)dictNextExp(8));
    printf("%d\n", (int)dictNextExp(9));
    printf("%d\n", (int)dictNextExp(16));
    printf("%d\n", (int)dictNextExp(17));
    printf("%d\n", (int)dictNextExp(1024));
    printf("%d\n", (int)dictNextExp(1025));
    printf("%d\n", (int)dictNextExp(65536));
    printf("%d\n", (int)dictNextExp(65537));
    return 0;
}
