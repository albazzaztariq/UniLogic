# PyTorch pattern 2: Autograd-style computational graph (simplified)
# Simplified self-contained version — no PyTorch imports
# Pattern: Value node with grad_fn, backward traversal through a DAG


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
        result = Value(self.data + other.data, "AddBackward")
        result.set_inputs([self, other])
        return result

    def mul(self, other):
        result = Value(self.data * other.data, "MulBackward")
        result.set_inputs([self, other])
        return result

    def neg(self):
        result = Value(-self.data, "NegBackward")
        result.set_inputs([self])
        return result

    def backward_add(self, grad_output):
        for inp in self._inputs:
            inp.grad = inp.grad + grad_output

    def backward_mul(self, grad_output):
        if len(self._inputs) == 2:
            self._inputs[0].grad = self._inputs[0].grad + grad_output * self._inputs[1].data
            self._inputs[1].grad = self._inputs[1].grad + grad_output * self._inputs[0].data

    def backward_neg(self, grad_output):
        if len(self._inputs) == 1:
            self._inputs[0].grad = self._inputs[0].grad + (-grad_output)

    def backward(self):
        self.grad = 1
        stack = [self]
        while len(stack) > 0:
            node = stack[len(stack) - 1]
            stack = stack[:len(stack) - 1]
            if node.grad_fn_name == "AddBackward":
                node.backward_add(node.grad)
            elif node.grad_fn_name == "MulBackward":
                node.backward_mul(node.grad)
            elif node.grad_fn_name == "NegBackward":
                node.backward_neg(node.grad)
            for inp in node._inputs:
                if inp.requires_grad:
                    stack.append(inp)


def main():
    x = Value(3, "leaf")
    y = Value(4, "leaf")
    x.requires_grad = True
    y.requires_grad = True

    # z = x * y + x  =>  dz/dx = y + 1 = 5, dz/dy = x = 3
    z = x.mul(y)
    w = z.add(x)

    print("w.data (3*4+3=15): " + str(w.data))
    w.backward()
    print("x.grad (y+1=5): " + str(x.grad))
    print("y.grad (x=3): " + str(y.grad))

    # Test negation: q = -x, dq/dx = -1
    x2 = Value(5, "leaf")
    x2.requires_grad = True
    q = x2.neg()
    print("q.data (-5): " + str(q.data))
    q.backward()
    print("x2.grad (-1): " + str(x2.grad))


if __name__ == "__main__":
    main()
