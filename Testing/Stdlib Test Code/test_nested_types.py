from dataclasses import dataclass, field

def __ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class __ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)

@dataclass
class Inner:
    value: int = 0
    code: int = 0

@dataclass
class Middle:
    label: str = ""
    core: Inner = field(default_factory=Inner)

@dataclass
class Outer:
    id: int = 0
    mid: Middle = field(default_factory=Middle)


def set_inner(v, c):
    i = Inner()
    i.value = v
    i.code = c
    return i

def main():
    o = Outer()
    o.id = 1
    o.mid.label = 'test'
    o.mid.core.value = 42
    o.mid.core.code = 99
    print(o.id)
    print(o.mid.label)
    print(o.mid.core.value)
    print(o.mid.core.code)
    o2 = Outer()
    o2.id = 2
    o2.mid.label = 'second'
    o2.mid.core.value = 100
    o2.mid.core.code = 200
    print(o2.id)
    print(o2.mid.label)
    print(o2.mid.core.value)
    print(o2.mid.core.code)
    o.mid.core.value = 777
    print(o.mid.core.value)
    return 0


if __name__ == "__main__":
    main()
