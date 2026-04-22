#!/usr/bin/env python3
from pycparser import c_parser, c_ast

code = """
typedef struct { double valuedouble; int valueint; } cJSON;
double cJSON_SetNumberHelper(cJSON *object, double number) {
    return object->valuedouble = number;
}
"""
p = c_parser.CParser()
ast = p.parse(code)

# Find the return statement
func = ast.ext[-1]  # last is the function
body = func.body
stmt = body.block_items[-1]  # last statement = return
print(f"Return stmt type: {type(stmt).__name__}")
if hasattr(stmt, 'expr'):
    print(f"Return expr type: {type(stmt.expr).__name__}")
    if isinstance(stmt.expr, c_ast.Assignment):
        print(f"  It IS an Assignment")
    else:
        print(f"  It is NOT an Assignment")
        print(f"  Showing expr AST:")
        stmt.expr.show()
