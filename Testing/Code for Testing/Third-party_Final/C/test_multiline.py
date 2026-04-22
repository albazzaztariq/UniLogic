#!/usr/bin/env python3
from pycparser import c_parser

# Test exact problematic pattern - partial truncation at mid-statement
code_good = """
void *foo(void) {
    return
          ((void *)0)
              ;
}
"""
code_mid = """
void *foo(void) {
    return
"""
# The issue: when we have only lines up to "return \n", pycparser sees
# an incomplete statement — but we're treating "end of input" as OK.
# Let me test what pycparser says for these partial inputs.

for name, code in [("good", code_good), ("mid_truncated", code_mid)]:
    try:
        p = c_parser.CParser()
        p.parse(code, filename='test')
        print(f"{name}: PASS")
    except Exception as e:
        msg = str(e)
        print(f"{name}: FAIL - {msg}")
        print(f"  'end of input' in msg: {'end of input' in msg.lower()}")
        print(f"  'eof' in msg: {'eof' in msg.lower()}")
        print(f"  'invalid expression' in msg: {'invalid expression' in msg.lower()}")
