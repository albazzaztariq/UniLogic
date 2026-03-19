from dataclasses import dataclass, field
from math import ceil, floor, pow, sqrt

def __ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class __ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)


def math_sqrt(x):
    return sqrt(x)

def math_floor(x):
    return floor(x)

def sort_doubles(arr, n):
    for i in range(1, n):
        key = arr[i]
        j = (i - 1)
        while (j >= 0):
            if (arr[j] > key):
                arr[(j + 1)] = arr[j]
                j = (j - 1)
            else:
                break
        arr[(j + 1)] = key

def stat_mean(arr, n):
    sum = 0.0
    for i in range(n):
        sum = (sum + arr[i])
    return (sum / float(n))

def quantile_sorted(sorted, n, p):
    if (p == 0.0):
        return sorted[0]
    if (p == 1.0):
        return sorted[(n - 1)]
    idx = (float((n - 1)) * p)
    fl = math_floor(idx)
    lo = int(fl)
    fraction = (idx - fl)
    if (fraction < 0.0001):
        return sorted[lo]
    return (sorted[lo] + (fraction * (sorted[(lo + 1)] - sorted[lo])))

def stat_quantile(arr, n, p):
    sorted = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
    for i in range(n):
        sorted[i] = arr[i]
    sort_doubles(sorted, n)
    return quantile_sorted(sorted, n, p)

def stat_median(arr, n):
    return stat_quantile(arr, n, 0.5)

def stat_mode(arr, n):
    sorted = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
    for i in range(n):
        sorted[i] = arr[i]
    sort_doubles(sorted, n)
    max_val = sorted[0]
    max_count = 0
    count = 0
    last_val = sorted[0]
    for i in range(n):
        if (sorted[i] == last_val):
            count = (count + 1)
        else:
            count = 1
            last_val = sorted[i]
        if (count > max_count):
            max_count = count
            max_val = sorted[i]
    return max_val

def stat_variance(arr, n):
    m = stat_mean(arr, n)
    sum_sq = 0.0
    for i in range(n):
        dev = (arr[i] - m)
        sum_sq = (sum_sq + (dev * dev))
    return (sum_sq / float(n))

def stat_stddev(arr, n):
    if (n == 1):
        return 0.0
    v = stat_variance(arr, n)
    return math_sqrt(v)

def main():
    print('=== Dataset 1: [2, 4, 4, 4, 5, 5, 7, 9] ===')
    d1 = [2.0, 4.0, 4.0, 4.0, 5.0, 5.0, 7.0, 9.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
    n1 = 8
    print('mean:')
    print(stat_mean(d1, n1))
    print('median:')
    print(stat_median(d1, n1))
    print('mode:')
    print(stat_mode(d1, n1))
    print('variance:')
    print(stat_variance(d1, n1))
    print('stddev:')
    print(stat_stddev(d1, n1))
    print('')
    print('=== Dataset 2: [1, 2, 3, 4, 5] ===')
    d2 = [1.0, 2.0, 3.0, 4.0, 5.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
    n2 = 5
    print('mean:')
    print(stat_mean(d2, n2))
    print('median:')
    print(stat_median(d2, n2))
    print('mode:')
    print(stat_mode(d2, n2))
    print('variance:')
    print(stat_variance(d2, n2))
    print('stddev:')
    print(stat_stddev(d2, n2))
    print('')
    print('=== Quantile tests ===')
    d3 = [3.0, 6.0, 7.0, 8.0, 8.0, 9.0, 10.0, 13.0, 15.0, 16.0, 20.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
    n3 = 11
    print('Q(0.25):')
    print(stat_quantile(d3, n3, 0.25))
    print('Q(0.5):')
    print(stat_quantile(d3, n3, 0.5))
    print('Q(0.75):')
    print(stat_quantile(d3, n3, 0.75))
    print('Q(0.0):')
    print(stat_quantile(d3, n3, 0.0))
    print('Q(1.0):')
    print(stat_quantile(d3, n3, 1.0))
    return 0


if __name__ == "__main__":
    main()
