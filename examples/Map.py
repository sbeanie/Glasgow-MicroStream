from Stream import *


def int_to_word(value):
    if value == 1:
        return "One"
    elif value == 2:
        return "Two"
    elif value == 3:
        return "Three"
    else:
        return "Mighty big"


topology = Topology()

source = Source([1, 2, 3, 4])

topology.add_stream(source)

mapped_stream = source.map(int_to_word)

mapped_stream.sink(print_val)

topology.run()

