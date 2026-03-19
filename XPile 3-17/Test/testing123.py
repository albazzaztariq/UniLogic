# @dr memory = gc

from dataclasses import dataclass, field

def add(a, b):
    return (a + b)

def is_even(n):
    return ((n % 2) == 0)

def clamp(val, lo, hi):
    if (val < lo):
        return lo
    if (val > hi):
        return hi
    return val

def main():
    print(add(2, 3))
    return 0


if __name__ == "__main__":
    main()
