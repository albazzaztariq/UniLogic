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
    buf = []
    buf[0] = 100
    buf[1] = 200
    buf[2] = 300
    print(buf[0])
    print(buf[2])
    vals = [1, 2, 3]
    print(vals[0])
    print(vals[2])
    return 0


if __name__ == "__main__":
    main()
