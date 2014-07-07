import math
import random
import numpy
from effectlayer import *
from effects.color_palette_library import *
import random

class ColorPaletteBattleLayer(EffectLayer):

    def __init__(self, model):
        self.paletteLibrary = ColorPaletteLibrary()
        self.initPalette(model)
        self.characteristicTime = 1/20. # average rate of color change for a single LED
        self.waitTimes = numpy.array([random.expovariate(self.characteristicTime) for i in range(model.numLEDs)])
        self.lastFrameTime = None

    def initPalette(self, model):
        self.palette = self.paletteLibrary.getPalette()
        self.colors = [random.choice(self.palette) for i in range(model.numLEDs)]    

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