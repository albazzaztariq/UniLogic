from dataclasses import dataclass, field


def main():
    x = 10
    # @norm 2
    y = (x + 5)
    # @norm 0
    z = (y * 2)
    print(z)
    return 0


if __name__ == "__main__":
    main()
