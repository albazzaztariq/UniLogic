from dataclasses import dataclass, field

def __ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class __ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)

@dataclass
class Vec2:
    x: int = 0
    y: int = 0


def vec_add(a, b):
    result = Vec2()
    result.x = (a.x + b.x)
    result.y = (a.y + b.y)
    return result

def main():
    v1 = Vec2()
    v1.x = 1
    v1.y = 2
    v2 = Vec2()
    v2.x = 3
    v2.y = 4
    v3 = vec_add(v1, v2)
    print(v3.x)
    print(v3.y)
    return 0


if __name__ == "__main__":
    main()
