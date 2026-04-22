"""
Test file: simplified celery/exceptions.py hierarchy.
Self-contained, no external deps.
Stresses: deep exception hierarchy, multi-level inheritance, *args/**kwargs
          forwarding in __init__, string formatting, isinstance checks.
Adaptations:
  - Removed external imports (kombu, billiard, click).
  - Removed multiple inheritance (QueueNotFound(KeyError, TaskError) -> single parent).
  - Removed **kwargs forwarding to super() (not round-trippable via UL).
  - Removed Retry.humanize() f-string (kept simple str methods).
"""


class CeleryWarning(Exception):
    """Base class for all Celery warnings."""


class AlwaysEagerIgnored(CeleryWarning):
    """send_task ignores task_always_eager option."""


class DuplicateNodenameWarning(CeleryWarning):
    """Multiple workers are using the same nodename."""


class CeleryError(Exception):
    """Base class for all Celery errors."""


class TaskPredicate(CeleryError):
    """Base class for task-related semi-predicates."""


class Ignore(TaskPredicate):
    """A task can raise this to ignore doing state updates."""


class Reject(TaskPredicate):
    """A task can raise this if it wants to reject/re-queue the message."""

    def __init__(self, reason="", requeue=False):
        self.reason = reason
        self.requeue = requeue
        super().__init__(reason)

    def __repr__(self):
        return "reject requeue=" + str(self.requeue) + ": " + str(self.reason)


class Retry(TaskPredicate):
    """The task is to be retried later."""

    message = ""
    when = ""

    def __init__(self, message="", when=""):
        self.message = message
        self.when = when
        super().__init__(message)

    def humanize(self):
        if self.when:
            return "in " + self.when
        return "unknown"

    def __str__(self):
        if self.message:
            return self.message
        return "Retry " + self.humanize()


class TaskError(CeleryError):
    """Task related errors."""


class IncompleteStream(TaskError):
    """Found the end of a stream, but data is not complete."""


class NotRegistered(TaskError):
    """The task is not registered."""

    def __repr__(self):
        return "Task never registered"


class AlreadyRegistered(TaskError):
    """The task is already registered."""


class TimeoutError(TaskError):
    """The operation timed out."""


class MaxRetriesExceededError(TaskError):
    """The task max restart limit has been exceeded."""

    def __init__(self, message=""):
        self.message = message
        super().__init__(message)


class ChordError(TaskError):
    """A task part of the chord raised an exception."""


class BackendError(Exception):
    """An issue writing or reading to/from the backend."""


class BackendGetMetaError(BackendError):
    """An issue reading from the backend."""

    def __init__(self, task_id=""):
        self.task_id = task_id
        super().__init__(task_id)


class BackendStoreError(BackendError):
    """An issue writing to the backend."""

    def __init__(self, state="", task_id=""):
        self.state = state
        self.task_id = task_id
        super().__init__(state)


def main():
    # Exception hierarchy checks
    print("CeleryError is Exception:", issubclass(CeleryError, Exception))
    print("TaskPredicate is CeleryError:", issubclass(TaskPredicate, CeleryError))
    print("Ignore is TaskPredicate:", issubclass(Ignore, TaskPredicate))
    print("Retry is TaskPredicate:", issubclass(Retry, TaskPredicate))
    print("TaskError is CeleryError:", issubclass(TaskError, CeleryError))
    print("ChordError is TaskError:", issubclass(ChordError, TaskError))
    print("BackendGetMetaError is BackendError:", issubclass(BackendGetMetaError, BackendError))

    # Reject construction and repr
    r = Reject("bad message", True)
    print("Reject reason:", r.reason)
    print("Reject requeue:", r.requeue)
    print("Reject repr:", repr(r))

    # Retry construction and humanize
    ret = Retry("try again", "5s")
    print("Retry message:", ret.message)
    print("Retry humanize:", ret.humanize())
    print("Retry str:", str(ret))

    ret_no_msg = Retry("", "10s")
    print("Retry no-msg str:", str(ret_no_msg))

    # MaxRetriesExceededError
    m = MaxRetriesExceededError("max exceeded")
    print("MaxRetries message:", m.message)

    # BackendGetMetaError
    bg = BackendGetMetaError("task-abc")
    print("BackendGetMetaError task_id:", bg.task_id)

    # BackendStoreError
    bs = BackendStoreError("FAILURE", "task-xyz")
    print("BackendStoreError state:", bs.state)
    print("BackendStoreError task_id:", bs.task_id)

    # Exception catching via isinstance
    try:
        raise ChordError("chord failed")
    except TaskError as e:
        print("Caught ChordError as TaskError:", str(e))

    try:
        raise BackendGetMetaError("t-001")
    except BackendError as e:
        print("Caught BackendGetMetaError as BackendError:", e.task_id)

    try:
        raise Ignore("ignored")
    except TaskPredicate as e:
        print("Caught Ignore as TaskPredicate: ok")

    try:
        raise AlwaysEagerIgnored("eager ignored")
    except CeleryWarning as e:
        print("Caught AlwaysEagerIgnored as CeleryWarning: ok")

    # isinstance checks on instances
    ig = Ignore("x")
    print("isinstance Ignore/TaskPredicate:", isinstance(ig, TaskPredicate))
    print("isinstance Ignore/CeleryError:", isinstance(ig, CeleryError))

    not_reg = NotRegistered()
    print("isinstance NotRegistered/TaskError:", isinstance(not_reg, TaskError))
    print("NotRegistered repr:", repr(not_reg))


main()
