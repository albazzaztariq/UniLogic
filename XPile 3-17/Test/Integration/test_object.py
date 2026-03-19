from dataclasses import dataclass, field

class Animal:
    def __init__(self, name="", age=0):
        self.name = name
        self.age = age
    def speak(self):
        return '...'

class Dog(Animal):
    def __init__(self, name="", age=0, breed=""):
        super().__init__(name, age)
        self.breed = breed
    def speak(self):
        return 'Woof!'
    def info(self):
        return self.breed


def main():
    d = Dog()
    d.name = 'Rex'
    d.age = 3
    d.breed = 'Labrador'
    print(d.name)
    print(d.age)
    print(d.speak())
    print(d.info())
    return 0


if __name__ == "__main__":
    main()
