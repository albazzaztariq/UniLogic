# PyTorch pattern 1: nn.Module __getattr__ interception
# Simplified self-contained version — no PyTorch imports
# Pattern: Module stores params/buffers in dicts, __getattr__ intercepts attribute access


class Module:
    def __init__(self):
        self._parameters = {}
        self._buffers = {}
        self._modules = {}
        self._training = True

    def register_parameter(self, name, value):
        self._parameters[name] = value

    def register_buffer(self, name, value):
        self._buffers[name] = value

    def register_module(self, name, module):
        self._modules[name] = module

    def __getattr__(self, name):
        if name in self._parameters:
            return self._parameters[name]
        if name in self._buffers:
            return self._buffers[name]
        if name in self._modules:
            return self._modules[name]
        return None

    def training_mode(self, mode):
        self._training = mode
        return self

    def is_training(self):
        return self._training

    def parameter_count(self):
        return len(self._parameters)

    def buffer_count(self):
        return len(self._buffers)

    def module_count(self):
        return len(self._modules)


class Linear(Module):
    def __init__(self, in_features, out_features):
        Module.__init__(self)
        self.in_features = in_features
        self.out_features = out_features
        self.register_parameter("weight", in_features * out_features)
        self.register_parameter("bias", out_features)

    def forward(self, x):
        return x * self.in_features + self.out_features


class Sequential(Module):
    def __init__(self):
        Module.__init__(self)

    def add(self, name, layer):
        self.register_module(name, layer)

    def forward(self, x):
        result = x
        for key in self._modules:
            m = self._modules[key]
            result = m.forward(result)
        return result


def main():
    m = Module()
    m.register_parameter("weight", 42)
    m.register_parameter("bias", 7)
    m.register_buffer("running_mean", 0)
    m.register_buffer("running_var", 1)

    print("parameter_count: " + str(m.parameter_count()))
    print("buffer_count: " + str(m.buffer_count()))
    print("weight via getattr: " + str(m.__getattr__("weight")))
    print("running_mean via getattr: " + str(m.__getattr__("running_mean")))
    print("training default: " + str(m.is_training()))
    m.training_mode(False)
    print("training after eval: " + str(m.is_training()))

    lin = Linear(4, 8)
    print("Linear param count: " + str(lin.parameter_count()))
    print("Linear forward(2): " + str(lin.forward(2)))

    seq = Sequential()
    seq.add("layer0", lin)
    print("Sequential module count: " + str(seq.module_count()))
    print("Sequential forward(3): " + str(seq.forward(3)))


if __name__ == "__main__":
    main()
