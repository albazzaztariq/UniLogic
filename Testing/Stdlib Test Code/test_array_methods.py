from dataclasses import dataclass, field

def __ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class __ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)


def array_sum(arr, n):
    total = 0
    i = 0
    while (i < n):
        total = (total + arr[i])
        i = (i + 1)
    return total

def array_contains(arr, n, val):
    i = 0
    while (i < n):
        if (arr[i] == val):
            return True
        i = (i + 1)
    return False

def main():
    nums = [3, 1, 4, 1, 5]
    total = sum(nums)
    print(total)
    found = (4 in nums)
    print(found)
    nope = (9 in nums)
    print(nope)
    return 0


if __name__ == "__main__":
    main()
