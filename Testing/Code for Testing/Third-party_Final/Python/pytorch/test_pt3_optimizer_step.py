# PyTorch pattern 3: Optimizer step() with parameter groups
# Simplified self-contained version — no PyTorch imports
# Pattern: param groups with lr/momentum, step() applies gradient descent


class Parameter:
    def __init__(self, data):
        self.data = data
        self.grad = 0

    def set_grad(self, g):
        self.grad = g

    def zero_grad(self):
        self.grad = 0


class SGD:
    def __init__(self):
        self.param_groups = []
        self.state = {}

    def add_param_group(self, group):
        self.param_groups.append(group)

    def zero_grad(self):
        for group in self.param_groups:
            params = group["params"]
            for p in params:
                p.zero_grad()

    def step(self):
        for group in self.param_groups:
            lr = group["lr"]
            momentum = group["momentum"]
            params = group["params"]
            for p in params:
                if p.grad == 0:
                    continue
                param_id = id(p)
                if param_id not in self.state:
                    self.state[param_id] = {"momentum_buffer": 0}
                buf = self.state[param_id]["momentum_buffer"]
                buf = momentum * buf + p.grad
                self.state[param_id]["momentum_buffer"] = buf
                p.data = p.data - lr * buf


class Adam:
    def __init__(self):
        self.param_groups = []
        self.state = {}
        self.step_count = 0

    def add_param_group(self, group):
        self.param_groups.append(group)

    def zero_grad(self):
        for group in self.param_groups:
            for p in group["params"]:
                p.zero_grad()

    def step(self):
        self.step_count = self.step_count + 1
        for group in self.param_groups:
            lr = group["lr"]
            beta1 = group["beta1"]
            beta2 = group["beta2"]
            eps = group["eps"]
            for p in group["params"]:
                if p.grad == 0:
                    continue
                param_id = id(p)
                if param_id not in self.state:
                    self.state[param_id] = {"m": 0, "v": 0}
                s = self.state[param_id]
                s["m"] = beta1 * s["m"] + (1 - beta1) * p.grad
                s["v"] = beta2 * s["v"] + (1 - beta2) * p.grad * p.grad
                m_hat = s["m"] / (1 - beta1 ** self.step_count)
                v_hat = s["v"] / (1 - beta2 ** self.step_count)
                p.data = p.data - lr * m_hat / (v_hat ** 0.5 + eps)


def main():
    # SGD test
    w = Parameter(10.0)
    b = Parameter(2.0)
    w.set_grad(1.0)
    b.set_grad(0.5)

    sgd = SGD()
    group = {"params": [w, b], "lr": 0.1, "momentum": 0.9}
    sgd.add_param_group(group)

    sgd.step()
    print("SGD step1 w.data: " + str(w.data))
    print("SGD step1 b.data: " + str(b.data))

    w.set_grad(1.0)
    b.set_grad(0.5)
    sgd.step()
    print("SGD step2 w.data: " + str(w.data))
    print("SGD step2 b.data: " + str(b.data))

    sgd.zero_grad()
    print("SGD after zero_grad w.grad: " + str(w.grad))

    # Adam test
    p1 = Parameter(5.0)
    p1.set_grad(2.0)

    adam = Adam()
    adam_group = {"params": [p1], "lr": 0.001, "beta1": 0.9, "beta2": 0.999, "eps": 1e-8}
    adam.add_param_group(adam_group)

    adam.step()
    print("Adam step1 p1.data: " + str(round(p1.data, 6)))

    p1.set_grad(2.0)
    adam.step()
    print("Adam step2 p1.data: " + str(round(p1.data, 6)))


if __name__ == "__main__":
    main()
