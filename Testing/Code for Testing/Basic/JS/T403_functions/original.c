#include <stdio.h>

int is_prime(int n) {
    if (n < 2) {
        return 0;
    }
    int i = 2;
    while (i * i <= n) {
        if (n % i == 0) {
            return 0;
        }
        i = i + 1;
    }
    return 1;
}

int count_primes(int limit) {
    int count = 0;
    int n = 2;
    while (n <= limit) {
        if (is_prime(n)) {
            count = count + 1;
        }
        n = n + 1;
    }
    return count;
}

int collatz_length(int n) {
    int steps = 0;
    while (n != 1) {
        if (n % 2 == 0) {
            n = n / 2;
        } else {
            n = 3 * n + 1;
        }
        steps = steps + 1;
    }
    return steps;
}

int digit_sum(int n) {
    if (n < 0) {
        n = -n;
    }
    int total = 0;
    while (n > 0) {
        total = total + n % 10;
        n = n / 10;
    }
    return total;
}

int count_digits(int n) {
    if (n == 0) {
        return 1;
    }
    if (n < 0) {
        n = -n;
    }
    int count = 0;
    while (n > 0) {
        count = count + 1;
        n = n / 10;
    }
    return count;
}

int is_palindrome_number(int n) {
    if (n < 0) {
        return 0;
    }
    int original = n;
    int reversed = 0;
    while (n > 0) {
        reversed = reversed * 10 + n % 10;
        n = n / 10;
    }
    if (reversed == original) {
        return 1;
    }
    return 0;
}

int main() {
    printf("--- Primes ---\n");
    printf("is_prime 2: %d\n", is_prime(2));
    printf("is_prime 17: %d\n", is_prime(17));
    printf("is_prime 20: %d\n", is_prime(20));
    printf("is_prime 97: %d\n", is_prime(97));
    printf("count_primes 100: %d\n", count_primes(100));

    printf("--- Collatz ---\n");
    printf("collatz 1: %d\n", collatz_length(1));
    printf("collatz 6: %d\n", collatz_length(6));
    printf("collatz 27: %d\n", collatz_length(27));

    printf("--- Digits ---\n");
    printf("digit_sum 12345: %d\n", digit_sum(12345));
    printf("digit_sum -999: %d\n", digit_sum(-999));
    printf("count_digits 0: %d\n", count_digits(0));
    printf("count_digits 12345: %d\n", count_digits(12345));

    printf("--- Palindrome ---\n");
    printf("palindrome 121: %d\n", is_palindrome_number(121));
    printf("palindrome 12321: %d\n", is_palindrome_number(12321));
    printf("palindrome 123: %d\n", is_palindrome_number(123));
    printf("palindrome -121: %d\n", is_palindrome_number(-121));

    return 0;
}
