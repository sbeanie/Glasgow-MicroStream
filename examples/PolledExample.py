from Stream import *


class DataSource:

    def __init__(self, values):
        self.values = values
        self.counter = 0

    def get(self):
        if self.counter >= len(self.values):
            return None
        value = self.values[self.counter]
        self.counter += 1
        return value


topology = Topology()

data_source = DataSource([1, 2, 3, 4, 5])
sourceA = PolledSource(1000, data_source.get)

topology.add_stream(sourceA)

sourceA.sink(print_val)

time.sleep(1)
topology.run()
