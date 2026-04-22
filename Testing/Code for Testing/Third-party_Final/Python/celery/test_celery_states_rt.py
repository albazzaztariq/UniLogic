# @dr python_compat = true


def _ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class _ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)

# python_compat type aliases (UL keywords remapped by py2ul)
_py_list = list
_py_map = dict
_py_type = type
_py_int = int
_py_float = float
_py_string = str
_py_bool = bool
_py_object = object

class state:
    def __init__(self, s=''):
        self.s = s
    def __gt__(self, other):
        return (precedence(self.s) < precedence(other.s))
    def __ge__(self, other):
        return (precedence(self.s) <= precedence(other.s))
    def __lt__(self, other):
        return (precedence(self.s) > precedence(other.s))
    def __le__(self, other):
        return (precedence(self.s) >= precedence(other.s))
    def __str__(self):
        return self.s
    def __repr__(self):
        return (('state(' + self.s) + ')')

PENDING = 'PENDING'
RECEIVED = 'RECEIVED'
STARTED = 'STARTED'
SUCCESS = 'SUCCESS'
FAILURE = 'FAILURE'
REVOKED = 'REVOKED'
REJECTED = 'REJECTED'
RETRY = 'RETRY'
IGNORED = 'IGNORED'
PRECEDENCE = ['SUCCESS', 'FAILURE', '', 'REVOKED', 'STARTED', 'RECEIVED', 'REJECTED', 'RETRY', 'PENDING']
PRECEDENCE_LOOKUP = {}
NONE_PRECEDENCE = None
READY_STATES = []
UNREADY_STATES = []
EXCEPTION_STATES = []
PROPAGATE_STATES = []
ALL_STATES = []

def precedence(s):
    global NONE_PRECEDENCE, PRECEDENCE_LOOKUP
    try:
        return PRECEDENCE_LOOKUP[s]
    except Exception as __ex:
        __ex_type = __ex.type_name if isinstance(__ex, _ul_Exception) else type(__ex).__name__
        __ex_msg = __ex.message if isinstance(__ex, _ul_Exception) else str(__ex)
        if __ex_type == "KeyError":
            return NONE_PRECEDENCE
        else:
            raise

def main():
    print(((str('PENDING:') + ' ') + str(PENDING)))
    print(((str('SUCCESS:') + ' ') + str(SUCCESS)))
    print(((str('FAILURE:') + ' ') + str(FAILURE)))
    print(((str('precedence(SUCCESS):') + ' ') + str(precedence(SUCCESS))))
    print(((str('precedence(FAILURE):') + ' ') + str(precedence(FAILURE))))
    print(((str('precedence(PENDING):') + ' ') + str(precedence(PENDING))))
    print(((str('precedence(unknown):') + ' ') + str(precedence('CUSTOM'))))
    print(((str('NONE_PRECEDENCE:') + ' ') + str(NONE_PRECEDENCE)))
    print(((str('SUCCESS in READY_STATES:') + ' ') + str((SUCCESS in READY_STATES))))
    print(((str('PENDING in READY_STATES:') + ' ') + str((PENDING in READY_STATES))))
    print(((str('PENDING in UNREADY_STATES:') + ' ') + str((PENDING in UNREADY_STATES))))
    print(((str('FAILURE in EXCEPTION_STATES:') + ' ') + str((FAILURE in EXCEPTION_STATES))))
    print(((str('FAILURE in PROPAGATE_STATES:') + ' ') + str((FAILURE in PROPAGATE_STATES))))
    print(((str('REVOKED in PROPAGATE_STATES:') + ' ') + str((REVOKED in PROPAGATE_STATES))))
    print(((str('STARTED in PROPAGATE_STATES:') + ' ') + str((STARTED in PROPAGATE_STATES))))
    s_pending = state(PENDING)
    s_success = state(SUCCESS)
    s_failure = state(FAILURE)
    s_started = state(STARTED)
    s_custom = state('PROGRESS')
    print(((str('state(PENDING) < state(SUCCESS):') + ' ') + str((s_pending < s_success))))
    print(((str('state(SUCCESS) > state(PENDING):') + ' ') + str((s_success > s_pending))))
    print(((str('state(FAILURE) > state(SUCCESS):') + ' ') + str((s_failure > s_success))))
    print(((str('state(SUCCESS) >= state(SUCCESS):') + ' ') + str((s_success >= s_success))))
    print(((str('state(SUCCESS) <= state(SUCCESS):') + ' ') + str((s_success <= s_success))))
    print(((str('state(PROGRESS) > state(STARTED):') + ' ') + str((s_custom > s_started))))
    print(((str('state(PROGRESS) > state(SUCCESS):') + ' ') + str((s_custom > s_success))))
    print(((str('PRECEDENCE length:') + ' ') + str(len(PRECEDENCE))))
    print(((str('PRECEDENCE[0]:') + ' ') + str(PRECEDENCE[0])))
    print(((str('PRECEDENCE[8]:') + ' ') + str(PRECEDENCE[8])))
    print(((str('ALL_STATES length:') + ' ') + str(len(ALL_STATES))))
    print(((str('str(state(FAILURE)):') + ' ') + str(str(s_failure))))
    print(((str('repr(state(RETRY)):') + ' ') + str(repr(state(RETRY)))))
    return 0

def __module_init__():
    global ALL_STATES, EXCEPTION_STATES, NONE_PRECEDENCE, PRECEDENCE, PRECEDENCE_LOOKUP, PROPAGATE_STATES, READY_STATES, UNREADY_STATES
    _i = 0
    for _s in PRECEDENCE:
        PRECEDENCE_LOOKUP[_s] = _i
        _i = (_i + 1)
    NONE_PRECEDENCE = PRECEDENCE_LOOKUP['']
    READY_STATES = [SUCCESS, FAILURE, REVOKED]
    UNREADY_STATES = [PENDING, RECEIVED, STARTED, REJECTED, RETRY]
    EXCEPTION_STATES = [RETRY, FAILURE, REVOKED]
    PROPAGATE_STATES = [FAILURE, REVOKED]
    ALL_STATES = [PENDING, RECEIVED, STARTED, SUCCESS, FAILURE, RETRY, REVOKED]


if __name__ == "__main__":
    __module_init__()
    main()
