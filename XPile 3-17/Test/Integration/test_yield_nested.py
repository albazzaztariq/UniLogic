from dataclasses import dataclass, field


def mul_table(rows, cols):
    r = 1
    while (r <= rows):
        c = 1
        while (c <= cols):
            yield (r * c)
            c += 1
        r += 1

def triangle(n):
    row = 1
    while (row <= n):
        col = 1
        while (col <= row):
            yield col
            col += 1
        row += 1

def main():
    for v in mul_table(2, 3):
        print(v)
    for t in triangle(3):
        print(t)
    return 0


if __name__ == "__main__":
    main()
