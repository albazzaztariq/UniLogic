from dataclasses import dataclass, field

def __ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class __ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)


def main():
    scores = []
    scores.append(10)
    scores.append(20)
    scores.append(30)
    print(len(scores))
    print(scores[0])
    print(scores[1])
    print(scores[2])
    scores.insert(1, 15)
    print(scores[1])
    print(len(scores))
    scores.remove(15)
    print(len(scores))
    dropped = scores.pop(0)
    print(dropped)
    print(len(scores))
    nums = [5, 3, 1, 4, 2]
    print(len(nums))
    nums.sort()
    print(nums[0])
    print(nums[4])
    nums.reverse()
    print(nums[0])
    print(nums[4])
    if (3 in nums):
        print('found 3')
    nums.clear()
    print(len(nums))
    items = [100, 200, 300]
    total = 0
    for x in items:
        total = (total + x)
    print(total)
    return 0


if __name__ == "__main__":
    main()
