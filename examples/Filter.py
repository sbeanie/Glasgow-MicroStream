from Stream import *

topology = Topology()

source = Source([1, 2, 3, 4, 5])

topology.add_stream(source)

filtered_stream = source.filter(lambda x: x % 2 == 0)

filtered_stream.sink(print_val)

topology.run()

