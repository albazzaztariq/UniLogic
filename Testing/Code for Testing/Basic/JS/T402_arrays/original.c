#include <stdio.h>

void print_array(int arr[], int count) {
    int i = 0;
    while (i < count) {
        if (i > 0) {
            printf(" ");
        }
        printf("%d", arr[i]);
        i = i + 1;
    }
    printf("\n");
}

int array_sum(int arr[], int count) {
    int total = 0;
    int i = 0;
    while (i < count) {
        total = total + arr[i];
        i = i + 1;
    }
    return total;
}

int array_max(int arr[], int count) {
    int result = arr[0];
    int i = 1;
    while (i < count) {
        if (arr[i] > result) {
            result = arr[i];
        }
        i = i + 1;
    }
    return result;
}

int array_min(int arr[], int count) {
    int result = arr[0];
    int i = 1;
    while (i < count) {
        if (arr[i] < result) {
            result = arr[i];
        }
        i = i + 1;
    }
    return result;
}

int count_above(int arr[], int count, int threshold) {
    int result = 0;
    int i = 0;
    while (i < count) {
        if (arr[i] > threshold) {
            result = result + 1;
        }
        i = i + 1;
    }
    return result;
}

void reverse_array(int arr[], int count) {
    int lo = 0;
    int hi = count - 1;
    while (lo < hi) {
        int temp = arr[lo];
        arr[lo] = arr[hi];
        arr[hi] = temp;
        lo = lo + 1;
        hi = hi - 1;
    }
}

void bubble_sort(int arr[], int count) {
    int i = 0;
    while (i < count - 1) {
        int j = 0;
        while (j < count - 1 - i) {
            if (arr[j] > arr[j + 1]) {
                int temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
            j = j + 1;
        }
        i = i + 1;
    }
}

int main() {
    int data[] = {64, 34, 25, 12, 22, 11, 90, 45};
    int count = 8;

    printf("original: ");
    print_array(data, count);
    printf("sum: %d\n", array_sum(data, count));
    printf("max: %d\n", array_max(data, count));
    printf("min: %d\n", array_min(data, count));
    printf("above 30: %d\n", count_above(data, count, 30));

    reverse_array(data, count);
    printf("reversed: ");
    print_array(data, count);

    bubble_sort(data, count);
    printf("sorted: ");
    print_array(data, count);
    printf("sum after sort: %d\n", array_sum(data, count));

    return 0;
}
