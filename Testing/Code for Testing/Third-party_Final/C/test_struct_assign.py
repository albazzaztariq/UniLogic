#!/usr/bin/env python3
from pycparser import c_parser

# Test struct pointer field assignment to (void*)0
code = """
typedef struct { void (*allocate)(int); void (*deallocate)(void*); } Hooks;
typedef struct { Hooks hooks; unsigned char *buffer; int length; } PrintBuffer;

void *foo(PrintBuffer *p) {
    p->hooks.deallocate(p->buffer);
    p->length = 0;
    p->buffer =
               ((void *)0)
                   ;
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
