#include <stdio.h>

int* find_max(int *arr, int n) {
    int *max_ptr = &arr[0];
    int i;
    for (i = 1; i < n; i++) {
        if (arr[i] > *max_ptr) {
            max_ptr = &arr[i];
        }
    }
    return max_ptr;
}

void double_value(int *p) {
    *p = *p * 2;
}

int main() {
    int nums[5] = {3, 7, 2, 9, 4};
    int *biggest = find_max(nums, 5);
    printf("max = %d\n", *biggest);
    double_value(biggest);
    printf("after double = %d\n", *biggest);
    printf("in array = %d\n", nums[3]);
    return 0;
}
