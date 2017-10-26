from random import randint


class TemperatureSource:

    def __init__(self, start, min_temp, max_temp, change_ratio):
        self.min_temp = min_temp
        self.max_temp = max_temp
        self.current = start
        self.change_ratio = change_ratio

    def get(self):
        rand_val = randint(self.min_temp, self.max_temp)
        new_val = (rand_val - self.current) * self.change_ratio + self.current
        self.current = new_val
        return new_val
