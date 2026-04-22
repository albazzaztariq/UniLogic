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

class CeleryWarning(Exception):
    pass

class AlwaysEagerIgnored(CeleryWarning):
    pass

class DuplicateNodenameWarning(CeleryWarning):
    pass

class CeleryError(Exception):
    pass

class TaskPredicate(CeleryError):
    pass

class Ignore(TaskPredicate):
    pass

class Reject(TaskPredicate):
    def __init__(self, reason='', requeue=False):
        self.reason = reason
        self.requeue = requeue
        super().__init__(reason)
    def __repr__(self):
        return ((('reject requeue=' + str(self.requeue)) + ': ') + str(self.reason))

class Retry(TaskPredicate):
    def __init__(self, message='', when=''):
        self.message = message
        self.when = when
        super().__init__(message)
    def humanize(self):
        if self.when:
            return ('in ' + self.when)
        return 'unknown'
    def __str__(self):
        if self.message:
            return self.message
        return ('Retry ' + self.humanize())

class TaskError(CeleryError):
    pass

class IncompleteStream(TaskError):
    pass

class NotRegistered(TaskError):
    def __init__(self):
        super().__init__()
    def __repr__(self):
        return 'Task never registered'

class AlreadyRegistered(TaskError):
    pass

class TimeoutError(TaskError):
    pass

class MaxRetriesExceededError(TaskError):
    def __init__(self, message=''):
        self.message = message
        super().__init__(message)

class ChordError(TaskError):
    pass

class BackendError(Exception):
    pass

class BackendGetMetaError(BackendError):
    def __init__(self, task_id=''):
        self.task_id = task_id
        super().__init__(task_id)

class BackendStoreError(BackendError):
    def __init__(self, state='', task_id=''):
        self.state = state
        self.task_id = task_id
        super().__init__(state)


def main():
    print(((str('CeleryError is Exception:') + ' ') + str(issubclass(CeleryError, Exception))))
    print(((str('TaskPredicate is CeleryError:') + ' ') + str(issubclass(TaskPredicate, CeleryError))))
    print(((str('Ignore is TaskPredicate:') + ' ') + str(issubclass(Ignore, TaskPredicate))))
    print(((str('Retry is TaskPredicate:') + ' ') + str(issubclass(Retry, TaskPredicate))))
    print(((str('TaskError is CeleryError:') + ' ') + str(issubclass(TaskError, CeleryError))))
    print(((str('ChordError is TaskError:') + ' ') + str(issubclass(ChordError, TaskError))))
    print(((str('BackendGetMetaError is BackendError:') + ' ') + str(issubclass(BackendGetMetaError, BackendError))))
    r = Reject('bad message', True)
    print(((str('Reject reason:') + ' ') + str(r.reason)))
    print(((str('Reject requeue:') + ' ') + str(r.requeue)))
    print(((str('Reject repr:') + ' ') + str(repr(r))))
    ret = Retry('try again', '5s')
    print(((str('Retry message:') + ' ') + str(ret.message)))
    print(((str('Retry humanize:') + ' ') + str(ret.humanize())))
    print(((str('Retry str:') + ' ') + str(str(ret))))
    ret_no_msg = Retry('', '10s')
    print(((str('Retry no-msg str:') + ' ') + str(str(ret_no_msg))))
    m = MaxRetriesExceededError('max exceeded')
    print(((str('MaxRetries message:') + ' ') + str(m.message)))
    bg = BackendGetMetaError('task-abc')
    print(((str('BackendGetMetaError task_id:') + ' ') + str(bg.task_id)))
    bs = BackendStoreError('FAILURE', 'task-xyz')
    print(((str('BackendStoreError state:') + ' ') + str(bs.state)))
    print(((str('BackendStoreError task_id:') + ' ') + str(bs.task_id)))
    try:
        raise ChordError('chord failed')
    except TaskError as e:
        print(((str('Caught ChordError as TaskError:') + ' ') + str(str(e))))
    try:
        raise BackendGetMetaError('t-001')
    except BackendError as e:
        print(((str('Caught BackendGetMetaError as BackendError:') + ' ') + str(e.task_id)))
    try:
        raise Ignore('ignored')
    except TaskPredicate as e:
        print('Caught Ignore as TaskPredicate: ok')
    try:
        raise AlwaysEagerIgnored('eager ignored')
    except CeleryWarning as e:
        print('Caught AlwaysEagerIgnored as CeleryWarning: ok')
    ig = Ignore('x')
    print(((str('isinstance Ignore/TaskPredicate:') + ' ') + str(isinstance(ig, TaskPredicate))))
    print(((str('isinstance Ignore/CeleryError:') + ' ') + str(isinstance(ig, CeleryError))))
    not_reg = NotRegistered()
    print(((str('isinstance NotRegistered/TaskError:') + ' ') + str(isinstance(not_reg, TaskError))))
    print(((str('NotRegistered repr:') + ' ') + str(repr(not_reg))))
    return 0


if __name__ == "__main__":
    main()
