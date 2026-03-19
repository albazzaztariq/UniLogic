from dataclasses import dataclass, field


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
