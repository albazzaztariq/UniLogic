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

class TypedDescriptor:
    def __init__(self, attr_name, type_name):
        self.attr_name = attr_name
        self.type_name = type_name
    def __get__(self, obj, objtype):
        if (obj == None):
            return self
        val = obj.__dict__.get(self.attr_name)
        if (val == None):
            return ''
        return val
    def __set__(self, obj, value):
        obj.__dict__[self.attr_name] = value
    def __repr__(self):
        return (((('TypedDescriptor(' + self.attr_name) + ', ') + self.type_name) + ')')

class ReadOnlyDescriptor:
    def __init__(self, value):
        self.value = value
    def __get__(self, obj, objtype):
        if (obj == None):
            return self
        return self.value
    def __set__(self, obj, value):
        raise _ul_Exception("AttributeError", 'read-only attribute')

class ValidatedDescriptor:
    def __init__(self, attr_name):
        self.attr_name = attr_name
    def __get__(self, obj, objtype):
        if (obj == None):
            return self
        val = obj.__dict__.get(self.attr_name)
        if (val == None):
            return 0
        return val
    def __set__(self, obj, value):
        if (value < 0):
            raise _ul_Exception("ValueError", 'value must be non-negative')
        obj.__dict__[self.attr_name] = value

class Holder:
    def __init__(self):
        0


def main():
    d = TypedDescriptor('name', 'string')
    obj = Holder()
    print(((str('descriptor repr:') + ' ') + str(repr(d))))
    val = d.__get__(obj, Holder)
    print(((str('initial get:') + ' ') + str(val)))
    d.__set__(obj, 'Alice')
    val2 = d.__get__(obj, Holder)
    print(((str('after set:') + ' ') + str(val2)))
    d.__set__(obj, 'Bob')
    val3 = d.__get__(obj, Holder)
    print(((str('after update:') + ' ') + str(val3)))
    self_ref = d.__get__(None, Holder)
    print(((str('class access is descriptor:') + ' ') + str((self_ref == d))))
    rod = ReadOnlyDescriptor('active')
    obj2 = Holder()
    status = rod.__get__(obj2, Holder)
    print(((str('status:') + ' ') + str(status)))
    raised = False
    try:
        rod.__set__(obj2, 'inactive')
    except Exception as __ex:
        __ex_type = __ex.type_name if isinstance(__ex, _ul_Exception) else type(__ex).__name__
        __ex_msg = __ex.message if isinstance(__ex, _ul_Exception) else str(__ex)
        if __ex_type == "AttributeError":
            raised = True
        else:
            raise
    print(((str('read-only set raises:') + ' ') + str(raised)))
    vd = ValidatedDescriptor('count')
    obj3 = Holder()
    v0 = vd.__get__(obj3, Holder)
    print(((str('initial count:') + ' ') + str(v0)))
    vd.__set__(obj3, 5)
    v1 = vd.__get__(obj3, Holder)
    print(((str('count after set:') + ' ') + str(v1)))
    vd.__set__(obj3, (vd.__get__(obj3, Holder) + 1))
    v2 = vd.__get__(obj3, Holder)
    print(((str('count after increment:') + ' ') + str(v2)))
    raised2 = False
    try:
        vd.__set__(obj3, (-1))
    except Exception as __ex:
        __ex_type = __ex.type_name if isinstance(__ex, _ul_Exception) else type(__ex).__name__
        __ex_msg = __ex.message if isinstance(__ex, _ul_Exception) else str(__ex)
        if __ex_type == "ValueError":
            raised2 = True
        else:
            raise
    print(((str('negative raises:') + ' ') + str(raised2)))
    v3 = vd.__get__(obj3, Holder)
    print(((str('count unchanged:') + ' ') + str(v3)))
    obj4 = Holder()
    obj5 = Holder()
    vd.__set__(obj4, 10)
    vd.__set__(obj5, 20)
    v4 = vd.__get__(obj4, Holder)
    v5 = vd.__get__(obj5, Holder)
    print(((str('obj4 count:') + ' ') + str(v4)))
    print(((str('obj5 count:') + ' ') + str(v5)))
    print(((str('independent:') + ' ') + str((v4 != v5))))
    d_age = TypedDescriptor('age', 'int')
    obj6 = Holder()
    d_age.__set__(obj6, 30)
    age_val = d_age.__get__(obj6, Holder)
    print(((str('age:') + ' ') + str(age_val)))
    print(((str('age descriptor:') + ' ') + str(repr(d_age))))
    return 0


if __name__ == "__main__":
    main()
