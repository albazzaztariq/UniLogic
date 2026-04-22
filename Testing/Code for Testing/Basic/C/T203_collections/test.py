from dataclasses import dataclass, field

def _ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class _ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)


def main():
    nums = [10, 20, 30, 40, 50]
    print(nums[0])
    print(nums[4])
    sum = 0
    for n in nums:
        sum = (sum + n)
    print(sum)
    scores = []
    scores.append(100)
    scores.append(200)
    scores.append(300)
    print(len(scores))
    print(scores[0])
    print(scores[2])
    scores.insert(1, 150)
    print(scores[1])
    scores.remove(150)
    print(len(scores))
    dropped = scores.pop(0)
    print(dropped)
    items = [5, 10, 15]
    item_sum = 0
    for it in items:
        item_sum = (item_sum + it)
    print(item_sum)
    sliced = nums[1:4]
    print(sliced[0])
    print(sliced[1])
    print(sliced[2])
    if (30 in nums):
        print('found')
    return 0


if __name__ == "__main__":
    main()
