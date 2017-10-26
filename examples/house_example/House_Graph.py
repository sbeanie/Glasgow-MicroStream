from Stream import *
from TemperatureProvider import TemperatureSource


def mean(numbers):
    return float(sum(numbers)) / max(len(numbers), 1)


class Flat:

    def __init__(self, topology, flat_name):
        self.flat_name = flat_name
        self.topology = topology

        self.bedroom_1_temp_source = PolledSource(1000, TemperatureSource(21, 14, 33, 0.05).get)
        self.bedroom_2_temp_source = PolledSource(1000, TemperatureSource(23, 16, 31, 0.05).get)
        self.bedroom_3_temp_source = PolledSource(1000, TemperatureSource(21, 8, 28, 0.05).get)
        self.bedroom_union = self.bedroom_1_temp_source.union([self.bedroom_2_temp_source, self.bedroom_3_temp_source])

        # The kitchen heats up quick so has a high change ratio
        self.kitchen_temp_source = PolledSource(1000, TemperatureSource(24, 19, 36, 0.09).get)

        # The bathroom window is always open
        self.bathroom_temp_source = PolledSource(1000, TemperatureSource(19, 17, 24, 0.03).get)

        self.flat_union = self.bedroom_union.union([self.bathroom_temp_source, self.kitchen_temp_source])

        topology.add_stream(self.bedroom_1_temp_source)
        topology.add_stream(self.bedroom_2_temp_source)
        topology.add_stream(self.bedroom_3_temp_source)

        topology.add_stream(self.kitchen_temp_source)

        topology.add_stream(self.bathroom_temp_source)

        bedroom_1_window = self.bedroom_1_temp_source.window(10000, 1, lambda x: 0)
        self.bedroom_1_average_temp_stream = bedroom_1_window.batch(lambda values, partition: mean(values))

        bedroom_2_window = self.bedroom_2_temp_source.window(10000, 1, lambda x: 0)
        self.bedroom_2_average_temp_stream = bedroom_2_window.batch(lambda values, partition: mean(values))

        bedroom_3_window = self.bedroom_3_temp_source.window(10000, 1, lambda x: 0)
        self.bedroom_3_average_temp_stream = bedroom_3_window.batch(lambda values, partition: mean(values))

        bathroom_window = self.bathroom_temp_source.window(10000, 1, lambda x: 0)
        self.bathroom_average_temp_stream = bathroom_window.batch(lambda values, partition: mean(values))

        kitchen_window = self.kitchen_temp_source.window(10000, 1, lambda x: 0)
        self.kitchen_average_temp_stream = kitchen_window.batch(lambda values, partition: mean(values))

        bedroom_union_window = self.bedroom_union.window(10000, 1, lambda x: 0)
        self.bedroom_union_average_temp_stream = bedroom_union_window.batch(lambda values, partition: mean(values))

        flat_union_window = self.flat_union.window(10000, 1, lambda x: 0)
        self.flat_union_average_temp_stream = flat_union_window.batch(lambda values, partition: mean(values))

    def add_print_sinks(self, prefix):
        self.bedroom_1_temp_source.sink(ValPrinter(prefix + self.flat_name + " Bedroom 1").print_val)
        self.bedroom_2_temp_source.sink(ValPrinter(prefix + self.flat_name + " Bedroom 2").print_val)
        self.bedroom_3_temp_source.sink(ValPrinter(prefix + self.flat_name + " Bedroom 3").print_val)

        self.bedroom_1_average_temp_stream.sink(ValPrinter(prefix + self.flat_name + " Bedroom 1 AVG").print_val)
        self.bedroom_2_average_temp_stream.sink(ValPrinter(prefix + self.flat_name + " Bedroom 2 AVG").print_val)
        self.bedroom_3_average_temp_stream.sink(ValPrinter(prefix + self.flat_name + " Bedroom 3 AVG").print_val)

        self.bedroom_union.sink(ValPrinter(prefix + self.flat_name + " Bedrooms Union").print_val)
        self.bedroom_union_average_temp_stream.sink(ValPrinter(prefix + self.flat_name + " Bedrooms AVG").print_val)

        self.kitchen_temp_source.sink(ValPrinter(prefix + self.flat_name + " Kitchen").print_val)
        self.kitchen_average_temp_stream.sink(ValPrinter(prefix + self.flat_name + " Kitchen AVG").print_val)

        self.bathroom_temp_source.sink(ValPrinter(prefix + self.flat_name + " Bathroom").print_val)
        self.bathroom_average_temp_stream.sink(ValPrinter(prefix + self.flat_name + " Bathroom AVG").print_val)

        self.flat_union.sink(ValPrinter(prefix + self.flat_name + " Flat Union").print_val)
        self.flat_union_average_temp_stream.sink(ValPrinter(prefix + self.flat_name + " Flat AVG").print_val)


topology = Topology()

flat_1 = Flat(topology, "Flat 1")
flat_1.add_print_sinks("")

flat_2 = Flat(topology, "Flat 2")
flat_2.add_print_sinks("")

topology.run()

