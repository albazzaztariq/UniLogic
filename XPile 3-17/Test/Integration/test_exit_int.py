from dataclasses import dataclass, field


def validate(x):
    if (x < 0):
        return (-1)
    return x

def main():
    val = validate(10)
    if (val < 0):
        print(0)
        return 1
    print(val)
    return 0


if __name__ == "__main__":
    main()
