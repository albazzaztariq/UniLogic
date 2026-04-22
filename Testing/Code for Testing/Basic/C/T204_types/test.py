from dataclasses import dataclass, field

def _ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class _ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)

@dataclass
class Point:
    x: int = 0
    y: int = 0

@dataclass
class Rectangle:
    top_left: Point = field(default_factory=Point)
    bottom_right: Point = field(default_factory=Point)


def make_point(px, py):
    p = Point()
    p.x = px
    p.y = py
    return p

def distance_sq(a, b):
    dx = (b.x - a.x)
    dy = (b.y - a.y)
    return ((dx * dx) + (dy * dy))

def main():
    p1 = Point()
    p1.x = 3
    p1.y = 4
    print(p1.x)
    print(p1.y)
    p2 = make_point(10, 20)
    print(p2.x)
    print(p2.y)
    dist = distance_sq(p1, p2)
    print(dist)
    r = Rectangle()
    r.top_left = make_point(0, 0)
    r.bottom_right = make_point(100, 50)
    print(r.top_left.x)
    print(r.bottom_right.y)
    p1.x = 99
    print(p1.x)
    return 0


if __name__ == "__main__":
    main()
