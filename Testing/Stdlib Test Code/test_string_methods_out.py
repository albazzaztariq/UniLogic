from dataclasses import dataclass, field
def strlen(s): return len(s)
def strstr(s, sub): idx = s.find(sub); return s[idx:] if idx >= 0 else None


def str_len(s):
    return strlen(s)

def str_contains(s, sub):
    found = strstr(s, sub)
    return (found != None)

def main():
    s = 'Hello World'
    len = str_len(s)
    print(len)
    has = str_contains(s, 'World')
    print(has)
    return 0


if __name__ == "__main__":
    main()
