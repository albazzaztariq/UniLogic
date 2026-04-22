#!/usr/bin/env python3
from pycparser import c_parser

# Test return ((void *)0);
code = """
void *foo(void) {
    return
          ((void *)0)
              ;
}
"""
try:
    p = c_parser.CParser()
    p.parse(code)
    print("PASS")
except Exception as e:
    print(f"FAIL: {e}")

# Also test with compact form
code2 = """
void *foo(void) {
    return ((void *)0);
}
"""
try:
    p = c_parser.CParser()
    p.parse(code2)
    print("PASS compact")
except Exception as e:
    print(f"FAIL compact: {e}")
