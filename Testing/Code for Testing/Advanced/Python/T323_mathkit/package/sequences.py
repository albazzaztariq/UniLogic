"""Integer sequence generators and checks."""


def fibonacci(n: int, result: list) -> None:
    """Fill result with first n Fibonacci numbers."""
    if n == 0:
        return
    result[0] = 0
    if n == 1:
        return
    result[1] = 1
    i: int = 2
    while i < n:
        result[i] = result[i - 1] + result[i - 2]
        i = i + 1


def is_prime(n: int) -> bool:
    if n < 2:
        return False
    if n == 2:
        return True
    if n % 2 == 0:
        return False
    d: int = 3
    while d * d <= n:
        if n % d == 0:
            return False
        d = d + 2
    return True


def count_primes(limit: int) -> int:
    count: int = 0
    n: int = 2
    while n <= limit:
        if is_prime(n):
            count = count + 1
        n = n + 1
    return count


def gcd(a: int, b: int) -> int:
    while b != 0:
        temp: int = b
        b = a % b
        a = temp
    return a


def lcm(a: int, b: int) -> int:
    return (a * b) // gcd(a, b)


def factorial(n: int) -> int:
    result: int = 1
    i: int = 2
    while i <= n:
        result = result * i
        i = i + 1
    return result


def power(base: int, exp: int) -> int:
    result: int = 1
    i: int = 0
    while i < exp:
        result = result * base
        i = i + 1
    return result


def triangular(n: int) -> int:
    """Return the nth triangular number."""
    return n * (n + 1) // 2


def sum_of_digits(n: int) -> int:
    total: int = 0
    val: int = n
    if val < 0:
        val = -val
    while val > 0:
        total = total + val % 10
        val = val // 10
    return total
