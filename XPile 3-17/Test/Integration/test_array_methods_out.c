#include <stdio.h>
#include <stdint.h>
#include <string.h>

int array_sum(int* arr, int n);
int array_contains(int* arr, int n, int val);
int main(void);

int array_sum(int* arr, int n)
{
    int total = 0;
    int i = 0;
    while ((i < n)) {
        total = (total + arr[i]);
        i = (i + 1);
    }
    return total;
}

int array_contains(int* arr, int n, int val)
{
    int i = 0;
    while ((i < n)) {
        if ((arr[i] == val)) {
            return 1;
        }
        i = (i + 1);
    }
    return 0;
}

int main(void)
{
    int nums[] = {3, 1, 4, 1, 5};
    int total = array_sum(nums, 5);
    printf("%d\n", total);
    int found = array_contains(nums, 5, 4);
    printf("%d\n", found);
    int nope = array_contains(nums, 5, 9);
    printf("%d\n", nope);
    return 0;
}


