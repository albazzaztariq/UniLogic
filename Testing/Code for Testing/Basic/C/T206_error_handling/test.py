from dataclasses import dataclass, field
import sys

def _ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class _ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)


def safe_divide(a, b):
    if (b == 0):
        return (False, 'division by zero')
    return (True, (a // b))

def chain_calc(x):
    _r0 = safe_divide(x, 2)
    if not _r0[0]:
        return _r0
    half = _r0[1]
    doubled = (half * 2)
    return (True, doubled)

def risky_divide(a, b):
    if (b == 0):
        raise _ul_Exception("ArithmeticError", 'cannot divide by zero')
    return (a // b)

def main():
    try:
        r = risky_divide(10, 0)
        print('should not reach')
    except Exception as __ex:
        __ex_type = __ex.type_name if isinstance(__ex, _ul_Exception) else type(__ex).__name__
        __ex_msg = __ex.message if isinstance(__ex, _ul_Exception) else str(__ex)
        if __ex_type == "ArithmeticError":
            e = __ex_msg
            print(('caught: ' + e))
        else:
            raise
    try:
        r = risky_divide(10, 2)
        print(r)
    except Exception as __ex:
        __ex_type = __ex.type_name if isinstance(__ex, _ul_Exception) else type(__ex).__name__
        __ex_msg = __ex.message if isinstance(__ex, _ul_Exception) else str(__ex)
        if __ex_type == "ArithmeticError":
            print('should not catch')
        else:
            raise
    try:
        print('in try')
    finally:
        print('in finally')
    try:
        raise _ul_Exception("SomeError", 'unknown problem')
    except Exception as __ex:
        __ex_type = __ex.type_name if isinstance(__ex, _ul_Exception) else type(__ex).__name__
        __ex_msg = __ex.message if isinstance(__ex, _ul_Exception) else str(__ex)
        if True:
            e = __ex_msg
            print(('catch-all: ' + e))
    try:
        raise _ul_Exception("TestError", 'test')
    except Exception as __ex:
        __ex_type = __ex.type_name if isinstance(__ex, _ul_Exception) else type(__ex).__name__
        __ex_msg = __ex.message if isinstance(__ex, _ul_Exception) else str(__ex)
        if __ex_type == "TestError":
            e = __ex_msg
            print(('caught: ' + e))
        else:
            raise
    finally:
        print('finally ran')
    _r1 = safe_divide(10, 2)
    if not _r1[0]:
        print("error: " + str(_r1[1]), file=sys.stderr)
        sys.exit(1)
    a = _r1[1]
    print(a)
    _r2 = chain_calc(20)
    if not _r2[0]:
        print("error: " + str(_r2[1]), file=sys.stderr)
        sys.exit(1)
    b = _r2[1]
    print(b)
    return 0


if __name__ == "__main__":
    main()
