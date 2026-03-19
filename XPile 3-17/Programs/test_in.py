from dataclasses import dataclass, field

def __ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class __ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)


def main():
    msg = 'hello world'
    if ('world' in msg):
        print(1)
    if ('xyz' in msg):
        print(0)
    else:
        print(1)
    nums = [10, 20, 30, 40, 50]
    if (30 in nums):
        print(1)
    if (99 in nums):
        print(0)
    else:
        print(1)
    return 0


if __name__ == "__main__":
    main()
