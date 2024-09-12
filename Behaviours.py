import random
import cppobjects #need to be here to that channel can be imported
from abc import ABCMeta, abstractmethod

class Behavior:

    def __init__(self):
        self.value = 0
        self.channel = None
        #self.params = params

    def setChannelObj(self, channel):
        self.channel = channel

    @abstractmethod
    def updateValue(self):
        pass

    @abstractmethod
    def getValue(self):
        return self.value

    @abstractmethod
    def setValue(self, value):
        self.value = value


class Bsetpoint(Behavior):

    def __init__(self, params):
        super().__init__()
        self.params = params
        self.constant = float(self.params[0])
        
    def updateValue(self):
        self.value = self.constant + random.uniform(-3,3)

        #example on how to access other channel properties
        #print(self.channel.findChannelbyName("Ch 1.1.1").getBehaviour().getValue())
        return self.value

    def setValue(self, value):
        self.constant = value

    def __str__(self):
        return "Bsetpoint"

