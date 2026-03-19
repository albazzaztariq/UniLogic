#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
static char* __ul_strcat(const char* a, const char* b) {
    int la = (int)strlen(a), lb = (int)strlen(b);
    char* r = (char*)malloc(la + lb + 1); memcpy(r, a, la); memcpy(r + la, b, lb); r[la+lb] = 0; return r; }

int main(void);

int main(void)
{
    double s = sqrt(625.0);
    printf("%f\n", s);
    double p = pow(2.0, 10.0);
    printf("%f\n", p);
    double m = fmod(17.0, 5.0);
    printf("%f\n", m);
    double chained = sqrt(pow(3.0, 4.0));
    printf("%f\n", chained);
    return 0;
}


