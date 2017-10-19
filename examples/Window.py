from Stream import *


topology = Topology()

source = Source([1, 2, 3, 4, 5, 6, 7, 8, 9, 10])

topology.add_stream(source)

window = source.window(5000, 2, lambda x: x % 2 != 0)

# Using sink on a Window provides all stream output values and subscribes to all changes of the window's streams.
# If you want a corresponding partition use window.get_stream(index).sink(print_val) for example.
window.sink(print_val)

topology.run()
