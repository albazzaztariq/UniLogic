from dataclasses import dataclass, field

def __ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class __ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)


def add(a, b):
    return (a + b)

def subtract(a, b):
    return (a - b)

def multiply(a, b):
    return (a * b)

def safe_divide(a, b):
    if (b == 0):
        return (-1)
    return (a // b)

def power(base, exp):
    result = 1
    i = 0
    while (i < exp):
        result = (result * base)
        i = (i + 1)
    return result

def factorial(n):
    if (n <= 1):
        return 1
    return (n * factorial((n - 1)))

def gcd(a, b):
    while (b != 0):
        temp = b
        b = (a % b)
        a = temp
    return a

def main():
    print(add(10, 25))
    print(subtract(100, 37))
    print(multiply(6, 7))
    print(safe_divide(100, 3))
    print(safe_divide(10, 0))
    print(power(2, 10))
    print(factorial(10))
    print(gcd(48, 18))
    print(gcd(100, 75))
    result = add(multiply(3, 4), subtract(20, 8))
    print(result)
    return 0


if __name__ == "__main__":
    main()
