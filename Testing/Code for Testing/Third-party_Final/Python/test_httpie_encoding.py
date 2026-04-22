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

UTF8 = 'utf-8'
TOO_SMALL_SEQUENCE = 32

def detect_encoding(content_len, bom_type):
    if (content_len <= TOO_SMALL_SEQUENCE):
        return UTF8
    if (bom_type == 'utf8sig'):
        return 'utf-8-sig'
    if (bom_type == 'utf16'):
        return 'utf-16'
    if (bom_type == 'utf32'):
        return 'utf-32'
    return UTF8

def smart_decode(content, encoding):
    if (not encoding):
        encoding = UTF8
    return [content, encoding]

def smart_encode(content, target_encoding):
    return ((target_encoding + ':') + content)

def normalize_encoding(name):
    if ((name == 'utf8') or (name == 'UTF8')):
        return UTF8
    if ((name == 'ascii') or (name == 'ASCII')):
        return 'ascii'
    return name.lower()

def main():
    print(((str('UTF8:') + ' ') + str(UTF8)))
    print(((str('TOO_SMALL_SEQUENCE:') + ' ') + str(TOO_SMALL_SEQUENCE)))
    enc = detect_encoding(5, 'none')
    print(((str('short detect:') + ' ') + str(enc)))
    enc = detect_encoding(64, 'none')
    print(((str('long utf-8 detect:') + ' ') + str(enc)))
    print(((str('utf-8-sig detect:') + ' ') + str(detect_encoding(51, 'utf8sig'))))
    print(((str('utf-16 detect:') + ' ') + str(detect_encoding(51, 'utf16'))))
    print(((str('utf-32 detect:') + ' ') + str(detect_encoding(51, 'utf32'))))
    print(((str('no bom detect:') + ' ') + str(detect_encoding(51, 'none'))))
    text = smart_decode('hello world', 'utf-8')[0]
    used_enc = smart_decode('hello world', 'utf-8')[1]
    print(((((((str('smart_decode:') + ' ') + str(text)) + ' ') + str('encoding:')) + ' ') + str(used_enc)))
    text2 = smart_decode('auto detect me', '')[0]
    used_enc2 = smart_decode('auto detect me', '')[1]
    print(((((((str('smart_decode auto:') + ' ') + str(text2)) + ' ') + str('encoding:')) + ' ') + str(used_enc2)))
    text3 = smart_decode('another test', None)[0]
    used_enc3 = smart_decode('another test', None)[1]
    print(((((((str('smart_decode none enc:') + ' ') + str(text3)) + ' ') + str('encoding:')) + ' ') + str(used_enc3)))
    encoded = smart_encode('hello world', 'utf-8')
    print(((str('smart_encode:') + ' ') + str(encoded)))
    encoded_ascii = smart_encode('hello', 'ascii')
    print(((str('smart_encode ascii:') + ' ') + str(encoded_ascii)))
    original = 'Testing round-trip encoding'
    encoded2 = smart_encode(original, 'utf-8')
    content_part = encoded2.split(':', 1)[1]
    print(((str('round-trip ok:') + ' ') + str((content_part == original))))
    print(((str('round-trip encoding:') + ' ') + str(encoded2.split(':')[0])))
    print(((str('normalize utf8:') + ' ') + str(normalize_encoding('utf8'))))
    print(((str('normalize UTF8:') + ' ') + str(normalize_encoding('UTF8'))))
    print(((str('normalize ascii:') + ' ') + str(normalize_encoding('ascii'))))
    print(((str('normalize ASCII:') + ' ') + str(normalize_encoding('ASCII'))))
    print(((str('normalize latin-1:') + ' ') + str(normalize_encoding('latin-1'))))
    result = smart_decode('tuple test', 'ascii')
    print(((str('result len:') + ' ') + str(len(result))))
    print(((str('result[0] type:') + ' ') + str(_py_type(result[0]).__name__)))
    print(((str('result[1] type:') + ' ') + str(_py_type(result[1]).__name__)))
    print(((str('at boundary:') + ' ') + str(detect_encoding(32, 'none'))))
    print(((str('just over boundary:') + ' ') + str(detect_encoding(33, 'none'))))
    return 0


if __name__ == "__main__":
    main()
