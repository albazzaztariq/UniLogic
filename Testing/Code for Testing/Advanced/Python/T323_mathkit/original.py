"""mathkit — merged from 5-file package: vectors.py, stats.py, matrix.py, sequences.py, main.py"""


# === vectors.py ===

def vec_add(a: list, b: list, result: list, size: int) -> None:
    i: int = 0
    while i < size:
        result[i] = a[i] + b[i]
        i = i + 1


def vec_sub(a: list, b: list, result: list, size: int) -> None:
    i: int = 0
    while i < size:
        result[i] = a[i] - b[i]
        i = i + 1


def vec_scale(a: list, scalar: int, result: list, size: int) -> None:
    i: int = 0
    while i < size:
        result[i] = a[i] * scalar
        i = i + 1


def vec_dot(a: list, b: list, size: int) -> int:
    total: int = 0
    i: int = 0
    while i < size:
        total = total + a[i] * b[i]
        i = i + 1
    return total


def vec_magnitude_sq(a: list, size: int) -> int:
    return vec_dot(a, a, size)


def vec_equal(a: list, b: list, size: int) -> bool:
    i: int = 0
    while i < size:
        if a[i] != b[i]:
            return False
        i = i + 1
    return True


def vec_to_string(a: list, size: int) -> str:
    result: str = "("
    i: int = 0
    while i < size:
        if i > 0:
            result = result + ", "
        result = result + str(a[i])
        i = i + 1
    result = result + ")"
    return result


# === stats.py ===

def stat_sum(data: list, count: int) -> int:
    total: int = 0
    i: int = 0
    while i < count:
        total = total + data[i]
        i = i + 1
    return total


def stat_min(data: list, count: int) -> int:
    result: int = data[0]
    i: int = 1
    while i < count:
        if data[i] < result:
            result = data[i]
        i = i + 1
    return result


def stat_max(data: list, count: int) -> int:
    result: int = data[0]
    i: int = 1
    while i < count:
        if data[i] > result:
            result = data[i]
        i = i + 1
    return result


def stat_mean_x10(data: list, count: int) -> int:
    total: int = stat_sum(data, count)
    return (total * 10) // count


def stat_variance_x100(data: list, count: int) -> int:
    mean_x10: int = stat_mean_x10(data, count)
    total: int = 0
    i: int = 0
    while i < count:
        diff: int = data[i] * 10 - mean_x10
        total = total + diff * diff
        i = i + 1
    return total // count


def stat_range(data: list, count: int) -> int:
    return stat_max(data, count) - stat_min(data, count)


def stat_median(data: list, count: int, work: list) -> int:
    i: int = 0
    while i < count:
        work[i] = data[i]
        i = i + 1
    i = 0
    while i < count - 1:
        j: int = i + 1
        while j < count:
            if work[j] < work[i]:
                temp: int = work[i]
                work[i] = work[j]
                work[j] = temp
            j = j + 1
        i = i + 1
    if count % 2 == 1:
        return work[count // 2]
    return (work[count // 2 - 1] + work[count // 2]) // 2


# === matrix.py ===

def mat2_multiply(a: list, b: list, result: list) -> None:
    result[0] = a[0] * b[0] + a[1] * b[2]
    result[1] = a[0] * b[1] + a[1] * b[3]
    result[2] = a[2] * b[0] + a[3] * b[2]
    result[3] = a[2] * b[1] + a[3] * b[3]


def mat2_det(m: list) -> int:
    return m[0] * m[3] - m[1] * m[2]


def mat2_trace(m: list) -> int:
    return m[0] + m[3]


def mat2_transpose(m: list, result: list) -> None:
    result[0] = m[0]
    result[1] = m[2]
    result[2] = m[1]
    result[3] = m[3]


def mat2_identity(result: list) -> None:
    result[0] = 1
    result[1] = 0
    result[2] = 0
    result[3] = 1


def mat2_add(a: list, b: list, result: list) -> None:
    i: int = 0
    while i < 4:
        result[i] = a[i] + b[i]
        i = i + 1


def mat2_scale(m: list, scalar: int, result: list) -> None:
    i: int = 0
    while i < 4:
        result[i] = m[i] * scalar
        i = i + 1


def mat2_to_string(m: list) -> str:
    return "[" + str(m[0]) + " " + str(m[1]) + "; " + str(m[2]) + " " + str(m[3]) + "]"


def mat3_det(m: list) -> int:
    return (m[0] * (m[4] * m[8] - m[5] * m[7])
            - m[1] * (m[3] * m[8] - m[5] * m[6])
            + m[2] * (m[3] * m[7] - m[4] * m[6]))


def mat3_trace(m: list) -> int:
    return m[0] + m[4] + m[8]


def mat3_to_string(m: list) -> str:
    return ("[" + str(m[0]) + " " + str(m[1]) + " " + str(m[2]) + "; "
            + str(m[3]) + " " + str(m[4]) + " " + str(m[5]) + "; "
            + str(m[6]) + " " + str(m[7]) + " " + str(m[8]) + "]")


# === sequences.py ===

def fibonacci(n: int, result: list) -> None:
    if n == 0:
        return
    result[0] = 0
    if n == 1:
        return
    result[1] = 1
    i: int = 2
    while i < n:
        result[i] = result[i - 1] + result[i - 2]
        i = i + 1


def is_prime(n: int) -> bool:
    if n < 2:
        return False
    if n == 2:
        return True
    if n % 2 == 0:
        return False
    d: int = 3
    while d * d <= n:
        if n % d == 0:
            return False
        d = d + 2
    return True


def count_primes(limit: int) -> int:
    count: int = 0
    n: int = 2
    while n <= limit:
        if is_prime(n):
            count = count + 1
        n = n + 1
    return count


def gcd(a: int, b: int) -> int:
    while b != 0:
        temp: int = b
        b = a % b
        a = temp
    return a


def lcm(a: int, b: int) -> int:
    return (a * b) // gcd(a, b)


def factorial(n: int) -> int:
    result: int = 1
    i: int = 2
    while i <= n:
        result = result * i
        i = i + 1
    return result


def power(base: int, exp: int) -> int:
    result: int = 1
    i: int = 0
    while i < exp:
        result = result * base
        i = i + 1
    return result


def triangular(n: int) -> int:
    return n * (n + 1) // 2


def sum_of_digits(n: int) -> int:
    total: int = 0
    val: int = n
    if val < 0:
        val = -val
    while val > 0:
        total = total + val % 10
        val = val // 10
    return total


# === main.py ===

def test_vectors() -> None:
    print("=== Vectors ===")
    a: list = [1, 2, 3, 0, 0]
    b: list = [4, 5, 6, 0, 0]
    r: list = [0, 0, 0, 0, 0]
    size: int = 3

    vec_add(a, b, r, size)
    print("add: " + vec_to_string(r, size))

    vec_sub(a, b, r, size)
    print("sub: " + vec_to_string(r, size))

    vec_scale(a, 3, r, size)
    print("scale: " + vec_to_string(r, size))

    d: int = vec_dot(a, b, size)
    print("dot: " + str(d))

    m: int = vec_magnitude_sq(a, size)
    print("mag_sq: " + str(m))

    eq: bool = vec_equal(a, a, size)
    print("eq_self: " + str(eq))

    eq2: bool = vec_equal(a, b, size)
    print("eq_diff: " + str(eq2))


def test_stats() -> None:
    print("=== Stats ===")
    data: list = [10, 20, 30, 40, 50, 15, 25, 35, 0, 0]
    count: int = 8
    work: list = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0]

    print("sum: " + str(stat_sum(data, count)))
    print("min: " + str(stat_min(data, count)))
    print("max: " + str(stat_max(data, count)))
    print("mean_x10: " + str(stat_mean_x10(data, count)))
    print("var_x100: " + str(stat_variance_x100(data, count)))
    print("range: " + str(stat_range(data, count)))
    print("median: " + str(stat_median(data, count, work)))


def test_matrix() -> None:
    print("=== Matrix ===")
    a: list = [1, 2, 3, 4]
    b: list = [5, 6, 7, 8]
    r: list = [0, 0, 0, 0]

    mat2_multiply(a, b, r)
    print("mul: " + mat2_to_string(r))

    print("det_a: " + str(mat2_det(a)))
    print("trace_a: " + str(mat2_trace(a)))

    mat2_transpose(a, r)
    print("trans: " + mat2_to_string(r))

    ident: list = [0, 0, 0, 0]
    mat2_identity(ident)
    print("ident: " + mat2_to_string(ident))

    mat2_add(a, b, r)
    print("add: " + mat2_to_string(r))

    mat2_scale(a, 2, r)
    print("scale: " + mat2_to_string(r))

    m3: list = [1, 2, 3, 0, 4, 5, 1, 0, 6]
    print("det3: " + str(mat3_det(m3)))
    print("trace3: " + str(mat3_trace(m3)))
    print("mat3: " + mat3_to_string(m3))


def test_sequences() -> None:
    print("=== Sequences ===")
    fib: list = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    fibonacci(10, fib)
    result: str = ""
    i: int = 0
    while i < 10:
        if i > 0:
            result = result + " "
        result = result + str(fib[i])
        i = i + 1
    print("fib10: " + result)

    print("prime_7: " + str(is_prime(7)))
    print("prime_9: " + str(is_prime(9)))
    print("primes_30: " + str(count_primes(30)))

    print("gcd_12_8: " + str(gcd(12, 8)))
    print("lcm_4_6: " + str(lcm(4, 6)))
    print("fact_6: " + str(factorial(6)))
    print("pow_2_10: " + str(power(2, 10)))
    print("tri_10: " + str(triangular(10)))
    print("digits_12345: " + str(sum_of_digits(12345)))


def main() -> int:
    test_vectors()
    test_stats()
    test_matrix()
    test_sequences()
    return 0


if __name__ == "__main__":
    main()
