from dataclasses import dataclass, field

def __ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class __ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)


def main():
    scores = [95, 87, 72, 100, 63]
    print(scores[0])
    print(scores[4])
    names = ['Alice', 'Bob', 'Carol']
    names.append('Dave')
    print(names[0])
    print(names[3])
    total = 0
    for s in scores:
        total += s
    print(total)
    for name in names:
        print(name)
    return 0


if __name__ == "__main__":
    main()
