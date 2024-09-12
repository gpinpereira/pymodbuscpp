import random
import cppobjects #need to be here to that channel can be imported
from abc import ABCMeta, abstractmethod
import math, time

class Behavior:

    def __init__(self):
        self.value = 0
        self.channel = None
        #self.params = params

    def _setChannelObj(self, channel):
        self.channel = channel

    def _findChannelbyName(self, name):
        obj = self.channel.findChannelbyName(name)
        if obj == None:
            raise Exception(f"Channel {name} not found")
        else:
            return obj

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
        self.range = float(self.params[1])
        
    def updateValue(self):
        self.value = self.constant + random.uniform(-self.range, self.range)

    def setValue(self, value):
        self.constant = value

    def __str__(self):
        return "Bsetpoint"



class Bcopy(Behavior):

    def __init__(self, params):
        super().__init__()
        self.params = params
        self.copy_channel_name = self.params[0]
        self.ref_channel = None

        
    def updateValue(self):
        if self.ref_channel == None:
            self.ref_channel = self._findChannelbyName(self.copy_channel_name)

        self.value = self.ref_channel.getBehaviour().getValue()

    def __str__(self):
        return "Bcopy"


class Bsinwave(Behavior):

    def __init__(self, params):
        super().__init__()
        self.params = params
        self.constant = float(self.params[0])
        self.amp = float(self.params[1])
        self.freq = float(self.params[2])
        self.phase = float(self.params[3])

    def updateValue(self):            
        self.value = self.constant + self.amp*math.sin(time.time()*self.freq+self.phase)

    def __str__(self):
        return "Bsinwave"