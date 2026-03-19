#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
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

double math_sqrt(double x);
double math_floor(double x);
void sort_doubles(double* arr, int n);
double stat_mean(double* arr, int n);
double quantile_sorted(double* sorted, int n, double p);
double stat_quantile(double* arr, int n, double p);
double stat_median(double* arr, int n);
double stat_mode(double* arr, int n);
double stat_variance(double* arr, int n);
double stat_stddev(double* arr, int n);
int main(void);

double math_sqrt(double x)
{
    return sqrt(x);
}

double math_floor(double x)
{
    return floor(x);
}

void sort_doubles(double* arr, int n)
{
    for (int i = 1; i < n; i++) {
        double key = arr[i];
        int j = (i - 1);
        while ((j >= 0)) {
            if ((arr[j] > key)) {
                arr[(j + 1)] = arr[j];
                j = (j - 1);
            } else {
                break;
            }
        }
        arr[(j + 1)] = key;
    }
}

double stat_mean(double* arr, int n)
{
    double sum = 0.0;
    for (int i = 0; i < n; i++) {
        sum = (sum + arr[i]);
    }
    return (sum / ((double)n));
}

double quantile_sorted(double* sorted, int n, double p)
{
    if ((p == 0.0)) {
        return sorted[0];
    }
    if ((p == 1.0)) {
        return sorted[(n - 1)];
    }
    double idx = (((double)(n - 1)) * p);
    double fl = math_floor(idx);
    int lo = ((int)fl);
    double fraction = (idx - fl);
    if ((fraction < 0.0001)) {
        return sorted[lo];
    }
    return (sorted[lo] + (fraction * (sorted[(lo + 1)] - sorted[lo])));
}

double stat_quantile(double* arr, int n, double p)
{
    double sorted[] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    for (int i = 0; i < n; i++) {
        sorted[i] = arr[i];
    }
    sort_doubles(sorted, n);
    return quantile_sorted(sorted, n, p);
}

double stat_median(double* arr, int n)
{
    return stat_quantile(arr, n, 0.5);
}

double stat_mode(double* arr, int n)
{
    double sorted[] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    for (int i = 0; i < n; i++) {
        sorted[i] = arr[i];
    }
    sort_doubles(sorted, n);
    double max_val = sorted[0];
    int max_count = 0;
    int count = 0;
    double last_val = sorted[0];
    for (int i = 0; i < n; i++) {
        if ((sorted[i] == last_val)) {
            count = (count + 1);
        } else {
            count = 1;
            last_val = sorted[i];
        }
        if ((count > max_count)) {
            max_count = count;
            max_val = sorted[i];
        }
    }
    return max_val;
}

double stat_variance(double* arr, int n)
{
    double m = stat_mean(arr, n);
    double sum_sq = 0.0;
    for (int i = 0; i < n; i++) {
        double dev = (arr[i] - m);
        sum_sq = (sum_sq + (dev * dev));
    }
    return (sum_sq / ((double)n));
}

double stat_stddev(double* arr, int n)
{
    if ((n == 1)) {
        return 0.0;
    }
    double v = stat_variance(arr, n);
    return math_sqrt(v);
}

int main(void)
{
    printf("%s\n", "=== Dataset 1: [2, 4, 4, 4, 5, 5, 7, 9] ===");
    double d1[] = {2.0, 4.0, 4.0, 4.0, 5.0, 5.0, 7.0, 9.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    int n1 = 8;
    printf("%s\n", "mean:");
    printf("%f\n", stat_mean(d1, n1));
    printf("%s\n", "median:");
    printf("%f\n", stat_median(d1, n1));
    printf("%s\n", "mode:");
    printf("%f\n", stat_mode(d1, n1));
    printf("%s\n", "variance:");
    printf("%f\n", stat_variance(d1, n1));
    printf("%s\n", "stddev:");
    printf("%f\n", stat_stddev(d1, n1));
    printf("%s\n", "");
    printf("%s\n", "=== Dataset 2: [1, 2, 3, 4, 5] ===");
    double d2[] = {1.0, 2.0, 3.0, 4.0, 5.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    int n2 = 5;
    printf("%s\n", "mean:");
    printf("%f\n", stat_mean(d2, n2));
    printf("%s\n", "median:");
    printf("%f\n", stat_median(d2, n2));
    printf("%s\n", "mode:");
    printf("%f\n", stat_mode(d2, n2));
    printf("%s\n", "variance:");
    printf("%f\n", stat_variance(d2, n2));
    printf("%s\n", "stddev:");
    printf("%f\n", stat_stddev(d2, n2));
    printf("%s\n", "");
    printf("%s\n", "=== Quantile tests ===");
    double d3[] = {3.0, 6.0, 7.0, 8.0, 8.0, 9.0, 10.0, 13.0, 15.0, 16.0, 20.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    int n3 = 11;
    printf("%s\n", "Q(0.25):");
    printf("%f\n", stat_quantile(d3, n3, 0.25));
    printf("%s\n", "Q(0.5):");
    printf("%f\n", stat_quantile(d3, n3, 0.5));
    printf("%s\n", "Q(0.75):");
    printf("%f\n", stat_quantile(d3, n3, 0.75));
    printf("%s\n", "Q(0.0):");
    printf("%f\n", stat_quantile(d3, n3, 0.0));
    printf("%s\n", "Q(1.0):");
    printf("%f\n", stat_quantile(d3, n3, 1.0));
    return 0;
}


