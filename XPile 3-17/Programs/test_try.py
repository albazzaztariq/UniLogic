from dataclasses import dataclass, field

def __ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class __ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)


def divide(a, b):
    if (b == 0):
        raise __ul_Exception("ZeroDivisionError", 'division by zero')
    return (a // b)

def main():
    try:
        result = divide(10, 0)
        print('should not reach here')
    except Exception as __ex:
        __ex_type = __ex.type_name if isinstance(__ex, __ul_Exception) else type(__ex).__name__
        __ex_msg = __ex.message if isinstance(__ex, __ul_Exception) else str(__ex)
        if __ex_type == "ZeroDivisionError":
            print('test1: caught division by zero')
        else:
            raise
    try:
        raise __ul_Exception("ValueError", 'bad input')
    except Exception as __ex:
        __ex_type = __ex.type_name if isinstance(__ex, __ul_Exception) else type(__ex).__name__
        __ex_msg = __ex.message if isinstance(__ex, __ul_Exception) else str(__ex)
        if __ex_type == "ValueError":
            e = __ex_msg
            print(('test2: ' + e))
        else:
            raise
    try:
        print('test3: in try')
    finally:
        print('test3: in finally')
    try:
        raise __ul_Exception("RuntimeError", 'something went wrong')
    except Exception as __ex:
        __ex_type = __ex.type_name if isinstance(__ex, __ul_Exception) else type(__ex).__name__
        __ex_msg = __ex.message if isinstance(__ex, __ul_Exception) else str(__ex)
        if True:
            e = __ex_msg
            print(('test4: ' + e))
    try:
        raise __ul_Exception("TypeError", 'type mismatch')
    except Exception as __ex:
        __ex_type = __ex.type_name if isinstance(__ex, __ul_Exception) else type(__ex).__name__
        __ex_msg = __ex.message if isinstance(__ex, __ul_Exception) else str(__ex)
        if __ex_type == "ValueError":
            print('test5: not this one')
        elif __ex_type == "TypeError":
            e = __ex_msg
            print(('test5: ' + e))
        else:
            raise
    try:
        raise __ul_Exception("TestError", 'oops')
    except Exception as __ex:
        __ex_type = __ex.type_name if isinstance(__ex, __ul_Exception) else type(__ex).__name__
        __ex_msg = __ex.message if isinstance(__ex, __ul_Exception) else str(__ex)
        if __ex_type == "TestError":
            print('test6: caught')
        else:
            raise
    finally:
        print('test6: finally ran')
    try:
        x = divide(10, 2)
        print(('test7: ' + str(x)))
    except Exception as __ex:
        __ex_type = __ex.type_name if isinstance(__ex, __ul_Exception) else type(__ex).__name__
        __ex_msg = __ex.message if isinstance(__ex, __ul_Exception) else str(__ex)
        if __ex_type == "ZeroDivisionError":
            print('test7: should not reach')
        else:
            raise
    return 0


if __name__ == "__main__":
    main()
