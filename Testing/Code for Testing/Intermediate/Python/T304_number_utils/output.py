from dataclasses import dataclass, field

def __ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class __ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)


def is_prime(n):
    if (n < 2):
        return False
    if (n == 2):
        return True
    if ((n % 2) == 0):
        return False
    i = 3
    while ((i * i) <= n):
        if ((n % i) == 0):
            return False
        i = (i + 2)
    return True

def fibonacci(n):
    if (n <= 0):
        return 0
    if (n == 1):
        return 1
    a = 0
    b = 1
    i = 2
    while (i <= n):
        temp = (a + b)
        a = b
        b = temp
        i = (i + 1)
    return b

def count_digits(n):
    if (n == 0):
        return 1
    count = 0
    val = n
    if (val < 0):
        val = (0 - val)
    while (val > 0):
        val = (val // 10)
        count = (count + 1)
    return count

def sum_digits(n):
    total = 0
    val = n
    if (val < 0):
        val = (0 - val)
    while (val > 0):
        total = (total + (val % 10))
        val = (val // 10)
    return total

def reverse_number(n):
    result = 0
    val = n
    while (val > 0):
        result = ((result * 10) + (val % 10))
        val = (val // 10)
    return result

def main():
    i = 2
    while (i <= 30):
        if is_prime(i):
            print(i)
        i = (i + 1)
    j = 0
    while (j <= 10):
        print(fibonacci(j))
        j = (j + 1)
    print(count_digits(12345))
    print(count_digits(0))
    print(sum_digits(12345))
    print(sum_digits(9999))
    print(reverse_number(12345))
    print(reverse_number(100))
    return 0


if __name__ == "__main__":
    main()
