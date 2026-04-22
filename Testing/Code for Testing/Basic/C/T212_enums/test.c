#define MAX_ITEMS 50
#define DOUBLED (MAX_ITEMS * 2)
#define VERSION "1.0"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#define __ul_malloc(size) malloc(size)
#define __ul_free(ptr)    free(ptr)
static char* __ul_strcat(const char* a, const char* b) {
    int la = (int)strlen(a), lb = (int)strlen(b);
    char* r = (char*)__ul_malloc(la + lb + 1); memcpy(r, a, la); memcpy(r + la, b, lb); r[la+lb] = 0; return r; }
static int __ul_cmp_int(const void* a, const void* b) { return (*(int*)a - *(int*)b); }
static int __ul_cmp_float(const void* a, const void* b) { float fa=*(float*)a, fb=*(float*)b; return (fa>fb)-(fa<fb); }
static int __ul_cmp_double(const void* a, const void* b) { double da=*(double*)a, db=*(double*)b; return (da>db)-(da<db); }
static int __ul_cmp_string(const void* a, const void* b) { return strcmp(*(char**)a, *(char**)b); }

enum Status { Status_pending = 0, Status_active = 1, Status_closed = 2 };

enum Level { Level_low = 10, Level_medium = 20, Level_high = 30 };

int counter = 0;
char* prefix = "item_";
int increment_counter(void);
int main(void);

int increment_counter(void)
{
    counter = (counter + 1);
    return counter;
}

int main(void)
{
    printf("%d\n", MAX_ITEMS);
    printf("%d\n", DOUBLED);
    printf("%s\n", VERSION);
    printf("%d\n", counter);
    printf("%s\n", prefix);
    printf("%d\n", increment_counter());
    printf("%d\n", increment_counter());
    printf("%d\n", increment_counter());
    int s = Status_active;
    printf("%d\n", s);
    int total = ((Status_pending + Status_active) + Status_closed);
    printf("%d\n", total);
    if ((s == Status_active)) {
        printf("%s\n", "is active");
    }
    int lev = Level_medium;
    if (lev == 10) {
        printf("%s\n", "low");
    } else if (lev == 20) {
        printf("%s\n", "medium");
    } else if (lev == 30) {
        printf("%s\n", "high");
    }
    return 0;
}


