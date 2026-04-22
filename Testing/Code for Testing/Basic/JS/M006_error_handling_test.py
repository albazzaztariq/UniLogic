from dataclasses import dataclass, field

def __ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class __ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)


def divide(a, b):
    if (b == 0):
        raise __ul_Exception("DivisionByZero", 'cannot divide by zero')
    return (a // b)

def risky(x):
    if (x < 0):
        raise __ul_Exception("ValueError", 'negative input')
    return (x * 2)

def main():
    print(divide(10, 2))
    try:
        result = divide(10, 0)
        print(result)
    except Exception as __ex:
        __ex_type = __ex.type_name if isinstance(__ex, __ul_Exception) else type(__ex).__name__
        __ex_msg = __ex.message if isinstance(__ex, __ul_Exception) else str(__ex)
        if __ex_type == "DivisionByZero":
            err = __ex_msg
            print('caught: divide by zero')
        else:
            raise
    try:
        val = risky((-5))
        print(val)
    except Exception as __ex:
        __ex_type = __ex.type_name if isinstance(__ex, __ul_Exception) else type(__ex).__name__
        __ex_msg = __ex.message if isinstance(__ex, __ul_Exception) else str(__ex)
        if __ex_type == "ValueError":
            err = __ex_msg
            print('caught: negative input')
        else:
            raise
    try:
        val = risky(10)
        print(val)
    except Exception as __ex:
        __ex_type = __ex.type_name if isinstance(__ex, __ul_Exception) else type(__ex).__name__
        __ex_msg = __ex.message if isinstance(__ex, __ul_Exception) else str(__ex)
        if __ex_type == "ValueError":
            err = __ex_msg
            print('should not reach here')
        else:
            raise
    return 0


if __name__ == "__main__":
    main()
