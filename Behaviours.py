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

        print("Bsetpoint: ", params, channel)


    def getValue(self):
        self.value = random.uniform(-1,1)
        if self.value>0:
            return True
        return False

    def setValue(self, value):
        self.value = value+10

    def __str__(self):
        return "Bsetpoint"

