/* test_redis_zslRandomLevel.c
 * Extracted from Redis t_zset.c: zslRandomLevel()
 * Generates random level for skip list nodes using powerlaw distribution.
 * Uses seeded random() so output is deterministic.
 */
#include <stdio.h>
#include <stdlib.h>

#define ZSKIPLIST_MAXLEVEL 32
#define ZSKIPLIST_P        0.25

static int zslRandomLevel(void)
{
    static const int threshold = (int)(ZSKIPLIST_P * RAND_MAX);
    int level = 1;
    while (rand() < threshold)
        level += 1;
    return (level < ZSKIPLIST_MAXLEVEL) ? level : ZSKIPLIST_MAXLEVEL;
}

int main(void)
{
    int i;
    int counts[ZSKIPLIST_MAXLEVEL + 1];
    for (i = 0; i <= ZSKIPLIST_MAXLEVEL; i++)
        counts[i] = 0;

    srand(42);
    for (i = 0; i < 10000; i++) {
        int lvl = zslRandomLevel();
        counts[lvl]++;
    }

    /* Print level distribution for levels 1-8 */
    for (i = 1; i <= 8; i++) {
        printf("level %d: %d\n", i, counts[i]);
    }

    /* Also print 10 individual levels for direct comparison */
    srand(42);
    for (i = 0; i < 10; i++) {
        printf("%d\n", zslRandomLevel());
    }

    return 0;
}
