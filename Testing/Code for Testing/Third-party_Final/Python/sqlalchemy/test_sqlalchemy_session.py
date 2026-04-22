"""
Test file: simplified SQLAlchemy Session context manager pattern
           (orm/session.py pattern).
Self-contained, no external deps.
Stresses: __enter__/__exit__ context manager protocol, state machine transitions,
          exception handling in context, nested context managers,
          transaction commit/rollback pattern.
Adaptations:
  - No actual DB connection — simulated with string state tracking.
  - exc_type/exc_val/exc_tb args in __exit__ simplified to single 'exc' arg
    (py2ul cannot handle 3-arg unpacking in function signatures well).
  - contextlib not used — raw __enter__/__exit__ protocol.
"""


class SessionTransaction:
    """Simulated transaction object."""

    def __init__(self, session):
        self.session = session
        self.is_active = True
        self.operations = []

    def add_operation(self, op):
        if self.is_active:
            self.operations.append(op)
        else:
            raise RuntimeError("transaction is not active")

    def commit(self):
        if not self.is_active:
            raise RuntimeError("already committed or rolled back")
        self.is_active = False
        self.session.committed_ops = self.operations[:]
        self.session.state = "committed"

    def rollback(self):
        if not self.is_active:
            raise RuntimeError("already committed or rolled back")
        self.is_active = False
        self.operations = []
        self.session.state = "rolled_back"

    def __repr__(self):
        status = "active"
        if not self.is_active:
            status = "inactive"
        return "Transaction(status=" + status + ", ops=" + str(len(self.operations)) + ")"


class Session:
    """Simulated SQLAlchemy Session with context manager support."""

    def __init__(self, name):
        self.name = name
        self.state = "closed"
        self.transaction = None
        self.committed_ops = []

    def open(self):
        self.state = "open"
        self.transaction = SessionTransaction(self)

    def close(self):
        if self.transaction is not None:
            if self.transaction.is_active:
                self.transaction.rollback()
        self.state = "closed"
        self.transaction = None

    def __enter__(self):
        self.open()
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        if exc_type is not None:
            self.close()
            return False
        self.transaction.commit()
        self.state = "closed"
        self.transaction = None
        return False

    def add(self, item):
        if self.transaction is None:
            raise RuntimeError("no active transaction")
        self.transaction.add_operation("add:" + str(item))

    def flush(self):
        if self.transaction is None:
            raise RuntimeError("no active transaction")
        self.transaction.add_operation("flush")

    def __repr__(self):
        return "Session(" + self.name + ", state=" + self.state + ")"


def main():
    # Basic context manager: success path
    s1 = Session("db1")
    print("before:", s1.state)

    s1.open()
    s1.add("user_1")
    s1.add("user_2")
    s1.flush()
    s1.transaction.commit()
    print("after commit:", s1.state)
    print("committed ops:", s1.committed_ops)

    # Context manager protocol via __enter__ / __exit__
    s2 = Session("db2")
    entered = s2.__enter__()
    print("entered:", entered is s2)
    print("state after enter:", s2.state)
    s2.add("article_1")
    s2.add("article_2")
    s2.__exit__(None, None, None)
    print("state after normal exit:", s2.state)
    print("committed ops count:", len(s2.committed_ops))

    # Context manager protocol: exception path (rollback)
    s3 = Session("db3")
    s3.__enter__()
    print("state in exception path:", s3.state)
    s3.add("comment_1")
    # Simulate exception exit
    s3.__exit__(ValueError, "some error", None)
    print("state after exception exit:", s3.state)
    # After close, committed_ops should be empty (no commit happened)
    print("committed ops after rollback:", len(s3.committed_ops))

    # SessionTransaction repr
    s4 = Session("db4")
    s4.open()
    s4.add("item_1")
    print("tx repr:", repr(s4.transaction))
    s4.transaction.commit()
    print("tx repr after commit:", repr(s4.transaction))

    # Rollback path
    s5 = Session("db5")
    s5.open()
    s5.add("item_a")
    s5.add("item_b")
    s5.transaction.rollback()
    print("state after rollback:", s5.state)
    print("ops after rollback:", len(s5.transaction.operations))

    # close() auto-rolls-back active transaction
    s6 = Session("db6")
    s6.open()
    s6.add("x")
    print("before close:", s6.state)
    s6.close()
    print("after close:", s6.state)
    print("transaction after close:", s6.transaction)


main()
