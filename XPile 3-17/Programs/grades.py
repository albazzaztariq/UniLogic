from dataclasses import dataclass, field

def __ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class __ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)

class Student:
    def __init__(self, name="", grade1=0, grade2=0, grade3=0, grade4=0, grade5=0):
        self.name = name
        self.grade1 = grade1
        self.grade2 = grade2
        self.grade3 = grade3
        self.grade4 = grade4
        self.grade5 = grade5
    def average(self):
        sum = ((((self.grade1 + self.grade2) + self.grade3) + self.grade4) + self.grade5)
        return (sum // 5)
    def highest(self):
        best = self.grade1
        if (self.grade2 > best):
            best = self.grade2
        if (self.grade3 > best):
            best = self.grade3
        if (self.grade4 > best):
            best = self.grade4
        if (self.grade5 > best):
            best = self.grade5
        return best
    def lowest(self):
        worst = self.grade1
        if (self.grade2 < worst):
            worst = self.grade2
        if (self.grade3 < worst):
            worst = self.grade3
        if (self.grade4 < worst):
            worst = self.grade4
        if (self.grade5 < worst):
            worst = self.grade5
        return worst
    def letter_grade(self):
        avg = ((((self.grade1 + self.grade2) + self.grade3) + self.grade4) + self.grade5)
        avg = (avg // 5)
        if (avg >= 90):
            return 'A'
        if (avg >= 80):
            return 'B'
        if (avg >= 70):
            return 'C'
        if (avg >= 60):
            return 'D'
        return 'F'
    def is_passing(self):
        avg = ((((self.grade1 + self.grade2) + self.grade3) + self.grade4) + self.grade5)
        avg = (avg // 5)
        return (avg >= 60)


def print_student(s):
    print(s.name)
    print('  Average:')
    print(s.average())
    print('  Grade:')
    print(s.letter_grade())
    print('  Highest:')
    print(s.highest())
    print('  Lowest:')
    print(s.lowest())
    status = 'PASS'
    if (s.is_passing() == False):
        status = 'FAIL'
    print('  Status:')
    print(status)
    print('')

def main():
    s1 = Student()
    s1.name = 'Alice'
    s1.grade1 = 95
    s1.grade2 = 88
    s1.grade3 = 92
    s1.grade4 = 97
    s1.grade5 = 90
    s2 = Student()
    s2.name = 'Bob'
    s2.grade1 = 72
    s2.grade2 = 65
    s2.grade3 = 78
    s2.grade4 = 70
    s2.grade5 = 68
    s3 = Student()
    s3.name = 'Charlie'
    s3.grade1 = 85
    s3.grade2 = 90
    s3.grade3 = 88
    s3.grade4 = 82
    s3.grade5 = 91
    s4 = Student()
    s4.name = 'Diana'
    s4.grade1 = 55
    s4.grade2 = 60
    s4.grade3 = 45
    s4.grade4 = 58
    s4.grade5 = 52
    s5 = Student()
    s5.name = 'Eve'
    s5.grade1 = 98
    s5.grade2 = 95
    s5.grade3 = 100
    s5.grade4 = 97
    s5.grade5 = 99
    s6 = Student()
    s6.name = 'Frank'
    s6.grade1 = 40
    s6.grade2 = 35
    s6.grade3 = 42
    s6.grade4 = 38
    s6.grade5 = 45
    s7 = Student()
    s7.name = 'Grace'
    s7.grade1 = 78
    s7.grade2 = 82
    s7.grade3 = 75
    s7.grade4 = 80
    s7.grade5 = 77
    s8 = Student()
    s8.name = 'Hank'
    s8.grade1 = 88
    s8.grade2 = 91
    s8.grade3 = 85
    s8.grade4 = 90
    s8.grade5 = 86
    print('=== Student Grade Report ===')
    print('')
    print_student(s1)
    print_student(s2)
    print_student(s3)
    print_student(s4)
    print_student(s5)
    print_student(s6)
    print_student(s7)
    print_student(s8)
    print('=== Class Statistics ===')
    total_avg = (((((((s1.average() + s2.average()) + s3.average()) + s4.average()) + s5.average()) + s6.average()) + s7.average()) + s8.average())
    cavg = (total_avg // 8)
    print('Class average:')
    print(cavg)
    best = s1.average()
    best_name = 'Alice'
    if (s2.average() > best):
        best = s2.average()
        best_name = 'Bob'
    if (s3.average() > best):
        best = s3.average()
        best_name = 'Charlie'
    if (s4.average() > best):
        best = s4.average()
        best_name = 'Diana'
    if (s5.average() > best):
        best = s5.average()
        best_name = 'Eve'
    if (s6.average() > best):
        best = s6.average()
        best_name = 'Frank'
    if (s7.average() > best):
        best = s7.average()
        best_name = 'Grace'
    if (s8.average() > best):
        best = s8.average()
        best_name = 'Hank'
    print('Top student:')
    print(best_name)
    failing = 0
    if (s1.is_passing() == False):
        failing = (failing + 1)
    if (s2.is_passing() == False):
        failing = (failing + 1)
    if (s3.is_passing() == False):
        failing = (failing + 1)
    if (s4.is_passing() == False):
        failing = (failing + 1)
    if (s5.is_passing() == False):
        failing = (failing + 1)
    if (s6.is_passing() == False):
        failing = (failing + 1)
    if (s7.is_passing() == False):
        failing = (failing + 1)
    if (s8.is_passing() == False):
        failing = (failing + 1)
    print('Failing students:')
    print(failing)
    return 0


if __name__ == "__main__":
    main()
