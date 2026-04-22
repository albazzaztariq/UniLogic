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

class MockRequest:
    def __init__(self, url, headers=None):
        self._url = url
        _dict0 = {}
        self._headers = (headers or _dict0)
        _dict1 = {}
        self._new_headers = _dict1
        _tern0 = None
        if ('://' in url):
            _tern0 = url.split('://')[0]
        else:
            _tern0 = 'http'
        scheme = _tern0
        self.type = scheme
    def get_type(self):
        return self.type
    def get_host(self):
        after_scheme = self._url.split('://', 1)[(-1)]
        return after_scheme.split('/')[0]
    def get_origin_req_host(self):
        return self.get_host()
    def get_full_url(self):
        if (not self._headers.get('Host')):
            return self._url
        host = self._headers['Host']
        scheme = self.get_type()
        path = ('/' + '/'.join(self._url.split('://', 1)[(-1)].split('/')[1:len(self._url.split('://', 1)[(-1)].split('/'))]))
        return (((scheme + '://') + host) + path)
    def is_unverifiable(self):
        return True
    def has_header(self, name):
        return ((name in self._headers) or (name in self._new_headers))
    def get_header(self, name, _py_default=None):
        if (name in self._headers):
            return self._headers[name]
        if (name in self._new_headers):
            return self._new_headers[name]
        return _py_default
    def add_unredirected_header(self, name, value):
        self._new_headers[name] = value

class MockResponse:
    def __init__(self, headers):
        self._headers = headers
    def info(self):
        return self
    def get_all(self, name, _py_default=None):
        if (name in self._headers):
            return self._headers[name]
        if (_py_default == None):
            return []
        return _py_default

class RequestsCookieJar(dict):
    def __init__(self):
        super().__init__()
        _dict2 = {}
        self._cookies = _dict2
    def set(self, name, value, domain='', path='/'):
        if (not (domain in self._cookies)):
            _dict3 = {}
            self._cookies[domain] = _dict3
        if (not (path in self._cookies[domain])):
            _dict4 = {}
            self._cookies[domain][path] = _dict4
        self._cookies[domain][path][name] = value
        self[name] = value
    def get(self, name, _py_default=None, domain=None, path=None):
        if (domain != None):
            if (domain in self._cookies):
                d = self._cookies[domain]
            else:
                d = {}
            _tern1 = None
            if path:
                _tern1 = path
            else:
                _tern1 = '/'
            p_key = _tern1
            if (p_key in d):
                p = d[p_key]
            else:
                p = {}
            if (name in p):
                return p[name]
            return _py_default
        if (name in self):
            return self[name]
        return _py_default
    def keys(self):
        return super().keys()
    def values(self):
        return super().values()
    def items(self):
        return super().items()
    def list_domains(self):
        return list(self._cookies.keys())
    def list_paths(self):
        paths = set()
        for domain_data in self._cookies.values():
            for path in domain_data:
                paths.add(path)
        return sorted(paths)
    def __repr__(self):
        return (('<RequestsCookieJar [' + str(dict(self))) + ']>')


def merge_cookies(cookiejar, cookies):
    if (cookies == None):
        return cookiejar
    if isinstance(cookies, dict):
        for __pair0 in cookies.items():
            name = __pair0[0]
            value = __pair0[1]
            cookiejar.set(name, value)
    return cookiejar

def main():
    _dict5 = {}
    _dict5['Accept'] = str('application/json')
    req = MockRequest('https://httpbin.org/get', _dict5)
    print(((str('type:') + ' ') + str(req.get_type())))
    print(((str('host:') + ' ') + str(req.get_host())))
    print(((str('origin_host:') + ' ') + str(req.get_origin_req_host())))
    print(((str('full_url:') + ' ') + str(req.get_full_url())))
    print(((str('is_unverifiable:') + ' ') + str(req.is_unverifiable())))
    print(((str('has Accept header:') + ' ') + str(req.has_header('Accept'))))
    print(((str('has Cookie header:') + ' ') + str(req.has_header('Cookie'))))
    print(((str('get Accept:') + ' ') + str(req.get_header('Accept'))))
    print(((str('get missing:') + ' ') + str(req.get_header('X-Missing', 'none'))))
    req.add_unredirected_header('Cookie', 'session=abc123')
    print(((str('new Cookie header:') + ' ') + str(req.get_header('Cookie'))))
    _dict6 = {}
    _dict6['Host'] = str('override.example.com')
    req2 = MockRequest('https://example.com/path/to/page', _dict6)
    print(((str('full_url with Host:') + ' ') + str(req2.get_full_url())))
    _dict7 = {}
    _dict7['Set-Cookie'] = str('name=value; Path=/')
    resp = MockResponse(_dict7)
    print(((str('cookies from response:') + ' ') + str(resp.get_all('Set-Cookie'))))
    print(((str('missing header:') + ' ') + str(resp.get_all('X-Missing'))))
    jar = RequestsCookieJar()
    jar.set('session', 'abc123', 'example.com', '/')
    jar.set('token', 'xyz789', 'example.com', '/api')
    jar.set('pref', 'dark', 'other.com', '/')
    print(((str('jar repr:') + ' ') + str(repr(jar))))
    print(((str('get session:') + ' ') + str(jar.get('session'))))
    print(((str('get session by domain:') + ' ') + str(jar.get('session'))))
    print(((str('get token:') + ' ') + str(jar.get('token'))))
    print(((str('domains:') + ' ') + str(sorted(jar.list_domains()))))
    print(((str('paths:') + ' ') + str(jar.list_paths())))
    print(((str('jar keys:') + ' ') + str(sorted(jar.keys()))))
    jar2 = RequestsCookieJar()
    _dict8 = {}
    _dict8['a'] = str('1')
    _dict8['b'] = str('2')
    merge_cookies(jar2, _dict8)
    print(((str('merged jar:') + ' ') + str(sorted(jar2.items()))))
    merge_cookies(jar2, None)
    print(((str('merged with None:') + ' ') + str(sorted(jar2.items()))))
    return 0


if __name__ == "__main__":
    main()
