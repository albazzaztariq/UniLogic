# @dr python_compat = true
# @dr pyimport_0 = json___whole_

import json


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

class JSONTag:
    def __init__(self, serializer, key=''):
        self.serializer = serializer
        self.key = key
    def check(self, value):
        raise _ul_Exception("NotImplementedError", 'check not implemented')
    def to_json(self, value):
        raise _ul_Exception("NotImplementedError", 'to_json not implemented')
    def to_python(self, value):
        raise _ul_Exception("NotImplementedError", 'to_python not implemented')
    def tag(self, value):
        result = {}
        result[self.key] = self.to_json(value)
        return result

class TagStr(JSONTag):
    def __init__(self, serializer):
        super().__init__(serializer, ' s')
    def check(self, value):
        return isinstance(value, str)
    def to_json(self, value):
        return value
    def to_python(self, value):
        return value

class PassDict(JSONTag):
    def __init__(self, serializer):
        super().__init__(serializer, '')
    def check(self, value):
        return isinstance(value, dict)
    def to_json(self, value):
        result = {}
        for k in value:
            result[k] = str(self.serializer.tag(value[k]))
        return result
    def tag(self, value):
        return self.to_json(value)

class PassList(JSONTag):
    def __init__(self, serializer):
        super().__init__(serializer, '')
    def check(self, value):
        return isinstance(value, _py_list)
    def to_json(self, value):
        result = []
        for item in value:
            result.append(self.serializer.tag(item))
        return result
    def tag(self, value):
        return self.to_json(value)

class CustomTag(JSONTag):
    def __init__(self, serializer):
        super().__init__(serializer, ' cx')
    def check(self, value):
        return False
    def to_json(self, value):
        return value
    def to_python(self, value):
        return value

class TaggedJSONSerializer:
    def __init__(self):
        _dict0 = {}
        self.tags = _dict0
        self.order = []
        self.register(TagStr)
        self.register(PassDict)
        self.register(PassList)
    def register(self, tag_class, force=False, index=None):
        tag = tag_class(self)
        key = tag.key
        if key:
            if ((not force) and (key in self.tags)):
                raise _ul_Exception("KeyError", ('Tag already registered: ' + key))
            self.tags[key] = tag
        if (index == None):
            self.order.append(tag)
        else:
            self.order.insert(index, tag)
    def tag(self, value):
        for t in self.order:
            if t.check(value):
                return t.tag(value)
        return value
    def untag(self, value):
        if (len(value) != 1):
            return value
        key = next(iter(value))
        if (not (key in self.tags)):
            return value
        return self.tags[key].to_python(value[key])
    def _untag_scan(self, value):
        if isinstance(value, dict):
            scanned = {}
            for k in value:
                scanned[k] = self._untag_scan(value[k])
            value = self.untag(scanned)
        else:
            if isinstance(value, _py_list):
                result = []
                for item in value:
                    result.append(self._untag_scan(item))
                value = result
        return value
    def dumps(self, value):
        return json.dumps(self.tag(value))
    def loads(self, value):
        return self._untag_scan(json.loads(value))


def main():
    s = TaggedJSONSerializer()
    print(((str('registered tags:') + ' ') + str(sorted(s.tags.keys()))))
    print(((str('order length:') + ' ') + str(len(s.order))))
    print(((str('tag int:') + ' ') + str(s.tag(42))))
    print(((str('tag str:') + ' ') + str(s.tag('hello'))))
    print(((str('tag list:') + ' ') + str(s.tag(['a', 'b', 'c']))))
    d = {}
    d['x'] = 'val'
    print(((str('tag dict:') + ' ') + str(s.tag(d))))
    try:
        s.register(TagStr)
        print('no error -- UNEXPECTED')
    except Exception as __ex:
        __ex_type = __ex.type_name if isinstance(__ex, _ul_Exception) else type(__ex).__name__
        __ex_msg = __ex.message if isinstance(__ex, _ul_Exception) else str(__ex)
        if __ex_type == "KeyError":
            e = __ex_msg
            print(((str('duplicate registration raises KeyError:') + ' ') + str(True)))
        else:
            raise
    s.register(TagStr, True, None)
    print(((str('force overwrite ok:') + ' ') + str(True)))
    s.register(CustomTag, False, 0)
    print(((str('custom tag at front:') + ' ') + str((s.order[0].key == ' cx'))))
    print(((str('custom in tags:') + ' ') + str((' cx' in s.tags))))
    return 0


if __name__ == "__main__":
    main()
