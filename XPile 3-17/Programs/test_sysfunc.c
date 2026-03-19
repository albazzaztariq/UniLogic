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
#include <time.h>
#ifdef _WIN32
#include <windows.h>
static void __ul_sleep_ms(int ms) { Sleep(ms); }
#else
#include <unistd.h>
static void __ul_sleep_ms(int ms) { usleep(ms * 1000); }
#endif
static long long __ul_clock_ns(void) {
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    return (long long)ts.tv_sec * 1000000000LL + ts.tv_nsec;
}
#include <stdlib.h>

int main(void);

int main(void)
{
    int t = ((int)time(NULL));
    if ((t > 0)) {
        printf("%d\n", 1);
    }
    int c = __ul_clock_ns();
    if ((c > 0)) {
        printf("%d\n", 1);
    }
    srand(42);
    int r1 = (1 + rand() % (100 - 1 + 1));
    if (((r1 >= 1) && (r1 <= 100))) {
        printf("%d\n", 1);
    }
    srand(42);
    float f = ((double)rand() / (double)RAND_MAX);
    if (((f >= 0.0) && (f < 1.0))) {
        printf("%d\n", 1);
    }
    char* path = getenv("PATH");
    if ((sizeof(path) > 0)) {
        printf("%d\n", 1);
    }
    return 0;
}


