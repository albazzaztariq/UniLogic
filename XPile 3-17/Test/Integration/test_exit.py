from dataclasses import dataclass, field
import sys


def check(val):
    if (val < 0):
        print((-1))
        sys.exit(1)
    return (val * 2)

def main():
    result = check(5)
    print(result)
    sys.exit(0)
    return 0


if __name__ == "__main__":
    main()
