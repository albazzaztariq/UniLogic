#define MAX 100
#define PI 3.14159
#define VERSION "1.0.0"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
static char* __ul_strcat(const char* a, const char* b) {
    int la = (int)strlen(a), lb = (int)strlen(b);
    char* r = (char*)malloc(la + lb + 1); memcpy(r, a, la); memcpy(r + la, b, lb); r[la+lb] = 0; return r; }

int main(void);

int main(void)
{
    printf("%d\n", MAX);
    printf("%f\n", PI);
    printf("%s\n", VERSION);
    return 0;
}


