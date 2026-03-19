"""Numerical computation: matrix ops, statistics, sorting, searching."""

def matrix_multiply(a: list, b: list, rows_a: int, cols_a: int, cols_b: int) -> list:
    result: list = []
    i: int = 0
    while i < rows_a:
        j: int = 0
        while j < cols_b:
            s: float = 0.0
            k: int = 0
            while k < cols_a:
                s = s + a[i * cols_a + k] * b[k * cols_b + j]
                k += 1
            result.append(s)
            j += 1
        i += 1
    return result

def dot_product(a: list, b: list, n: int) -> float:
    s: float = 0.0
    i: int = 0
    while i < n:
        s = s + a[i] * b[i]
        i += 1
    return s

def vector_add(a: list, b: list, n: int) -> list:
    result: list = []
    i: int = 0
    while i < n:
        result.append(a[i] + b[i])
        i += 1
    return result

def mean(data: list, n: int) -> float:
    s: float = 0.0
    i: int = 0
    while i < n:
        s = s + data[i]
        i += 1
    return s / n

def variance(data: list, n: int) -> float:
    m: float = mean(data, n)
    s: float = 0.0
    i: int = 0
    while i < n:
        diff: float = data[i] - m
        s = s + diff * diff
        i += 1
    return s / n

def std_dev(data: list, n: int) -> float:
    v: float = variance(data, n)
    guess: float = v / 2.0
    j: int = 0
    while j < 20:
        if guess <= 0.0:
            return 0.0
        guess = (guess + v / guess) / 2.0
        j += 1
    return guess

def min_val(data: list, n: int) -> float:
    m: float = data[0]
    i: int = 1
    while i < n:
        if data[i] < m:
            m = data[i]
        i += 1
    return m

def max_val(data: list, n: int) -> float:
    m: float = data[0]
    i: int = 1
    while i < n:
        if data[i] > m:
            m = data[i]
        i += 1
    return m

def bubble_sort(data: list, n: int) -> list:
    i: int = 0
    while i < n:
        j: int = 0
        while j < n - i - 1:
            if data[j] > data[j + 1]:
                tmp: float = data[j]
                data[j] = data[j + 1]
                data[j + 1] = tmp
            j += 1
        i += 1
    return data

def binary_search(data: list, n: int, target: float) -> int:
    lo: int = 0
    hi: int = n - 1
    while lo <= hi:
        mid: int = (lo + hi) / 2
        if data[mid] == target:
            return mid
        elif data[mid] < target:
            lo = mid + 1
        else:
            hi = mid - 1
    return -1

def fibonacci(n: int) -> int:
    if n <= 1:
        return n
    a: int = 0
    b: int = 1
    i: int = 2
    while i <= n:
        c: int = a + b
        a = b
        b = c
        i += 1
    return b

def is_prime(n: int) -> bool:
    if n < 2:
        return False
    i: int = 2
    while i * i <= n:
        if n % i == 0:
            return False
        i += 1
    return True

def count_primes(limit: int) -> int:
    count: int = 0
    i: int = 2
    while i < limit:
        if is_prime(i):
            count += 1
        i += 1
    return count

def main() -> int:
    data: list = [3.0, 1.0, 4.0, 1.0, 5.0, 9.0, 2.0, 6.0]
    n: int = 8
    print(mean(data, n))
    print(min_val(data, n))
    print(max_val(data, n))

    sorted_data: list = bubble_sort(data, n)
    print(sorted_data[0])
    print(sorted_data[7])

    print(fibonacci(10))
    print(count_primes(20))
    print(is_prime(17))

    a: list = [1.0, 2.0, 3.0]
    b: list = [4.0, 5.0, 6.0]
    print(dot_product(a, b, 3))

    return 0
