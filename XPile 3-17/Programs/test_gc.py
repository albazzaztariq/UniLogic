# @dr memory = gc

from dataclasses import dataclass, field

def __ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class __ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)

class Node:
    def __init__(self, value=0, label=""):
        self.value = value
        self.label = label


def make_nodes(count):
    i = 0
    total = 0
    while (i < count):
        n = Node()
        n.value = i
        n.label = 'node'
        total = (total + n.value)
        i = (i + 1)
    return total

def main():
    result = make_nodes(100)
    print(result)
    return 0


if __name__ == "__main__":
    main()
