# @dr python_compat = true
# @dr pyimport_0 = enum__IntEnum
# @dr pyimport_1 = enum__unique

from dataclasses import dataclass, field
from enum import IntEnum, unique


def __ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class __ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)

from enum import IntEnum

class ExitStatus(IntEnum):
    SUCCESS = 0
    ERROR = 1
    ERROR_TIMEOUT = 2
    ERROR_HTTP_3XX = 3
    ERROR_HTTP_4XX = 4
    ERROR_HTTP_5XX = 5
    ERROR_TOO_MANY_REDIRECTS = 6
    PLUGIN_ERROR = 7
    ERROR_CTRL_C = 130


def http_status_to_exit_status(http_status, follow=False):
    if (((300 <= http_status) and (http_status <= 399)) and (not follow)):
        return ExitStatus.ERROR_HTTP_3XX
    else:
        if ((400 <= http_status) and (http_status <= 499)):
            return ExitStatus.ERROR_HTTP_4XX
        else:
            if ((500 <= http_status) and (http_status <= 599)):
                return ExitStatus.ERROR_HTTP_5XX
            else:
                return ExitStatus.SUCCESS

def main():
    cases = [[200, False], [301, False], [301, True], [404, False], [500, False], [599, False], [200, True]]
    for __pair0 in cases:
        code = __pair0[0]
        follow = __pair0[1]
        result = http_status_to_exit_status(code, follow)
        print((((((((('http_status_to_exit_status(' + str(code)) + ', ') + str(follow)) + ') = ') + str(result.name)) + ' (') + str(result.value)) + ')'))
    print(((str('ExitStatus.SUCCESS:') + ' ') + str(ExitStatus.SUCCESS.value)))
    print(((str('ExitStatus.ERROR_CTRL_C:') + ' ') + str(ExitStatus.ERROR_CTRL_C.value)))
    return 0


if __name__ == "__main__":
    main()
