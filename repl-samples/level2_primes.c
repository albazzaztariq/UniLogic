/* Prime Checker
   Tests numbers for primality and finds primes in a range
   Concepts: functions, loops, conditionals */

#include <stdio.h>
#include <string.h>

/* Check if a number is prime */
int is_prime(int n) {
    if (n < 2) return 0;
    if (n == 2) return 1;
    if (n % 2 == 0) return 0;
    /* Only check odd divisors up to sqrt(n) */
    int i = 3;
    while (i * i <= n) {
        if (n % i == 0) return 0;
        i += 2;
    }
    return 1;
}

/* Count how many primes exist below a limit */
int count_primes(int limit) {
    int count = 0;
    for (int num = 2; num < limit; num++) {
        if (is_prime(num)) {
            count++;
        }
    }
    return count;
}

int main(void) {
    printf("=== Prime Checker ===\n");
    printf("\n");

    /* Test specific numbers */
    int test_nums[] = {1, 2, 7, 15, 23, 42, 97};
    int test_count = 7;
    for (int i = 0; i < test_count; i++) {
        int n = test_nums[i];
        if (is_prime(n)) {
            printf("%d is prime\n", n);
        } else {
            printf("%d is not prime\n", n);
        }
    }

    printf("\n");

    /* Find all primes below 50 */
    printf("Primes below 50:\n");
    char line[256] = "";
    for (int n = 2; n < 50; n++) {
        if (is_prime(n)) {
            if (strlen(line) > 0) {
                strcat(line, " ");
            }
            char num_str[16];
            sprintf(num_str, "%d", n);
            strcat(line, num_str);
        }
    }
    printf("%s\n", line);

    printf("\n");
    int total = count_primes(100);
    printf("Total primes below 100: %d\n", total);

    return 0;
}
