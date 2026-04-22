from dataclasses import dataclass, field
def strlen(s): return len(s)
def strstr(s, sub): idx = s.find(sub); return s[idx:] if idx >= 0 else None

def __ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class __ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)


def str_len(s):
    return strlen(s)

def str_contains(s, sub):
    found = strstr(s, sub)
    return (found != None)

def main():
    s = 'Hello World'
    len = len(s)
    print(len)
    has = str_contains(s, 'World')
    print(has)
    return 0


if __name__ == "__main__":
    main()
