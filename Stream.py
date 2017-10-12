import random


class Source:
    def __init__(self):
        pass

    def get(self): pass


class Stream:

    def __init__(self, source):
        self.source = source

    def get(self):
        return self.source.get()

    def printStream(self): pass

    def split(self, values, f):
        sc = StreamContainer(self)
        streams = []
        for num in range(0, values):
            streams.append(SplitStream(sc, num, f))
        return streams

    def filter(self, f):
        return FilterStream(self, f)


class StreamContainer():
    vals = []

    def __init__(self, stream):
        self.stream = stream

    def get(self, num, f):
        for val in self.vals:
            if f(val) == num:
                self.vals.remove(val)
                return val
        nextval = self.stream.get()
        while nextval is not None:
            currentval = nextval
            nextval = self.stream.get()
            if f(currentval) == num:
                return currentval
            else:
                self.vals.append(currentval)


class SplitStream(Stream):

    def __init__(self, streamcontainer, num, f):
        self.streamcontainer = streamcontainer
        self.num = num
        self.f = f

    def get(self):
        self.streamcontainer.get(self.num, self.f)

    def printStream(self):
        nextval = self.get()
        while nextval is not None:
            currentval = nextval
            nextval = self.get()
            print currentval


class FilterStream(Stream):

    def __init__(self, stream, f):
        self.stream = stream
        self.f = f

    def get(self):
        nextval = self.stream.get()
        while nextval is not None:
            currentval = nextval
            nextval = self.stream.get()
            if self.f(currentval):
                 return currentval
        return None

    def printStream(self):
        nextval = self.get()
        while nextval is not None:
            currentval = nextval
            nextval = self.get()
            print currentval


class RandomSource(Source):

    def __init__(self):
        Source.__init__(self)

    def get(self):
        return random.randint(0,10)



randomSource = RandomSource()

s = Stream(randomSource)

# streams = s.split(2, lambda x : x % 2 == 0)
#
# streams[0].printStream()


filter1 = s.filter(lambda x: x % 2 == 0)

filter2 = filter1.filter(lambda x: x > 6)

# filter2.split(lambda x: x == 8)

filter2.printStream()

