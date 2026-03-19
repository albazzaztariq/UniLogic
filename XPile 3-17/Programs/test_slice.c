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
#include <stdlib.h>

int main(void);

int main(void)
{
    char* greeting = "Hello World";
    int _sl1 = (5) - (0);
    char* _slice0 = (char*)__ul_malloc(_sl1 + 1);
    memcpy(_slice0, greeting + (0), _sl1);
    _slice0[_sl1] = 0;
    char* hello = _slice0;
    printf("%s\n", hello);
    int _sl2 = (11) - (6);
    char* _slice1 = (char*)__ul_malloc(_sl2 + 1);
    memcpy(_slice1, greeting + (6), _sl2);
    _slice1[_sl2] = 0;
    char* world = _slice1;
    printf("%s\n", world);
    int nums[] = {10, 20, 30, 40, 50};
    int _sn3 = (4) - (1);
    int _slice2[_sn3];
    memcpy(_slice2, &nums[1], _sn3 * sizeof(int));
    int* middle = _slice2;
    printf("%d\n", middle[0]);
    printf("%d\n", middle[1]);
    printf("%d\n", middle[2]);
    int _sl4 = (1) - (0);
    char* _slice3 = (char*)__ul_malloc(_sl4 + 1);
    memcpy(_slice3, greeting + (0), _sl4);
    _slice3[_sl4] = 0;
    char* h = _slice3;
    printf("%s\n", h);
    return 0;
}


