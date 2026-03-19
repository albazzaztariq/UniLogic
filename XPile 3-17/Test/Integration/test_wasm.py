from dataclasses import dataclass, field


def add(a, b):
    return (a + b)

def main():
    x = add(10, 32)
    print(x)
    y = (x * 2)
    print(y)
    return 0


if __name__ == "__main__":
    main()
