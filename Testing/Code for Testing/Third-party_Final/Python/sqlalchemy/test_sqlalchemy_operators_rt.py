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

class BinaryExpression:
    def __init__(self, _py_left, op, _py_right):
        self.left = _py_left
        self.op = op
        self.right = _py_right
    def __repr__(self):
        return (((((('(' + repr(self.left)) + ' ') + self.op) + ' ') + repr(self.right)) + ')')
    def __and__(self, other):
        return BinaryExpression(self, 'AND', other)
    def __or__(self, other):
        return BinaryExpression(self, 'OR', other)
    def __invert__(self):
        return UnaryExpression('NOT', self)

class UnaryExpression:
    def __init__(self, op, operand):
        self.op = op
        self.operand = operand
    def __repr__(self):
        return (((('(' + self.op) + ' ') + repr(self.operand)) + ')')
    def __and__(self, other):
        return BinaryExpression(self, 'AND', other)
    def __or__(self, other):
        return BinaryExpression(self, 'OR', other)
    def __invert__(self):
        return UnaryExpression('NOT', self)

class Column:
    def __init__(self, name):
        self.name = name
    def __repr__(self):
        return self.name
    def __eq__(self, other):
        return BinaryExpression(self, '=', other)
    def __ne__(self, other):
        return BinaryExpression(self, '!=', other)
    def __lt__(self, other):
        return BinaryExpression(self, '<', other)
    def __le__(self, other):
        return BinaryExpression(self, '<=', other)
    def __gt__(self, other):
        return BinaryExpression(self, '>', other)
    def __ge__(self, other):
        return BinaryExpression(self, '>=', other)
    def __and__(self, other):
        return BinaryExpression(self, 'AND', other)
    def __or__(self, other):
        return BinaryExpression(self, 'OR', other)
    def __invert__(self):
        return UnaryExpression('NOT', self)


def main():
    col_id = Column('id')
    col_name = Column('name')
    col_age = Column('age')
    expr_eq = (col_id == 5)
    print(((str('eq:') + ' ') + str(repr(expr_eq))))
    expr_ne = (col_name != 'admin')
    print(((str('ne:') + ' ') + str(repr(expr_ne))))
    expr_lt = (col_age < 30)
    print(((str('lt:') + ' ') + str(repr(expr_lt))))
    expr_le = (col_age <= 30)
    print(((str('le:') + ' ') + str(repr(expr_le))))
    expr_gt = (col_age > 18)
    print(((str('gt:') + ' ') + str(repr(expr_gt))))
    expr_ge = (col_age >= 18)
    print(((str('ge:') + ' ') + str(repr(expr_ge))))
    expr_and = ((col_id == 5) & (col_name != 'admin'))
    print(((str('and:') + ' ') + str(repr(expr_and))))
    expr_or = ((col_age < 10) | (col_age > 90))
    print(((str('or:') + ' ') + str(repr(expr_or))))
    tmp1 = (col_id == 5)
    tmp2 = (col_age > 18)
    tmp3 = (tmp1 & tmp2)
    expr_chain = (tmp3 & (col_name != 'admin'))
    print(((str('chain:') + ' ') + str(repr(expr_chain))))
    expr_not = (~(col_id == 5))
    print(((str('not:') + ' ') + str(repr(expr_not))))
    tmp4 = (col_id == 5)
    tmp5 = (~tmp4)
    expr_not_and = (tmp5 & (col_age > 18))
    print(((str('not_and:') + ' ') + str(repr(expr_not_and))))
    tmp6 = (col_id == 1)
    tmp7 = (col_age > 10)
    expr_complex = (tmp6 & tmp7)
    tmp8 = (col_id == 2)
    tmp9 = (col_age < 5)
    expr_complex2 = (tmp8 & tmp9)
    expr_full = (expr_complex | expr_complex2)
    print(((str('complex:') + ' ') + str(repr(expr_full))))
    tmp10 = (col_id == 7)
    tmp11 = (~tmp10)
    expr_double_not = (~tmp11)
    print(((str('double_not:') + ' ') + str(repr(expr_double_not))))
    return 0


if __name__ == "__main__":
    main()
