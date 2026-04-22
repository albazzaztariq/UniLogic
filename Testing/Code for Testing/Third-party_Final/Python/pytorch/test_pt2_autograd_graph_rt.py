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

class Value:
    def __init__(self, data, grad_fn_name):
        self.data = data
        self.grad = 0
        self.grad_fn_name = grad_fn_name
        self._inputs = []
        self.requires_grad = True
    def set_inputs(self, inputs):
        self._inputs = inputs
    def add(self, other):
        result = Value((self.data + other.data), 'AddBackward')
        result.set_inputs([self, other])
        return result
    def mul(self, other):
        result = Value((self.data * other.data), 'MulBackward')
        result.set_inputs([self, other])
        return result
    def neg(self):
        result = Value((-self.data), 'NegBackward')
        result.set_inputs([self])
        return result
    def backward_add(self, grad_output):
        for inp in self._inputs:
            inp.grad = (inp.grad + grad_output)
    def backward_mul(self, grad_output):
        if (len(self._inputs) == 2):
            self._inputs[0].grad = (self._inputs[0].grad + (grad_output * self._inputs[1].data))
            self._inputs[1].grad = (self._inputs[1].grad + (grad_output * self._inputs[0].data))
    def backward_neg(self, grad_output):
        if (len(self._inputs) == 1):
            self._inputs[0].grad = (self._inputs[0].grad + (-grad_output))
    def backward(self):
        self.grad = 1
        stack = [self]
        while (len(stack) > 0):
            node = stack[(len(stack) - 1)]
            stack = stack[0:(len(stack) - 1)]
            if (node.grad_fn_name == 'AddBackward'):
                node.backward_add(node.grad)
            else:
                if (node.grad_fn_name == 'MulBackward'):
                    node.backward_mul(node.grad)
                else:
                    if (node.grad_fn_name == 'NegBackward'):
                        node.backward_neg(node.grad)
            for inp in node._inputs:
                if inp.requires_grad:
                    stack.append(inp)


def main():
    x = Value(3, 'leaf')
    y = Value(4, 'leaf')
    x.requires_grad = True
    y.requires_grad = True
    z = x.mul(y)
    w = z.add(x)
    print(('w.data (3*4+3=15): ' + str(w.data)))
    w.backward()
    print(('x.grad (y+1=5): ' + str(x.grad)))
    print(('y.grad (x=3): ' + str(y.grad)))
    x2 = Value(5, 'leaf')
    x2.requires_grad = True
    q = x2.neg()
    print(('q.data (-5): ' + str(q.data)))
    q.backward()
    print(('x2.grad (-1): ' + str(x2.grad)))
    return 0


if __name__ == "__main__":
    main()
