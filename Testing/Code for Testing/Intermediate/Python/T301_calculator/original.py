"""Simple integer calculator with named operations."""


def add(a: int, b: int) -> int:
    return a + b


def subtract(a: int, b: int) -> int:
    return a - b


def multiply(a: int, b: int) -> int:
    return a * b


def safe_divide(a: int, b: int) -> int:
    if b == 0:
        return -1
    return a // b


def power(base: int, exp: int) -> int:
    result: int = 1
    i: int = 0
    while i < exp:
        result = result * base
        i = i + 1
    return result


def factorial(n: int) -> int:
    if n <= 1:
        return 1
    return n * factorial(n - 1)


def gcd(a: int, b: int) -> int:
    while b != 0:
        temp: int = b
        b = a % b
        a = temp
    return a


def main() -> int:
    print(add(10, 25))
    print(subtract(100, 37))
    print(multiply(6, 7))
    print(safe_divide(100, 3))
    print(safe_divide(10, 0))
    print(power(2, 10))
    print(factorial(10))
    print(gcd(48, 18))
    print(gcd(100, 75))

    # Compose operations
    result: int = add(multiply(3, 4), subtract(20, 8))
    print(result)
    return 0


if __name__ == "__main__":
    main()
