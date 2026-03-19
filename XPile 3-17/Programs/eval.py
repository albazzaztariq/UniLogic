from dataclasses import dataclass, field
import sys
def str_char_at(s, i): return s[i] if 0 <= i < len(s) else ''
def str_substr(s, start, length): return s[start:start+length]
def strlen(s): return len(s)

def __ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class __ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)


def skip_spaces(expr, pos):
    len = strlen(expr)
    while (pos[0] < len):
        ch = str_char_at(expr, pos[0])
        if (ch == ' '):
            pos[0] = (pos[0] + 1)
        else:
            break

def parse_number(expr, pos):
    len = strlen(expr)
    skip_spaces(expr, pos)
    if (pos[0] >= len):
        return (False, 'unexpected end of expression')
    start = pos[0]
    negative = False
    ch = str_char_at(expr, pos[0])
    if (ch == '-'):
        negative = True
        pos[0] = (pos[0] + 1)
        if (pos[0] >= len):
            return (False, 'unexpected end after minus')
        ch = str_char_at(expr, pos[0])
    if (ch < '0'):
        return (False, 'expected number')
    if (ch > '9'):
        return (False, 'expected number')
    result = 0
    while (pos[0] < len):
        ch = str_char_at(expr, pos[0])
        if (ch >= '0'):
            if (ch <= '9'):
                digit = 0
                if (ch == '0'):
                    digit = 0
                if (ch == '1'):
                    digit = 1
                if (ch == '2'):
                    digit = 2
                if (ch == '3'):
                    digit = 3
                if (ch == '4'):
                    digit = 4
                if (ch == '5'):
                    digit = 5
                if (ch == '6'):
                    digit = 6
                if (ch == '7'):
                    digit = 7
                if (ch == '8'):
                    digit = 8
                if (ch == '9'):
                    digit = 9
                result = ((result * 10) + digit)
                pos[0] = (pos[0] + 1)
            else:
                break
        else:
            break
    if negative:
        result = (0 - result)
    return (True, result)

def parse_primary(expr, pos):
    skip_spaces(expr, pos)
    len = strlen(expr)
    if (pos[0] >= len):
        return (False, 'unexpected end of expression')
    ch = str_char_at(expr, pos[0])
    if (ch == '('):
        pos[0] = (pos[0] + 1)
        _r0 = parse_add(expr, pos)
        if not _r0[0]:
            return _r0
        val = _r0[1]
        skip_spaces(expr, pos)
        if (pos[0] >= len):
            return (False, 'missing closing parenthesis')
        close = str_char_at(expr, pos[0])
        if (close != ')'):
            return (False, 'expected closing parenthesis')
        pos[0] = (pos[0] + 1)
        return (True, val)
    _r1 = parse_number(expr, pos)
    if not _r1[0]:
        return _r1
    num = _r1[1]
    return (True, num)

def parse_mul(expr, pos):
    _r2 = parse_primary(expr, pos)
    if not _r2[0]:
        return _r2
    lhs = _r2[1]
    while (pos[0] < strlen(expr)):
        skip_spaces(expr, pos)
        if (pos[0] >= strlen(expr)):
            break
        op = str_char_at(expr, pos[0])
        if (op == '*'):
            pos[0] = (pos[0] + 1)
            _r3 = parse_primary(expr, pos)
            if not _r3[0]:
                return _r3
            rhs = _r3[1]
            lhs = (lhs * rhs)
        else:
            if (op == '/'):
                pos[0] = (pos[0] + 1)
                _r4 = parse_primary(expr, pos)
                if not _r4[0]:
                    return _r4
                rhs = _r4[1]
                if (rhs == 0):
                    return (False, 'division by zero')
                lhs = (lhs // rhs)
            else:
                break
    return (True, lhs)

def parse_add(expr, pos):
    _r5 = parse_mul(expr, pos)
    if not _r5[0]:
        return _r5
    lhs = _r5[1]
    while (pos[0] < strlen(expr)):
        skip_spaces(expr, pos)
        if (pos[0] >= strlen(expr)):
            break
        op = str_char_at(expr, pos[0])
        if (op == '+'):
            pos[0] = (pos[0] + 1)
            _r6 = parse_mul(expr, pos)
            if not _r6[0]:
                return _r6
            rhs = _r6[1]
            lhs = (lhs + rhs)
        else:
            if (op == '-'):
                pos[0] = (pos[0] + 1)
                _r7 = parse_mul(expr, pos)
                if not _r7[0]:
                    return _r7
                rhs = _r7[1]
                lhs = (lhs - rhs)
            else:
                break
    return (True, lhs)

def evaluate(expr):
    pos = [0]
    _r8 = parse_add(expr, pos)
    if not _r8[0]:
        return _r8
    result = _r8[1]
    return (True, result)

def test_eval(expr):
    print(expr)
    _r9 = evaluate(expr)
    if not _r9[0]:
        print("error: " + str(_r9[1]), file=sys.stderr)
        sys.exit(1)
    result = _r9[1]
    print(result)
    print('')

def main():
    test_eval('3 + 4')
    test_eval('10 - 3')
    test_eval('6 * 7')
    test_eval('20 / 4')
    test_eval('3 + 4 * 2')
    test_eval('(3 + 4) * 2')
    test_eval('(10 + 2) * (8 - 3)')
    test_eval('100 - 20 * 3 + 5')
    test_eval('((2 + 3) * (4 + 1))')
    test_eval('42')
    return 0


if __name__ == "__main__":
    main()
