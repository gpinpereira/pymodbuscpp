import random


class Behavior:

    def __init__(self, params, channel):
        self.value = 0
        self.channel = channel
        self.params = params

    def getValue(self):
        return self.value

    def setValue(self, value):
        self.value = value


class Bsetpoint(Behavior):

    def __init__(self, params, channel):
        super().__init__(params, channel)

        self.constant = 0
        print("Bsetpoint: ", params, channel)


    def getValue(self):
        self.value = self.constant + random.uniform(-1,1)
        return self.value

    def setValue(self, value):
        self.constant = value

    def __str__(self):
        return "Bsetpoint"

