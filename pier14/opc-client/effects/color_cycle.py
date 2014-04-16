import math
import random
import numpy
from effectlayer import *

class ColorCycleLayer(EffectLayer):

    def __init__(self):
        self.fadeSpeed = 0.001
        self.colorSpread = 0.1
        self.hueCentralValue = 0

    def render(self, model, params, frame):
        self.hueCentralValue += self.fadeSpeed
        if(self.hueCentralValue>1):
            self.hueCentralValue -= 1
        color = numpy.array(colorsys.hsv_to_rgb(self.hueCentralValue,1,1))
        frame[:] += color
