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
    nums = [1, 2, 3, 4, 5]
    doubled = list(map(double_it, nums))
    for d in doubled:
        print(d)
    evens = list(filter(is_even, nums))
    for e in evens:
        print(e)
    return 0


if __name__ == "__main__":
    main()
