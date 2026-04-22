# @dr python_compat = true
# @dr pyimport_0 = collections__OrderedDict
# @dr pyimport_1 = compat__Mapping
# @dr pyimport_2 = compat__MutableMapping

from dataclasses import dataclass, field
from collections import OrderedDict
from compat import Mapping, MutableMapping


def __ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class __ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)

class CaseInsensitiveDict(MutableMapping):
    def __init__(self, data=None):
        self._store = OrderedDict()
        if (data == None):
            _dict0 = {}
            data = _dict0
        self.update(data)
    def __setitem__(self, key, value):
        self._store[key.lower()] = [key, value]
    def __getitem__(self, key):
        return self._store[key.lower()][1]
    def __delitem__(self, key):
        pass
    def __iter__(self):
        return 0
    def __len__(self):
        return len(self._store)
    def lower_items(self):
        return 0
    def __eq__(self, other):
        if isinstance(other, Mapping):
            other = CaseInsensitiveDict(other)
        else:
            return NotImplemented
        return (dict(self.lower_items()) == dict(other.lower_items()))
    def copy(self):
        return CaseInsensitiveDict(self._store.values())
    def __repr__(self):
        return str(dict(self.items()))

class LookupDict(dict):
    def __init__(self, name=None):
        self.name = name
        super().__init__()
    def __repr__(self):
        return (("<lookup '" + str(self.name)) + "'>")
    def __getitem__(self, key):
        return self.__dict__.get(key, None)
    def get(self, key, _py_default=None):
        return self.__dict__.get(key, _py_default)



if __name__ == "__main__":
    main()
