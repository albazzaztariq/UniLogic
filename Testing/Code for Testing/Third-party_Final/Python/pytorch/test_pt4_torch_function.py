# PyTorch pattern 4: __torch_function__ protocol (simplified)
# Simplified self-contained version — no PyTorch imports
# Pattern: dispatch table mapping operation names to handlers, with type checking


_HANDLED_FUNCTIONS = {}


def implements(op_name):
    """Decorator to register an implementation for an op."""
    def decorator(func):
        _HANDLED_FUNCTIONS[op_name] = func
        return func
    return decorator


class Tensor:
    def __init__(self, data, dtype):
        self.data = data
        self.dtype = dtype
        self.shape = [len(data)]

    def __torch_function_dispatch__(self, op_name, args):
        if op_name in _HANDLED_FUNCTIONS:
            return _HANDLED_FUNCTIONS[op_name](*args)
        return None

    def size(self):
        return self.shape[0]

    def item(self, idx):
        return self.data[idx]

    def dtype_name(self):
        return self.dtype


class QuantizedTensor(Tensor):
    def __init__(self, data, dtype, scale, zero_point):
        Tensor.__init__(self, data, dtype)
        self.scale = scale
        self.zero_point = zero_point

    def dequantize(self):
        result = []
        for v in self.data:
            result.append((v - self.zero_point) * self.scale)
        return Tensor(result, "float32")

    def dtype_name(self):
        return "quantized_" + self.dtype


def dispatch(op_name, *args):
    """Simulate torch function dispatch: check if first arg supports __torch_function__."""
    if len(args) > 0:
        first = args[0]
        if hasattr(first, '__torch_function_dispatch__'):
            result = first.__torch_function_dispatch__(op_name, list(args))
            if result is not None:
                return result
    return None


@implements("add")
def _add(a, b):
    if len(a.data) != len(b.data):
        return None
    result = []
    for i in range(len(a.data)):
        result.append(a.data[i] + b.data[i])
    return Tensor(result, a.dtype)


@implements("mul")
def _mul(a, b):
    if len(a.data) != len(b.data):
        return None
    result = []
    for i in range(len(a.data)):
        result.append(a.data[i] * b.data[i])
    return Tensor(result, a.dtype)


@implements("sum")
def _sum(a):
    total = 0
    for v in a.data:
        total = total + v
    return total


def main():
    t1 = Tensor([1, 2, 3], "float32")
    t2 = Tensor([4, 5, 6], "float32")

    r_add = dispatch("add", t1, t2)
    print("add result size: " + str(r_add.size()))
    print("add result[0]: " + str(r_add.item(0)))
    print("add result[2]: " + str(r_add.item(2)))

    r_mul = dispatch("mul", t1, t2)
    print("mul result[1]: " + str(r_mul.item(1)))

    r_sum = dispatch("sum", t1)
    print("sum result: " + str(r_sum))

    qt = QuantizedTensor([100, 110, 120], "int8", 0.1, 100)
    print("quantized dtype: " + str(qt.dtype_name()))
    dq = qt.dequantize()
    print("dequantize[0]: " + str(dq.item(0)))
    print("dequantize[1]: " + str(round(dq.item(1), 6)))
    print("dequantize[2]: " + str(dq.item(2)))

    r_add2 = dispatch("add", dq, t1)
    print("add dequant+t1 result[0]: " + str(r_add2.item(0)))

    # Unknown op falls back to None
    r_unknown = dispatch("unknown_op", t1, t2)
    print("unknown op result: " + str(r_unknown))


if __name__ == "__main__":
    main()
