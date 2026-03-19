from dataclasses import dataclass, field

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


def show_animal(a):
    print(a.name)
    print(a.age)

def show_dog(d):
    print(d.name)
    print(d.age)
    print(d.breed)

def show_puppy(p):
    print(p.name)
    print(p.age)
    print(p.breed)
    print(p.weight)

def main():
    a = Animal()
    a.name = 'Tiger'
    a.age = 10
    show_animal(a)
    d = Dog()
    d.name = 'Rex'
    d.age = 5
    d.breed = 'Husky'
    show_dog(d)
    p = Puppy()
    p.name = 'Tiny'
    p.age = 1
    p.breed = 'Poodle'
    p.weight = 3
    show_puppy(p)
    p.name = 'Spot'
    p.age = 2
    p.breed = 'Beagle'
    p.weight = 5
    show_puppy(p)
    return 0


if __name__ == "__main__":
    main()
