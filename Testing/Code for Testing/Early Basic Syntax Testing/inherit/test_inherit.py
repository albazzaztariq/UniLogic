from dataclasses import dataclass, field

def _ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class _ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)

@dataclass
class Animal:
    name: str = ""
    age: int = 0

@dataclass
class Dog(Animal):
    breed: str = ""

@dataclass
class Puppy(Dog):
    weight: int = 0


def describe(d):
    print(d.name)
    print(d.age)
    print(d.breed)

def main():
    a = Animal()
    a.name = 'Cat'
    a.age = 5
    print(a.name)
    print(a.age)
    d = Dog()
    d.name = 'Rex'
    d.age = 3
    d.breed = 'Labrador'
    print(d.name)
    print(d.age)
    print(d.breed)
    describe(d)
    p = Puppy()
    p.name = 'Tiny'
    p.age = 1
    p.breed = 'Beagle'
    p.weight = 8
    print(p.name)
    print(p.age)
    print(p.breed)
    print(p.weight)
    return 0


if __name__ == "__main__":
    main()
