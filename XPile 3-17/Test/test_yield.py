from dataclasses import dataclass, field

def countdown(n):
    while (n > 0):
        yield n
        n -= 1

def doubles(n):
    i = 1
    while (i <= n):
        yield (i * 2)
        i += 1

def main():
    for val in countdown(5):
        print(val)
    for d in doubles(4):
        print(d)


if __name__ == "__main__":
    main()
