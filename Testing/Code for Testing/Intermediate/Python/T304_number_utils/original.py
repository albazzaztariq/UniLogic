"""Number utilities — primes, fibonacci, digit operations."""


def is_prime(n: int) -> bool:
    if n < 2:
        return False
    if n == 2:
        return True
    if n % 2 == 0:
        return False
    i: int = 3
    while i * i <= n:
        if n % i == 0:
            return False
        i = i + 2
    return True


def fibonacci(n: int) -> int:
    if n <= 0:
        return 0
    if n == 1:
        return 1
    a: int = 0
    b: int = 1
    i: int = 2
    while i <= n:
        temp: int = a + b
        a = b
        b = temp
        i = i + 1
    return b


def count_digits(n: int) -> int:
    if n == 0:
        return 1
    count: int = 0
    val: int = n
    if val < 0:
        val = 0 - val
    while val > 0:
        val = val // 10
        count = count + 1
    return count


def sum_digits(n: int) -> int:
    total: int = 0
    val: int = n
    if val < 0:
        val = 0 - val
    while val > 0:
        total = total + val % 10
        val = val // 10
    return total


def reverse_number(n: int) -> int:
    result: int = 0
    val: int = n
    while val > 0:
        result = result * 10 + val % 10
        val = val // 10
    return result


def main() -> int:
    # Primes
    i: int = 2
    while i <= 30:
        if is_prime(i):
            print(i)
        i = i + 1

    # Fibonacci
    j: int = 0
    while j <= 10:
        print(fibonacci(j))
        j = j + 1

    # Digit operations
    print(count_digits(12345))
    print(count_digits(0))
    print(sum_digits(12345))
    print(sum_digits(9999))
    print(reverse_number(12345))
    print(reverse_number(100))

    return 0


if __name__ == "__main__":
    main()
