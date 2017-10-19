from datetime import datetime, timedelta
import threading
import time


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

    def map(self, map_function):
        map_stream = MapStream(map_function)
        self.subscribe(map_stream)
        return map_stream

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

    def window(self, time_in_millis, number_of_splits, func_val_to_int):
        split_streams = []
        for num in range(0, number_of_splits):
            split_streams.append(Stream())
        window_stream = Window(time_in_millis, func_val_to_int, split_streams)
        self.subscribe(window_stream)
        return window_stream

    def aggregate(self, func_val_to_val):
        aggregate_stream = AggregateStream(func_val_to_val)
        self.subscribe(aggregate_stream)
        return aggregate_stream


class FilterStream(Stream):

    def __init__(self, filter_function):
        Stream.__init__(self)
        self.filter_function = filter_function

    def receive(self, value):
        if self.filter_function(value):
            self.push(value)


class MapStream(Stream):

    def __init__(self, map_function):
        Stream.__init__(self)
        self.map_function = map_function

    def receive(self, value):
        self.push(self.map_function(value))


class SplitStream(Stream):

    def __init__(self, streams, func_val_to_int):
        self.streams = streams
        self.func_val_to_int = func_val_to_int
        Stream.__init__(self)

    def receive(self, value):
        n = self.func_val_to_int(value)
        self.streams[n % len(self.streams)].push(value)


class SinkStep(Stream):

    def __init__(self, sink_function):
        self.sink_function = sink_function
        Stream.__init__(self)

    def receive(self, value):
        self.sink_function(value)


class Window(Stream):

    def __init__(self, time_in_millis, func_val_to_int, streams):
        self.func_val_to_int = func_val_to_int
        self.streams = streams
        self.time_in_millis = time_in_millis
        self.values = []
        self.thread_running = False
        Stream.__init__(self)

    def get_streams(self):
        return self.streams

    def get_stream(self, index):
        return self.streams[index]

    def _earliest_value(self):
        if len(self.values) == 0:
            return None
        return self.values[0]

    def _is_value_expired(self, timestamped_value):
        expiry_time = timestamped_value.time + timedelta(milliseconds=self.time_in_millis)
        now = datetime.now()
        return now > expiry_time

    def _time(self):
        while self.thread_running:
            earliest_value = self._earliest_value()
            if earliest_value is None:
                time.sleep(self.time_in_millis/2000.0)
                continue
            else:
                future = earliest_value.time + timedelta(milliseconds=self.time_in_millis)
                sleep_time = future - datetime.now()
                if sleep_time.seconds < 0:
                    continue
                time.sleep(sleep_time.seconds)
                self.values.remove(earliest_value)
                self._send(earliest_value)
                continue

    def _send(self, t_val):
        values = []
        for num in range(0, len(self.streams)):
            values.append([])

        for timestamped_value in self.values:
            num = self.func_val_to_int(timestamped_value.value) % len(self.streams)
            values[num].append(timestamped_value.value)

        num = self.func_val_to_int(t_val.value) % len(self.streams)
        self.streams[num].receive(values[num])

        # Push to subscribers of all content
        self.push(values)

    def receive(self, value):
        if self.thread_running is False:
            self.thread_running = True
            threading.Thread(target=self._time).start()
        t_value = TimestampedValue(value, datetime.now())
        self.values.append(t_value)
        self._send(t_value)

    def batch(self, func_vals_to_val):
        batch_stream = BatchStream(self.time_in_millis, func_vals_to_val)
        self.subscribe(batch_stream)


class BatchStream(Stream):

    def __init__(self, time_in_millis, func_vals_to_val):
        Stream.__init__(self)
        self.time_in_millis = time_in_millis
        self.func_vals_to_val = func_vals_to_val
        self.thread_running = False
        self.values = []
        if self.thread_running is False:
            self.thread_running = True
            threading.Thread(target=self._time).start()

    def receive(self, values):
        self.values = values

    def _time(self):
        while self.thread_running:
            time.sleep(self.time_in_millis/1000.0)
            self._run()

    def _run(self):
        for num in range(0, len(self.values)):
            self.push(self.func_vals_to_val(self.values[num], num))


class AggregateStream(Stream):

    def __init__(self, func_val_to_val):
        self.func_val_to_val = func_val_to_val
        Stream.__init__(self)

    def receive(self, values):
        self.push(self.func_val_to_val(values))


class TimestampedValue:

    def __init__(self, value, time):
        self.value = value
        self.time = time


class Source(Stream):

    def __init__(self, values):
        self.values = values
        Stream.__init__(self)

    def start(self):
        for value in self.values:
            self.push(value)


class PolledSource(Stream):

    def __init__(self, time_interval_millis, func_data_provider):
        self.time_interval_millis = time_interval_millis
        self.func_data_provider = func_data_provider
        self.thread_running = False
        Stream.__init__(self)

    def _get_data(self):
        while self.thread_running:
            self.push(self.func_data_provider())
            time.sleep(self.time_interval_millis/1000.0)

    def start(self):
        self.thread_running = True
        threading.Thread(target=self._get_data())


def print_val(value):
    print str(datetime.now()) + ":    " + str(value)
