from dataclasses import dataclass, field


def minmax(a, b):
    if (a < b):
        return (a, b)
    return (b, a)

def main():
    lo, hi = minmax(10, 3)
    print(lo)
    print(hi)
    return 0


if __name__ == "__main__":
    main()
