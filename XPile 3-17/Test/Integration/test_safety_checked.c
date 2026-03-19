// @dr safety = checked

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
static char* __ul_strcat(const char* a, const char* b) {
    int la = (int)strlen(a), lb = (int)strlen(b);
    char* r = (char*)malloc(la + lb + 1); memcpy(r, a, la); memcpy(r + la, b, lb); r[la+lb] = 0; return r; }
#include <stdlib.h>

#define __ul_checked_access(arr, idx, size, name, line) \
    ((unsigned)(idx) < (unsigned)(size) ? (arr)[(idx)] : \
    (fprintf(stderr, "bounds check error: %s[%d] out of range (size %d) at line %d\n", \
        (name), (int)(idx), (int)(size), (line)), exit(1), (arr)[0]))

int main(void);

int main(void)
{
    int arr[] = {1, 2, 3};
    printf("%d\n", __ul_checked_access(arr, 1, 3, "arr", 7));
    printf("%d\n", __ul_checked_access(arr, 10, 3, "arr", 8));
    return 0;
}


