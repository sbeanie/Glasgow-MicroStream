from Stream import *

topology = Topology()

sourceA = Source([1, 2, 3, 4, 5])
sourceB = Source([6, 7, 8, 9, 10])

topology.add_stream(sourceA)
topology.add_stream(sourceB)

union_stream = sourceA.union(sourceB)

union_stream.sink(print_val)

topology.run()

