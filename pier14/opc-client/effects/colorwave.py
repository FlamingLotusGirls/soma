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

def mutateColor(color, hue_jitter=0.1, sat_jitter=0.5, val_jitter=0.5):
    return hsvColorAdd(color, ( jitter(hue_jitter) % 1.0, jitter(sat_jitter), jitter(val_jitter) ))



class ColorWave(EffectLayer):
    def __init__(self, model):
        self.model = model
        self.phases = [ random.random() for i in range(self.model.numLEDs) ]
        self.frequencies = [CENTER_FREQ + jitter(0.05) for i in range(self.model.numLEDs) ]
        self.color = randomColor()
        self.colors = [ mutateColor(self.color) for i in range(self.model.numLEDs)]

    def render(self, model, params, frame):
        for i in range(len(self.phases)):
            # self.phases[i] += jitter()
            self.frequencies[i] += jitter(0.00000000001)


        for i in range(self.model.numLEDs):
            # color = mutateColor(self.colors[i], 0, 0.005, 0)
            color = self.colors[i]
            frame[i] = color * (math.sin(math.pi*(params.time*self.frequencies[i] + self.phases[i])))**2

