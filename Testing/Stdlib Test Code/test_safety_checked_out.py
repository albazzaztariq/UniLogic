# @dr safety = checked

from dataclasses import dataclass, field


def main():
    arr = [1, 2, 3]
    print(arr[1])
    print(arr[10])
    return 0


if __name__ == "__main__":
    main()
