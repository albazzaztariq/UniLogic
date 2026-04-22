#include <stdio.h>

int add(int a, int b) {
    return a + b;
}

int subtract(int a, int b) {
    return a - b;
}

int multiply(int a, int b) {
    return a * b;
}

int divide(int a, int b) {
    return a / b;
}

int modulo(int a, int b) {
    return a % b;
}

int negate(int x) {
    return -x;
}

int absolute(int x) {
    if (x < 0) {
        return -x;
    }
    return x;
}

int max_of(int a, int b) {
    if (a > b) {
        return a;
    }
    return b;
}

int min_of(int a, int b) {
    if (a < b) {
        return a;
    }
    return b;
}

int main() {
    printf("add: %d\n", add(10, 25));
    printf("sub: %d\n", subtract(100, 37));
    printf("mul: %d\n", multiply(6, 7));
    printf("div: %d\n", divide(100, 3));
    printf("mod: %d\n", modulo(17, 5));
    printf("neg: %d\n", negate(42));
    printf("abs1: %d\n", absolute(-15));
    printf("abs2: %d\n", absolute(20));
    printf("max: %d\n", max_of(30, 50));
    printf("min: %d\n", min_of(30, 50));
    printf("compound: %d\n", add(multiply(3, 4), subtract(20, 8)));
    return 0;
}
