#include <stdio.h>

int classify(int n) {
    if (n > 0) {
        return 1;
    } else if (n < 0) {
        return -1;
    } else {
        return 0;
    }
}

int factorial(int n) {
    int result = 1;
    int i = 1;
    while (i <= n) {
        result = result * i;
        i = i + 1;
    }
    return result;
}

int fibonacci(int n) {
    if (n <= 1) {
        return n;
    }
    int a = 0;
    int b = 1;
    int i = 2;
    while (i <= n) {
        int temp = a + b;
        a = b;
        b = temp;
        i = i + 1;
    }
    return b;
}

int sum_range(int lo, int hi) {
    int total = 0;
    int i = lo;
    while (i <= hi) {
        total = total + i;
        i = i + 1;
    }
    return total;
}

int power(int base, int exp) {
    int result = 1;
    int i = 0;
    while (i < exp) {
        result = result * base;
        i = i + 1;
    }
    return result;
}

int gcd(int a, int b) {
    while (b != 0) {
        int temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

int main() {
    printf("classify 5: %d\n", classify(5));
    printf("classify -3: %d\n", classify(-3));
    printf("classify 0: %d\n", classify(0));
    printf("fact 0: %d\n", factorial(0));
    printf("fact 5: %d\n", factorial(5));
    printf("fact 10: %d\n", factorial(10));
    printf("fib 0: %d\n", fibonacci(0));
    printf("fib 1: %d\n", fibonacci(1));
    printf("fib 10: %d\n", fibonacci(10));
    printf("fib 15: %d\n", fibonacci(15));
    printf("sum 1..10: %d\n", sum_range(1, 10));
    printf("sum 1..100: %d\n", sum_range(1, 100));
    printf("pow 2^10: %d\n", power(2, 10));
    printf("pow 3^5: %d\n", power(3, 5));
    printf("gcd 48 18: %d\n", gcd(48, 18));
    printf("gcd 100 75: %d\n", gcd(100, 75));
    return 0;
}
