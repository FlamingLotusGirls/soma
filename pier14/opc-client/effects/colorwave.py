import math
import random
import numpy
from effectlayer import EffectLayer 
import colorsys

CENTER_FREQ = 1.2 #Hz

def jitter(size=0.15):
    return random.random() * size - (size/2)

class ColorWave(EffectLayer):
    def __init__(self, model):
        self.model = model
        self.phases = [ random.random() for i in range(self.model.numLEDs) ]
        self.frequencies = [CENTER_FREQ for i in range(self.model.numLEDs) ]
        self.color = numpy.array(colorsys.hsv_to_rgb(random.random(),1,1))
        self.colors = [self.color + [jitter(1.2), jitter(0.5), jitter(0.5)] for i in range(self.model.numLEDs)]

    def render(self, model, params, frame):
        for i in range(len(self.phases)):
            self.phases[i] += jitter()
            self.frequencies[i] += jitter(0.00000000001)


        for i in range(self.model.numLEDs):
            color = self.colors[i] + [ jitter(0.02), 0, 0]
            frame[i] = color * (math.sin(math.pi*(params.time*self.frequencies[i] + self.phases[i])))**2

