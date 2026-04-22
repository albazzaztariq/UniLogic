"""Main entry point exercising all mathkit modules."""

from vectors import vec_add, vec_sub, vec_scale, vec_dot, vec_magnitude_sq, vec_equal, vec_to_string
from stats import stat_sum, stat_min, stat_max, stat_mean_x10, stat_variance_x100, stat_range, stat_median
from matrix import mat2_multiply, mat2_det, mat2_trace, mat2_transpose, mat2_identity, mat2_add, mat2_scale, mat2_to_string, mat3_det, mat3_trace, mat3_to_string
from sequences import fibonacci, is_prime, count_primes, gcd, lcm, factorial, power, triangular, sum_of_digits


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
