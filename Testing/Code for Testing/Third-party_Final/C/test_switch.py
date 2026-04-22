#!/usr/bin/env python3
"""Test c2ul switch fall-through handling."""
import sys, tempfile, os
_SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
_XPILE_ROOT = os.path.normpath(os.path.join(_SCRIPT_DIR, '..', '..', '..', '..'))
sys.path.insert(0, os.path.join(_XPILE_ROOT, 'Tools'))
from c2ul import translate_file

c_code = r"""
void test(unsigned char *input_pointer) {
    int escape_characters = 0;
    switch (*input_pointer)
    {
        case '\"':
        case '\\':
        case '\b':
        case '\f':
        case '\n':
        case '\r':
        case '\t':
            escape_characters++;
            break;
        default:
            if (*input_pointer < 32)
            {
                escape_characters += 5;
            }
            break;
    }
}
"""

with tempfile.NamedTemporaryFile(mode='w', suffix='.c', delete=False, dir=os.path.join(_XPILE_ROOT, 'Tests', 'RealWorld', 'C')) as f:
    f.write(c_code)
    tmp_path = f.name

out_path = tmp_path.replace('.c', '.ul')
try:
    translate_file(tmp_path, out_path)
    with open(out_path) as f:
        content = f.read()
    print("Generated UL:")
    print(content)
finally:
    os.unlink(tmp_path)
    if os.path.exists(out_path):
        os.unlink(out_path)
