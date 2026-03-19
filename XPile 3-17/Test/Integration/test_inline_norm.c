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
    int x = 10;
    /* @norm 2 */
    int y = (x + 5);
    /* @norm 0 */
    int z = (y * 2);
    printf("%d\n", z);
    return 0;
}


