from dataclasses import dataclass, field

def _ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class _ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)


def main():
    count = 0
    while True:
        print(count)
        count += 1
        if not ((count < 5)):
            break
    x = 100
    while True:
        print(x)
        x += 1
        if not ((x < 0)):
            break
    return 0


if __name__ == "__main__":
    main()
