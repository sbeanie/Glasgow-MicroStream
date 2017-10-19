from Stream import *


def print_values_and_key(values, key):
    print "Key " + str(key) + ": " + str(values)
    return values


topology = Topology()

source = Source([1, 2, 3, 4, 5, 6, 7, 8, 9, 10])

topology.add_stream(source)

window = source.window(5000, 2, lambda x: x % 2 != 0)

window.batch(print_values_and_key)

# A batch window timer starts as soon as the batch stream is created.  If you directly run the topology after creating
# a batch window, all values will have expired by the time the first batch is submitted (unless you use a polled source)
time.sleep(1)

topology.run()
