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
