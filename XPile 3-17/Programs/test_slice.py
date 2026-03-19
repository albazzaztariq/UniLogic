from dataclasses import dataclass, field

def __ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class __ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)


def main():
    greeting = 'Hello World'
    hello = greeting[0:5]
    print(hello)
    world = greeting[6:11]
    print(world)
    nums = [10, 20, 30, 40, 50]
    middle = nums[1:4]
    print(middle[0])
    print(middle[1])
    print(middle[2])
    h = greeting[0:1]
    print(h)
    return 0


if __name__ == "__main__":
    main()
