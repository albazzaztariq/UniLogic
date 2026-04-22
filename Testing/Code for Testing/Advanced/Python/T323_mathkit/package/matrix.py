"""2x2 and 3x3 matrix operations using flat arrays.
A 2x2 matrix is stored as [a, b, c, d] = [[a, b], [c, d]].
A 3x3 matrix is stored as [a, b, c, d, e, f, g, h, i].
"""


def mat2_multiply(a: list, b: list, result: list) -> None:
    """Multiply two 2x2 matrices."""
    result[0] = a[0] * b[0] + a[1] * b[2]
    result[1] = a[0] * b[1] + a[1] * b[3]
    result[2] = a[2] * b[0] + a[3] * b[2]
    result[3] = a[2] * b[1] + a[3] * b[3]


def mat2_det(m: list) -> int:
    """Determinant of a 2x2 matrix."""
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
    """Determinant of a 3x3 matrix using cofactor expansion."""
    return (m[0] * (m[4] * m[8] - m[5] * m[7])
            - m[1] * (m[3] * m[8] - m[5] * m[6])
            + m[2] * (m[3] * m[7] - m[4] * m[6]))


def mat3_trace(m: list) -> int:
    return m[0] + m[4] + m[8]


def mat3_to_string(m: list) -> str:
    return ("[" + str(m[0]) + " " + str(m[1]) + " " + str(m[2]) + "; "
            + str(m[3]) + " " + str(m[4]) + " " + str(m[5]) + "; "
            + str(m[6]) + " " + str(m[7]) + " " + str(m[8]) + "]")
