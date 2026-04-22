"""
Simplified Flask signal system for round-trip testing.
Based on flask/src/flask/signals.py and blinker patterns.
"""

class Signal:
    def __init__(self, name):
        self.name = name
        self._receivers = []

    def connect(self, receiver):
        self._receivers.append(receiver)

    def send(self, *args):
        results = []
        for r in self._receivers:
            results.append(r(*args))
        return results

    def __repr__(self):
        return "<Signal '" + str(self.name) + "'>"


class Namespace:
    def __init__(self):
        self._signals = {}

    def signal(self, name):
        if name not in self._signals:
            self._signals[name] = Signal(name)
        return self._signals[name]


def main():
    ns = Namespace()
    template_rendered = ns.signal('template-rendered')
    request_started = ns.signal('request-started')
    got_request_exception = ns.signal('got-request-exception')

    print('template_rendered:', template_rendered)
    print('request_started:', request_started)

    received = []

    def on_template_rendered(data):
        received.append('rendered:' + data)
        return 'rendered:' + data

    template_rendered.connect(on_template_rendered)
    template_rendered.send('index.html')
    print('received:', received)
    print('same signal:', ns.signal('template-rendered') == template_rendered)
    return 0


if __name__ == '__main__':
    main()
