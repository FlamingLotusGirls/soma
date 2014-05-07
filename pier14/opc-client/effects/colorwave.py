import math
import random
import numpy as np
from effectlayer import EffectLayer 
from colorsys import hsv_to_rgb, rgb_to_hsv

CENTER_FREQ = 1.2 #Hz

def jitter(size=0.15):
    return random.random() * size - (size/2)

def randomColor():
    return np.array(hsv_to_rgb(random.random(),0.75+jitter(0.5),1))

def hsvColorAdd(npRgbColor, hsv):
    """
    Add a hsv tuple to a color expressed an RGB numpy array
    Return an RGB numpy array value.
    """
    hsv1 = rgb_to_hsv(*tuple(npRgbColor))
    newhsv = np.array(hsv1) + np.array(hsv)
    return np.array(hsv_to_rgb(*tuple(newhsv)))





class ColorWave(EffectLayer):
    def __init__(self, model):
        self.model = model
        self.phases = [ random.random() for i in range(self.model.numLEDs) ]
        self.frequencies = [CENTER_FREQ for i in range(self.model.numLEDs) ]
        self.color = randomColor()
        self.colors = [ hsvColorAdd(self.color, (jitter(0.1) % 1.0, jitter(0.5), jitter(0.5)))  for i in range(self.model.numLEDs)]

    def render(self, model, params, frame):
        for i in range(len(self.phases)):
            self.phases[i] += jitter()
            self.frequencies[i] += jitter(0.00000000001)


        for i in range(self.model.numLEDs):
            color = self.colors[i] + [ jitter(0.02), 0, 0]
            frame[i] = color * (math.sin(math.pi*(params.time*self.frequencies[i] + self.phases[i])))**2

