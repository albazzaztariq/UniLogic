"""
Test file: simplified SQLAlchemy operator overloading (sql/elements.py pattern).
Self-contained, no external deps.
Stresses: __eq__, __ne__, __and__, __or__, __invert__, operator chaining,
          __repr__, __str__, expression tree building.
Adaptations:
  - No actual SQL generation — just expression tree as string repr.
  - No TypeEngine, no ClauseElement base — simplified to plain class.
  - Boolean operators (&, |, ~) used directly — py2ul maps to both1/either1/bitflip.
  - Chained method calls (.method()) on comparison results use intermediate vars
    to avoid known UL bug: x == 5.__and__(y) parses as x == (5.__and__(y)).
"""


class BinaryExpression:
    """Represents a binary SQL expression (left op right)."""

    def __init__(self, left, op, right):
        self.left = left
        self.op = op
        self.right = right

    def __repr__(self):
        return "(" + repr(self.left) + " " + self.op + " " + repr(self.right) + ")"

    def __and__(self, other):
        return BinaryExpression(self, "AND", other)

    def __or__(self, other):
        return BinaryExpression(self, "OR", other)

    def __invert__(self):
        return UnaryExpression("NOT", self)


class UnaryExpression:
    """Represents a unary SQL expression (op operand)."""

    def __init__(self, op, operand):
        self.op = op
        self.operand = operand

    def __repr__(self):
        return "(" + self.op + " " + repr(self.operand) + ")"

    def __and__(self, other):
        return BinaryExpression(self, "AND", other)

    def __or__(self, other):
        return BinaryExpression(self, "OR", other)

    def __invert__(self):
        return UnaryExpression("NOT", self)


class Column:
    """Simplified SQLAlchemy Column with expression operators."""

    def __init__(self, name):
        self.name = name

    def __repr__(self):
        return self.name

    def __eq__(self, other):
        return BinaryExpression(self, "=", other)

    def __ne__(self, other):
        return BinaryExpression(self, "!=", other)

    def __lt__(self, other):
        return BinaryExpression(self, "<", other)

    def __le__(self, other):
        return BinaryExpression(self, "<=", other)

    def __gt__(self, other):
        return BinaryExpression(self, ">", other)

    def __ge__(self, other):
        return BinaryExpression(self, ">=", other)

    def __and__(self, other):
        return BinaryExpression(self, "AND", other)

    def __or__(self, other):
        return BinaryExpression(self, "OR", other)

    def __invert__(self):
        return UnaryExpression("NOT", self)


def main():
    col_id = Column("id")
    col_name = Column("name")
    col_age = Column("age")

    # Basic comparison operators
    expr_eq = col_id == 5
    print("eq:", repr(expr_eq))

    expr_ne = col_name != "admin"
    print("ne:", repr(expr_ne))

    expr_lt = col_age < 30
    print("lt:", repr(expr_lt))

    expr_le = col_age <= 30
    print("le:", repr(expr_le))

    expr_gt = col_age > 18
    print("gt:", repr(expr_gt))

    expr_ge = col_age >= 18
    print("ge:", repr(expr_ge))

    # Boolean combination (__and__) via & operator
    expr_and = (col_id == 5) & (col_name != "admin")
    print("and:", repr(expr_and))

    # Boolean combination (__or__) via | operator
    expr_or = (col_age < 10) | (col_age > 90)
    print("or:", repr(expr_or))

    # Chained: (id == 5) AND (age > 18) AND (name != 'admin')
    # Use intermediate vars to avoid chained-method-on-literal UL bug
    tmp1 = col_id == 5
    tmp2 = col_age > 18
    tmp3 = tmp1 & tmp2
    expr_chain = tmp3 & (col_name != "admin")
    print("chain:", repr(expr_chain))

    # Invert (__invert__) via ~ operator
    expr_not = ~(col_id == 5)
    print("not:", repr(expr_not))

    # Combined not + and
    tmp4 = col_id == 5
    tmp5 = ~tmp4
    expr_not_and = tmp5 & (col_age > 18)
    print("not_and:", repr(expr_not_and))

    # OR of two AND expressions
    tmp6 = col_id == 1
    tmp7 = col_age > 10
    expr_complex = tmp6 & tmp7
    tmp8 = col_id == 2
    tmp9 = col_age < 5
    expr_complex2 = tmp8 & tmp9
    expr_full = expr_complex | expr_complex2
    print("complex:", repr(expr_full))

    # Nested NOT
    tmp10 = col_id == 7
    tmp11 = ~tmp10
    expr_double_not = ~tmp11
    print("double_not:", repr(expr_double_not))


main()
