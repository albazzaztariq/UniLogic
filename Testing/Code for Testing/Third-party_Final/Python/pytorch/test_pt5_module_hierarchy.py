# PyTorch pattern 5: Module hierarchy with register_buffer/register_parameter
# Simplified self-contained version — no PyTorch imports
# Pattern: nested module tree, named_parameters(), named_buffers(), state_dict()


class Module:
    def __init__(self, name):
        self._name = name
        self._parameters = {}
        self._buffers = {}
        self._modules = {}

    def register_parameter(self, name, value):
        self._parameters[name] = value

    def register_buffer(self, name, value):
        self._buffers[name] = value

    def add_module(self, name, module):
        self._modules[name] = module

    def named_parameters(self, prefix):
        result = []
        for k in self._parameters:
            full = prefix + "." + k if prefix != "" else k
            result.append([full, self._parameters[k]])
        for k in self._modules:
            child_prefix = prefix + "." + k if prefix != "" else k
            child_params = self._modules[k].named_parameters(child_prefix)
            for pair in child_params:
                result.append(pair)
        return result

    def named_buffers(self, prefix):
        result = []
        for k in self._buffers:
            full = prefix + "." + k if prefix != "" else k
            result.append([full, self._buffers[k]])
        for k in self._modules:
            child_prefix = prefix + "." + k if prefix != "" else k
            child_bufs = self._modules[k].named_buffers(child_prefix)
            for pair in child_bufs:
                result.append(pair)
        return result

    def state_dict(self, prefix):
        sd = {}
        for k in self._parameters:
            full = prefix + "." + k if prefix != "" else k
            sd[full] = self._parameters[k]
        for k in self._buffers:
            full = prefix + "." + k if prefix != "" else k
            sd[full] = self._buffers[k]
        for k in self._modules:
            child_prefix = prefix + "." + k if prefix != "" else k
            child_sd = self._modules[k].state_dict(child_prefix)
            for ck in child_sd:
                sd[ck] = child_sd[ck]
        return sd

    def num_parameters(self):
        params = self.named_parameters("")
        return len(params)

    def num_buffers(self):
        bufs = self.named_buffers("")
        return len(bufs)


class BatchNorm(Module):
    def __init__(self, num_features):
        Module.__init__(self, "bn")
        self.num_features = num_features
        self.register_parameter("weight", num_features)
        self.register_parameter("bias", num_features)
        self.register_buffer("running_mean", 0)
        self.register_buffer("running_var", 1)
        self.register_buffer("num_batches_tracked", 0)

    def forward(self, x):
        return x


class Conv2d(Module):
    def __init__(self, in_ch, out_ch, kernel):
        Module.__init__(self, "conv")
        self.in_ch = in_ch
        self.out_ch = out_ch
        self.kernel = kernel
        weight_size = out_ch * in_ch * kernel * kernel
        self.register_parameter("weight", weight_size)
        self.register_parameter("bias", out_ch)

    def forward(self, x):
        return x


class ResBlock(Module):
    def __init__(self, channels):
        Module.__init__(self, "resblock")
        self.channels = channels
        conv1 = Conv2d(channels, channels, 3)
        bn1 = BatchNorm(channels)
        conv2 = Conv2d(channels, channels, 3)
        bn2 = BatchNorm(channels)
        self.add_module("conv1", conv1)
        self.add_module("bn1", bn1)
        self.add_module("conv2", conv2)
        self.add_module("bn2", bn2)

    def forward(self, x):
        return x


def main():
    conv = Conv2d(3, 64, 3)
    print("Conv2d param count: " + str(conv.num_parameters()))
    print("Conv2d buffer count: " + str(conv.num_buffers()))

    bn = BatchNorm(64)
    print("BatchNorm param count: " + str(bn.num_parameters()))
    print("BatchNorm buffer count: " + str(bn.num_buffers()))

    block = ResBlock(64)
    print("ResBlock total params: " + str(block.num_parameters()))
    print("ResBlock total buffers: " + str(block.num_buffers()))

    # Check named parameters
    params = block.named_parameters("")
    names = []
    for pair in params:
        names.append(pair[0])
    print("First param name: " + str(names[0]))
    print("Last param name: " + str(names[len(names) - 1]))

    # State dict
    sd = bn.state_dict("")
    print("BN state_dict size: " + str(len(sd)))

    # Nested module state dict
    root = Module("root")
    root.add_module("block", block)
    root.register_parameter("classifier_weight", 1000)
    root.register_parameter("classifier_bias", 1000)
    print("Root total params: " + str(root.num_parameters()))
    root_params = root.named_parameters("")
    has_classifier = False
    for pair in root_params:
        if pair[0] == "classifier_weight":
            has_classifier = True
    print("Root has classifier_weight: " + str(has_classifier))


if __name__ == "__main__":
    main()
