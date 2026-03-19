# @dr safety = checked

from dataclasses import dataclass, field
import sys

def __ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class __ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)


def main():
    index = 3
    length = 10
    if not ((index < length)):
        print(f"killswitch failed at line 9", file=sys.stderr)
        sys.exit(1)
    print('test1: passed')
    if not ((length > 0)):
        print(f"killswitch failed at line 13", file=sys.stderr)
        sys.exit(1)
    print('test2: passed')
    x = 5
    if not ((x == 5)):
        print(f"killswitch failed at line 19", file=sys.stderr)
        sys.exit(1)
    print('test3: passed')
    return 0


if __name__ == "__main__":
    main()
