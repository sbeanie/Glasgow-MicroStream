from Stream import *

topology = Topology()

source = Source([1, 2, 3, 4, 5, 6, 7, 8, 9, 10])

topology.add_stream(source)

stream_splits = source.split(2, lambda x: x % 2 != 0)


class IndexedStreamPrinter:

    def __init__(self, index):
        self.index = index

    def print_val(self, value):
        print "Stream index '" + str(self.index) + "': " + str(value)


for num in range(0, len(stream_splits)):
    indexed_stream_printer = IndexedStreamPrinter(num)
    stream_splits[num].sink(indexed_stream_printer.print_val)

topology.run()

