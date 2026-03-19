from dataclasses import dataclass, field

def __ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class __ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)


def main():
    attempts = 0
    # portal retry  (goto not supported in Python - restructure as loop)
    attempts = (attempts + 1)
    if (attempts > 3):
        print('done after retries')
        return 0
    if (attempts <= 3):
        pass  # goto retry  (goto not supported in Python - restructure as loop)
    return 0


if __name__ == "__main__":
    main()
