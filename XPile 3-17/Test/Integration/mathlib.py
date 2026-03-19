from dataclasses import dataclass, field


def factorial(n):
    if (n <= 1):
        return 1
    return (n * factorial((n - 1)))

def square(x):
    return (x * x)

def add(a, b):
    return (a + b)


if __name__ == "__main__":
    main()
