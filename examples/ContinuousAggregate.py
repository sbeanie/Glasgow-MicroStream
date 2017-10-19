from Stream import *


def print_max(values):
    max_val = None
    for val in values:
        if max_val is None:
            max_val = val
        else:
            if val > max_val:
                max_val = val
    print str(max_val) + ": " + str(values)
    return max_val


topology = Topology()

source = Source([1, 2, 3, 4, 5, 6, 7, 8, 9, 10])

topology.add_stream(source)

window = source.window(5000, 2, lambda x: x % 2 != 0)

for stream in window.get_streams():
    stream.aggregate(print_max)

topology.run()
