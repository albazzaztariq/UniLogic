from dataclasses import dataclass, field


def main():
    squares = [(x * x) for x in range(1, 6)]
    print(squares[0])
    print(squares[4])
    return 0


if __name__ == "__main__":
    main()
