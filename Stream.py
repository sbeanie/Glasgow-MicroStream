
class Topology:

    def __init__(self):
        self.suppliers = []
        return

    def add_stream(self, supplier):
        self.suppliers.append(supplier)
        return

    def run(self):
        for supplier in self.suppliers:
            supplier.start()

    def print_topology(self):
        for supplier in self.suppliers:
            print ("Supplier" + str(supplier.__class__))
            for subsciber in supplier.subscribers:
                print(subsciber.__class__)


class Stream:

    def __init__(self):
        self.subscribers = []
        return

    def subscribe(self, stream):
        self.subscribers.append(stream)

    def receive(self, value):
        self.push(value)

    def push(self, value):
        for subscriber in self.subscribers:
            subscriber.receive(value)

    def filter(self, f):
        filter_stream = FilterStream(f)
        self.subscribe(filter_stream)
        return filter_stream

    def sink(self, sink_function):
        sink_step = SinkStep(sink_function)
        self.subscribe(sink_step)
        return sink_step

    def split(self, number_of_splits, func_val_to_int):
        split_streams = []
        for num in range(0, number_of_splits):
            split_streams.append(Stream())
        split_stream = SplitStream(split_streams, func_val_to_int)
        self.subscribe(split_stream)
        return split_streams

    def union(self, streams):
        union_stream = Stream()

        try:
            iter(streams)
            for stream in streams:
                stream.subscribe(union_stream)
        except TypeError:
            streams.subscribe(union_stream)

        self.subscribe(union_stream)
        return union_stream


class Source(Stream):

    def __init__(self, values):
        self.values = values
        Stream.__init__(self)

    def start(self):
        for value in self.values:
            self.push(value)


class FilterStream(Stream):

    def __init__(self, filter_function):
        Stream.__init__(self)
        self.filter_function = filter_function

    def receive(self, value):
        if self.filter_function(value):
            self.push(value)


class SplitStream(Stream):

    def __init__(self, streams, func_val_to_int):
        self.streams = streams
        self.func_val_to_int = func_val_to_int
        Stream.__init__(self)

    def receive(self, value):
        n = self.func_val_to_int(value)
        self.streams[n].push(value)


class SinkStep(Stream):

    def __init__(self, sink_function):
        self.sink_function = sink_function
        Stream.__init__(self)

    def receive(self, value):
        self.sink_function(value)


def print_val(value):
    print value


topology = Topology()

source = Source([1, 2, 3, 4, 5])

source2 = Source([6, 7, 8, 9, 10])

topology.add_stream(source)
topology.add_stream(source2)

split_streams = source.split(2, lambda x: x % 2 == 0)

split_streams[0].sink(print_val)
# split_streams[1].sink(print_val)

union_stream = split_streams[1].union(source2)
union_stream.sink(print_val)

topology.run()

