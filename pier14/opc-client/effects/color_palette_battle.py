import math
import random
import numpy
from effectlayer import *
import random

class ColorPaletteBattleLayer(EffectLayer):

    def __init__(self, model):
        self.palette = self.generatePalette([0x441133, 0xcc3344, 0xbb6699, 0xff6655, 0xffddaa])
        self.colors = [random.choice(self.palette) for i in range(model.numLEDs)]    
        self.characteristicTime = 1/20. # average rate of color change for a single LED
        self.waitTimes = numpy.array([random.expovariate(self.characteristicTime) for i in range(model.numLEDs)])
        self.lastFrameTime = None

    def generatePalette(self, hexValues):
        def convertColor(val):
            r = (val & 0xff0000) >> 16
            g = (val & 0x00ff00) >> 8
            b = (val & 0x0000ff)
            return numpy.array([r/255., g/255., b/255.]) 

        return [ convertColor(val) for val in hexValues ] 


    def render(self, model, params, frame):
        if not self.lastFrameTime:
            self.lastFrameTime = params.time
        delta = params.time - self.lastFrameTime
        self.lastFrameTime = params.time

        self.waitTimes -= delta

        timedOut = numpy.where(self.waitTimes < 0)

        accumulatorColors = []
        if(params.buttonState[0]):
            accumulatorColors.append(self.palette[4])
        if(params.buttonState[1]):
            accumulatorColors.append(self.palette[2])

        for i in timedOut[0]:
            if i in model.lowerIndices and len(accumulatorColors):
                self.colors[i] = random.choice(accumulatorColors)
            else:
                self.colors[i] = random.choice(self.palette)
            self.waitTimes[i] = random.expovariate(self.characteristicTime)

        frame[:] = self.colors