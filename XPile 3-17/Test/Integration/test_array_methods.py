from dataclasses import dataclass, field


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
    total = array_sum(nums, len(nums))
    print(total)
    found = array_contains(nums, len(nums), 4)
    print(found)
    nope = array_contains(nums, len(nums), 9)
    print(nope)
    return 0


if __name__ == "__main__":
    main()
