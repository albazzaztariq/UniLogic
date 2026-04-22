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

class SessionTransaction:
    def __init__(self, session):
        self.session = session
        self.is_active = True
        self.operations = []
    def add_operation(self, op):
        if self.is_active:
            self.operations.append(op)
        else:
            raise _ul_Exception("RuntimeError", 'transaction is not active')
    def commit(self):
        if (not self.is_active):
            raise _ul_Exception("RuntimeError", 'already committed or rolled back')
        self.is_active = False
        self.session.committed_ops = self.operations[0:len(self.operations)]
        self.session.state = 'committed'
    def rollback(self):
        if (not self.is_active):
            raise _ul_Exception("RuntimeError", 'already committed or rolled back')
        self.is_active = False
        self.operations = []
        self.session.state = 'rolled_back'
    def __repr__(self):
        status = 'active'
        if (not self.is_active):
            status = 'inactive'
        return (((('Transaction(status=' + status) + ', ops=') + str(len(self.operations))) + ')')

class Session:
    def __init__(self, name):
        self.name = name
        self.state = 'closed'
        self.transaction = None
        self.committed_ops = []
    def open(self):
        self.state = 'open'
        self.transaction = SessionTransaction(self)
    def close(self):
        if (self.transaction != None):
            if self.transaction.is_active:
                self.transaction.rollback()
        self.state = 'closed'
        self.transaction = None
    def __enter__(self):
        self.open()
        return self
    def __exit__(self, exc_type, exc_val, exc_tb):
        if (exc_type != None):
            self.close()
            return False
        self.transaction.commit()
        self.state = 'closed'
        self.transaction = None
        return False
    def add(self, item):
        if (self.transaction == None):
            raise _ul_Exception("RuntimeError", 'no active transaction')
        self.transaction.add_operation(('add:' + str(item)))
    def flush(self):
        if (self.transaction == None):
            raise _ul_Exception("RuntimeError", 'no active transaction')
        self.transaction.add_operation('flush')
    def __repr__(self):
        return (((('Session(' + self.name) + ', state=') + self.state) + ')')


def main():
    s1 = Session('db1')
    print(((str('before:') + ' ') + str(s1.state)))
    s1.open()
    s1.add('user_1')
    s1.add('user_2')
    s1.flush()
    s1.transaction.commit()
    print(((str('after commit:') + ' ') + str(s1.state)))
    print(((str('committed ops:') + ' ') + str(s1.committed_ops)))
    s2 = Session('db2')
    entered = s2.__enter__()
    print(((str('entered:') + ' ') + str((entered == s2))))
    print(((str('state after enter:') + ' ') + str(s2.state)))
    s2.add('article_1')
    s2.add('article_2')
    s2.__exit__(None, None, None)
    print(((str('state after normal exit:') + ' ') + str(s2.state)))
    print(((str('committed ops count:') + ' ') + str(len(s2.committed_ops))))
    s3 = Session('db3')
    s3.__enter__()
    print(((str('state in exception path:') + ' ') + str(s3.state)))
    s3.add('comment_1')
    s3.__exit__(ValueError, 'some error', None)
    print(((str('state after exception exit:') + ' ') + str(s3.state)))
    print(((str('committed ops after rollback:') + ' ') + str(len(s3.committed_ops))))
    s4 = Session('db4')
    s4.open()
    s4.add('item_1')
    print(((str('tx repr:') + ' ') + str(repr(s4.transaction))))
    s4.transaction.commit()
    print(((str('tx repr after commit:') + ' ') + str(repr(s4.transaction))))
    s5 = Session('db5')
    s5.open()
    s5.add('item_a')
    s5.add('item_b')
    s5.transaction.rollback()
    print(((str('state after rollback:') + ' ') + str(s5.state)))
    print(((str('ops after rollback:') + ' ') + str(len(s5.transaction.operations))))
    s6 = Session('db6')
    s6.open()
    s6.add('x')
    print(((str('before close:') + ' ') + str(s6.state)))
    s6.close()
    print(((str('after close:') + ' ') + str(s6.state)))
    print(((str('transaction after close:') + ' ') + str(s6.transaction)))
    return 0


if __name__ == "__main__":
    main()
