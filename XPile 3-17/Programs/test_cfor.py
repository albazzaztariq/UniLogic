from dataclasses import dataclass, field

def __ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class __ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)


def main():
    sum1 = 0
    i = 0
    while (i < 5):
        sum1 = (sum1 + i)
        i += 1
    print(sum1)
    sum2 = 0
    j = 1
    while (j <= 5):
        sum2 = (sum2 + j)
        j += 1
    print(sum2)
    last = 0
    k = 0
    while (k < 100):
        if (k == 7):
            last = k
            break
        k += 1
    print(last)
    return 0


if __name__ == "__main__":
    main()
