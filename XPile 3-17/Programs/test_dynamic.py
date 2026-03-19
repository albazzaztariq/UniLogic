from dataclasses import dataclass, field

def __ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class __ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)


def main():
    nums = []
    nums.append(10)
    nums.append(20)
    nums.append(30)
    print(len(nums))
    print(nums[0])
    print(nums[2])
    popped = nums.pop()
    print(popped)
    print(len(nums))
    return 0


if __name__ == "__main__":
    main()
