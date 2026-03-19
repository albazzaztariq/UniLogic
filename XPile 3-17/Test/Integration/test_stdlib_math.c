#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
static char* __ul_strcat(const char* a, const char* b) {
    int la = (int)strlen(a), lb = (int)strlen(b);
    char* r = (char*)malloc(la + lb + 1); memcpy(r, a, la); memcpy(r + la, b, lb); r[la+lb] = 0; return r; }

double math_sqrt(double x);
double math_pow(double base, double exp);
double math_abs(double x);
double math_floor(double x);
double math_ceil(double x);
double math_min(double a, double b);
double math_max(double a, double b);
double math_pi(void);
double math_e(void);
int main(void);

double math_sqrt(double x)
{
    return sqrt(x);
}

double math_pow(double base, double exp)
{
    return pow(base, exp);
}

double math_abs(double x)
{
    if ((x < 0.0)) {
        return (0.0 - x);
    }
    return x;
}

double math_floor(double x)
{
    return floor(x);
}

double math_ceil(double x)
{
    return ceil(x);
}

double math_min(double a, double b)
{
    if ((a < b)) {
        return a;
    }
    return b;
}

double math_max(double a, double b)
{
    if ((a > b)) {
        return a;
    }
    return b;
}

double math_pi(void)
{
    return 3.141592653589793;
}

double math_e(void)
{
    return 2.718281828459045;
}

int main(void)
{
    double s = math_sqrt(144.0);
    printf("%f\n", s);
    double p = math_pow(2.0, 10.0);
    printf("%f\n", p);
    double a = math_abs((0.0 - 42.5));
    printf("%f\n", a);
    double fl = math_floor(3.7);
    printf("%f\n", fl);
    double ce = math_ceil(3.2);
    printf("%f\n", ce);
    double mn = math_min(5.0, 3.0);
    printf("%f\n", mn);
    double mx = math_max(5.0, 3.0);
    printf("%f\n", mx);
    double pi = math_pi();
    printf("%f\n", pi);
    double e = math_e();
    printf("%f\n", e);
    return 0;
}


