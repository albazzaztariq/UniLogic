#!/usr/bin/env python3
from pycparser import c_parser

# Test if pycparser handles void* cast in initializer
code1 = """
typedef struct { int *p; int n; } S;
static S g = { ((void *)0), 0 };
"""
code2 = """
typedef int size_t;
typedef struct { const unsigned char *json; size_t position; } error;
static error global_error = { ((void *)0), 0 };
"""
code3 = """
typedef int size_t;
typedef struct { const unsigned char *json; size_t position; } error;
static error global_error = { 0, 0 };
"""

for name, code in [("code1", code1), ("code2", code2), ("code3", code3)]:
    try:
        p = c_parser.CParser()
        p.parse(code, filename='test')
        print(f"{name}: PASS")
    except Exception as e:
        print(f"{name}: FAIL - {e}")
