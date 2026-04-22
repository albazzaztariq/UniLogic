from dataclasses import dataclass, field

def __ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class __ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)


def double_it(x):
    return (x * 2)

def is_even(x):
    return ((x % 2) == 0)

def main():
    print((10 + 5))
    print((10 - 3))
    print((6 * 7))
    print((20 // 4))
    print((17 % 5))
    neg = (0 - 42)
    print(neg)
    print((7 // 2))
    result = (2 + (3 * 4))
    print(result)
    nums = [1, 2, 3, 4, 5]
    doubled = list(map(double_it, nums))
    for d in doubled:
        print(d)
    evens = list(filter(is_even, nums))
    for e in evens:
        print(e)
    unsorted = [5, 2, 8, 1, 9]
    unsorted.sort()
    for s in unsorted:
        print(s)
    return 0


if __name__ == "__main__":
    main()
