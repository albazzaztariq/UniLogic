from dataclasses import dataclass, field

def add(a, b):
    return (a + b)

def factorial(n):
    if (n <= 1):
        return 1
    return (n * factorial((n - 1)))

def is_even(n):
    return ((n % 2) == 0)

def clamp(val, lo, hi):
    if (val < lo):
        return lo
    if (val > hi):
        return hi
    return val

def main():
    x = add(3, 7)
    print(x)
    limit = 100
    i = 0
    while (i < 5):
        print(i)
        i += 1
    if is_even(x):
        print(1)
    else:
        print(0)
    ratio = float(x)
    print(ratio)
    clamped = clamp(x, 0, 8)
    print(clamped)
    fact = factorial(5)
    print(fact)
    nums = [10, 20, 30, 40, 50]
    for n in nums:
        print(n)
    return 0


if __name__ == "__main__":
    main()
