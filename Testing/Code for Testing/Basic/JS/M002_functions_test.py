from dataclasses import dataclass, field

def __ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class __ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)


def add(a, b):
    return (a + b)

def multiply(a, b):
    return (a * b)

def factorial(n):
    if (n <= 1):
        return 1
    return (n * factorial((n - 1)))

def greet(name):
    print(('Hello, ' + name))

def main():
    print(add(3, 7))
    print(multiply(6, 8))
    print(factorial(5))
    print(factorial(10))
    greet('Tariq')
    print(add(multiply(3, 4), 5))
    return 0


if __name__ == "__main__":
    main()
