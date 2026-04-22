from dataclasses import dataclass, field

def _ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class _ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)


def main():
    nums = [10, 20, 30, 40, 50]
    if (30 in nums):
        print('found 30')
    if (99 in nums):
        print('should not print')
    fruits = ['apple', 'banana', 'mango']
    if ('banana' in fruits):
        print('found banana')
    if ('grape' in fruits):
        print('should not print')
    return 0


if __name__ == "__main__":
    main()
