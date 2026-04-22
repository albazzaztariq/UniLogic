from dataclasses import dataclass, field

def __ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class __ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)

class Animal:
    def __init__(self, name="", age=0):
        self.name = name
        self.age = age
    def speak(self):
        return (self.name + ' makes a sound')
    def describe(self):
        return ((self.name + ' age ') + str(self.age))

class Dog(Animal):
    def __init__(self, name="", age=0, breed=""):
        super().__init__(name, age)
        self.breed = breed
    def speak(self):
        return (self.name + ' barks')
    def fetch(self):
        return (self.name + ' fetches the ball')

class Cat(Animal):
    def __init__(self, name="", age=0):
        super().__init__(name, age)
    def speak(self):
        return (self.name + ' meows')


def main():
    a = Animal()
    a.name = 'Generic'
    a.age = 5
    print(a.speak())
    print(a.describe())
    d = Dog()
    d.name = 'Rex'
    d.age = 3
    d.breed = 'Labrador'
    print(d.speak())
    print(d.describe())
    print(d.fetch())
    c = Cat()
    c.name = 'Whiskers'
    c.age = 7
    print(c.speak())
    print(c.describe())
    return 0


if __name__ == "__main__":
    main()
