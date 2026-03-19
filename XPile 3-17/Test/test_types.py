from dataclasses import dataclass, field

@dataclass
class Point:
    x: int = 0
    y: int = 0

@dataclass
class Person:
    name: str = ""
    age: int = 0
    location: Point = field(default_factory=Point)

def print_point(p):
    print(p.x)
    print(p.y)

def make_point(x, y):
    result = Point()
    result.x = x
    result.y = y
    return result

def main():
    p = Point()
    p.x = 10
    p.y = 20
    print(p.x)
    print(p.y)
    bob = Person()
    bob.name = 'Bob'
    bob.age = 30
    bob.location.x = 100
    bob.location.y = 200
    print(bob.name)
    print(bob.age)
    print(bob.location.x)
    print(bob.location.y)
    print_point(p)
    q = Point()
    q = make_point(5, 15)
    print(q.x)
    print(q.y)


if __name__ == "__main__":
    main()
