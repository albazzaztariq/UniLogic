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

class RequestException(IOError):
    def __init__(self, message='', response=None, request=None):
        self.response = response
        self.request = request
        super().__init__(message)

class HTTPError(RequestException):
    pass

class ConnectionError(RequestException):
    pass

class ProxyError(ConnectionError):
    pass

class SSLError(ConnectionError):
    pass

class Timeout(RequestException):
    pass

class ConnectTimeout(ConnectionError):
    pass

class ReadTimeout(Timeout):
    pass

class URLRequired(RequestException):
    pass

class TooManyRedirects(RequestException):
    pass

class MissingSchema(RequestException):
    pass

class InvalidURL(RequestException):
    pass

class RequestsWarning(Warning):
    pass


def main():
    e = HTTPError('HTTP 404')
    print(((str('HTTPError is RequestException:') + ' ') + str(isinstance(e, RequestException))))
    print(((str('HTTPError is IOError:') + ' ') + str(isinstance(e, IOError))))
    e2 = ProxyError('proxy failed')
    print(((str('ProxyError is ConnectionError:') + ' ') + str(isinstance(e2, ConnectionError))))
    print(((str('ProxyError is RequestException:') + ' ') + str(isinstance(e2, RequestException))))
    e3 = ConnectTimeout('timed out')
    print(((str('ConnectTimeout is ConnectionError:') + ' ') + str(isinstance(e3, ConnectionError))))
    print(((str('ConnectTimeout is RequestException:') + ' ') + str(isinstance(e3, RequestException))))
    e4 = MissingSchema('no schema', None)
    print(((str('MissingSchema is RequestException:') + ' ') + str(isinstance(e4, RequestException))))
    print(((str('MissingSchema response:') + ' ') + str(e4.response)))
    e5 = SSLError('ssl error', 'resp_obj')
    print(((str('SSLError response:') + ' ') + str(e5.response)))
    try:
        raise TooManyRedirects('too many')
    except RequestException as ex:
        print(((str('Caught TooManyRedirects as RequestException:') + ' ') + str(str(ex))))
    try:
        raise SSLError('ssl error')
    except ConnectionError as ex:
        print(((str('Caught SSLError as ConnectionError:') + ' ') + str(str(ex))))
    try:
        raise ReadTimeout('read timeout')
    except RequestException as ex:
        print(((str('Caught ReadTimeout as RequestException:') + ' ') + str(str(ex))))
    print(((str('ConnectTimeout inherits ConnectionError:') + ' ') + str(issubclass(ConnectTimeout, ConnectionError))))
    print(((str('ConnectTimeout inherits RequestException:') + ' ') + str(issubclass(ConnectTimeout, RequestException))))
    print(((str('ReadTimeout inherits Timeout:') + ' ') + str(issubclass(ReadTimeout, Timeout))))
    print(((str('ProxyError inherits ConnectionError:') + ' ') + str(issubclass(ProxyError, ConnectionError))))
    return 0


if __name__ == "__main__":
    main()
