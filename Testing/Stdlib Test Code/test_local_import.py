from dataclasses import dataclass, field

def __ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class __ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)


def factorial(n):
    if (n <= 1):
        return 1
    return (n * factorial((n - 1)))

def square(x):
    return (x * x)

def add(a, b):
    return (a + b)

def main():
    f = factorial(5)
    print(f)
    s = square(7)
    print(s)
    sum = add(10, 20)
    print(sum)
    composed = square(add(3, 4))
    print(composed)
    return 0


if __name__ == "__main__":
    main()
