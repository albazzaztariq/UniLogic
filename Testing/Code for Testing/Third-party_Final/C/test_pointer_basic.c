#include <stdio.h>

void swap(int *a, int *b) {
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

void increment(int *val) {
    *val = *val + 1;
}

int sum_array(int *arr, int n) {
    int total = 0;
    int i;
    for (i = 0; i < n; i++) {
        total = total + arr[i];
    }
    return total;
}

int main() {
    int x = 10;
    int y = 20;
    printf("before swap: x=%d y=%d\n", x, y);
    swap(&x, &y);
    printf("after swap: x=%d y=%d\n", x, y);

    int val = 5;
    increment(&val);
    printf("after increment: val=%d\n", val);

    int nums[5] = {1, 2, 3, 4, 5};
    int s = sum_array(nums, 5);
    printf("sum: %d\n", s);

    int *ptr = &x;
    printf("ptr deref: %d\n", *ptr);
    *ptr = 99;
    printf("after deref assign: x=%d\n", x);

    return 0;
}
