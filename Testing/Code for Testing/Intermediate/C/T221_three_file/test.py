from dataclasses import dataclass, field

def _ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class _ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)


def area_rect(w, h):
    return (w * h)

def perimeter_rect(w, h):
    return (2 * (w + h))

def area_triangle(base, height):
    return ((base * height) // 2)

def cm_to_mm(cm):
    return (cm * 10)

def m_to_cm(m):
    return (m * 100)

def kg_to_grams(kg):
    return (kg * 1000)

def format_result(label, value):
    return ((label + ': ') + str(value))

def format_pair(label, a, b):
    return ((((label + ': ') + str(a)) + ' x ') + str(b))

def main():
    w = 5
    h = 3
    print(format_result('Area', area_rect(w, h)))
    print(format_result('Perimeter', perimeter_rect(w, h)))
    print(format_result('Triangle area', area_triangle(10, 6)))
    print(format_result('2m in cm', m_to_cm(2)))
    print(format_result('15cm in mm', cm_to_mm(15)))
    print(format_result('3kg in grams', kg_to_grams(3)))
    print(format_pair('Rectangle', w, h))
    area_mm = cm_to_mm(cm_to_mm(area_rect(2, 3)))
    print(format_result('Area in mm^2', area_mm))
    return 0


if __name__ == "__main__":
    main()
