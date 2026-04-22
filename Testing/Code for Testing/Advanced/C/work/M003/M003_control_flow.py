from dataclasses import dataclass, field

def __ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class __ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)


def classify(score):
    if (score > 90):
        return 'A'
    else:
        if (score > 80):
            return 'B'
        else:
            if (score > 70):
                return 'C'
            else:
                return 'F'

def main():
    print(classify(95))
    print(classify(85))
    print(classify(72))
    print(classify(50))
    count = 0
    while (count < 5):
        count += 1
    print(count)
    nums = [10, 20, 30, 40, 50]
    total = 0
    for n in nums:
        total += n
    print(total)
    code = 2
    match code:
        case 1:
            print('one')
        case 2:
            print('two')
        case 3:
            print('three')
        case _:
            print('other')
    i = 0
    while (i < 100):
        if (i == 3):
            break
        i += 1
    print(i)
    return 0


if __name__ == "__main__":
    main()
