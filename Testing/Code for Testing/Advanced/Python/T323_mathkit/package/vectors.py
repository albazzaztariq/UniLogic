"""Vector math operations using flat arrays."""


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
