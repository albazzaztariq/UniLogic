from dataclasses import dataclass, field


def add(a, b):
    return (a + b)

def main():
    result = add(10, 20)
    print(result)
    print(add(3, 4))
    return 0


if __name__ == "__main__":
    main()
