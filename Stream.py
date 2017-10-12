import random


class Supplier:
    def __init__(self):
        pass

    def get(self): pass


class Stream:

    def __init__(self, supplier):
        self.supplier = supplier

    def get(self):
        return self.supplier.get()

    def printStream(self): pass

    def split(self, values, f):
        sc = StreamContainer(self)
        streams = []
        for num in range(0, values):
            streams.append(SplitStream(sc, num, f))
        return streams

    def filter(self, f):
        return FilterStream(self, f)


class StreamContainer:
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
            if f(currentval) == num:
                return currentval
            else:
                self.vals.append(currentval)
                nextval = self.stream.get()


class SplitStream(Stream):

    def __init__(self, streamcontainer, num, f):
        self.streamcontainer = streamcontainer
        self.num = num
        self.f = f

    def get(self):
        return self.streamcontainer.get(self.num, self.f)

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
            if self.f(currentval):
                 return currentval
            nextval = self.stream.get()
        return None

    def printStream(self):
        nextval = self.get()
        while nextval is not None:
            currentval = nextval
            nextval = self.get()
            print currentval


class RandomSupplier(Supplier):

    def __init__(self):
        Supplier.__init__(self)

    def get(self):
        return random.randint(0,10)


class FixedDataSupplier(Supplier):

    vals = [0,1,2,3,4,5,6,7,8,9,10]

    def __init__(self):
        Supplier.__init__(self)

    def get(self):
        if len(self.vals) is 0:
            return None
        else:
            val = self.vals[0]
            self.vals.remove(val)
            return val

randomSource = RandomSupplier()
fixedDataSource = FixedDataSupplier()

s = Stream(fixedDataSource)

streams = s.split(2, lambda x: x % 2 == 0)

streams[1].printStream()
print "Stream finished"
streams[0].printStream()

b = Stream(randomSource)

filter1 = b.filter(lambda x: x % 2 == 0)
filter2 = filter1.filter(lambda x: x > 6)
filter2.printStream()

