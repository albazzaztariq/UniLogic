from dataclasses import dataclass, field

def __ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class __ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)


def fizzbuzz(n):
    if ((n % 15) == 0):
        return 'FizzBuzz'
    else:
        if ((n % 3) == 0):
            return 'Fizz'
        else:
            if ((n % 5) == 0):
                return 'Buzz'
            else:
                return str(n)

def count_fizz(limit):
    count = 0
    i = 1
    while (i <= limit):
        if (fizzbuzz(i) == 'Fizz'):
            count = (count + 1)
        i = (i + 1)
    return count

def count_buzz(limit):
    count = 0
    i = 1
    while (i <= limit):
        if (fizzbuzz(i) == 'Buzz'):
            count = (count + 1)
        i = (i + 1)
    return count

def count_fizzbuzz(limit):
    count = 0
    i = 1
    while (i <= limit):
        if (fizzbuzz(i) == 'FizzBuzz'):
            count = (count + 1)
        i = (i + 1)
    return count

def main():
    i = 1
    while (i <= 20):
        print(fizzbuzz(i))
        i = (i + 1)
    print(('Fizz count: ' + str(count_fizz(100))))
    print(('Buzz count: ' + str(count_buzz(100))))
    print(('FizzBuzz count: ' + str(count_fizzbuzz(100))))
    return 0


if __name__ == "__main__":
    main()
