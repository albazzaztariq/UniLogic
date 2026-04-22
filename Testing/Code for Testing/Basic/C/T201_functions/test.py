from dataclasses import dataclass, field

def _ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class _ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)


def add(a, b):
    return (a + b)

def greet(who):
    return ('Hello ' + who)

def square(n):
    return (n * n)

def is_positive(n):
    if (n > 0):
        return True
    return False

def factorial(n):
    if (n <= 1):
        return 1
    return (n * factorial((n - 1)))

def no_return():
    print('side effect')

def main():
    print(add(3, 7))
    print(greet('World'))
    print(square(6))
    print(is_positive(5))
    print(is_positive((-3)))
    print(factorial(5))
    no_return()
    print(add(square(2), square(3)))
    return 0


if __name__ == "__main__":
    main()
