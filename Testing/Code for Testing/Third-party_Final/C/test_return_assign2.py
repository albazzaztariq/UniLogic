#!/usr/bin/env python3
"""Test that c2ul correctly handles return assignment."""
import sys
import os
_SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
_XPILE_ROOT = os.path.normpath(os.path.join(_SCRIPT_DIR, '..', '..', '..', '..'))
sys.path.insert(0, os.path.join(_XPILE_ROOT, 'Tools'))

from c2ul import translate_file
import tempfile

c_code = """
typedef struct { double valuedouble; int valueint; } cJSON;
double cJSON_SetNumberHelper(cJSON *object, double number) {
    if (number >= 2147483647) {
        object->valueint = 2147483647;
    } else {
        object->valueint = (int)number;
    }
    return object->valuedouble = number;
}
"""

# Write to temp file
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
