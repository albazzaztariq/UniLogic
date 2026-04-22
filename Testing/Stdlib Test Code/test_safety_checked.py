# @dr safety = checked

from dataclasses import dataclass, field

def __ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class __ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)


def main():
    arr = [1, 2, 3]
    print(arr[1])
    print(arr[10])
    return 0


if __name__ == "__main__":
    main()
