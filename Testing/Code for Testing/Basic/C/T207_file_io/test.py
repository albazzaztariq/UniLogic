from dataclasses import dataclass, field

def _ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class _ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)


def main():
    content = 'line one'
    print(content)
    doc = (((('header' + '\n') + 'body') + '\n') + 'footer')
    print(doc)
    path = 'output.txt'
    print(('path: ' + path))
    data = '10,20,30'
    print(data)
    return 0


if __name__ == "__main__":
    main()
