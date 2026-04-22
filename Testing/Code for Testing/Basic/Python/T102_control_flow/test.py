from dataclasses import dataclass, field

def _ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class _ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)


def main():
    x = 15
    if (x > 10):
        print('greater')
    else:
        print('not greater')
    val = 50
    if (val > 100):
        print('big')
    else:
        if (val > 25):
            print('medium')
        else:
            print('small')
    code = 2
    match code:
        case 1:
            print('one')
        case 2:
            print('two')
        case 3:
            print('three')
    i = 0
    total = 0
    while (i < 5):
        total = (total + i)
        i = (i + 1)
    print(total)
    sum = 0
    j = 1
    while (j <= 5):
        sum = (sum + j)
        j += 1
    print(sum)
    nums = [10, 20, 30]
    arr_sum = 0
    for n in nums:
        arr_sum = (arr_sum + n)
    print(arr_sum)
    count = 0
    while True:
        count = (count + 1)
        if not ((count < 3)):
            break
    print(count)
    last = 0
    k = 0
    while (k < 100):
        if (k == 5):
            last = k
            break
        k = (k + 1)
    print(last)
    return 0


if __name__ == "__main__":
    main()
