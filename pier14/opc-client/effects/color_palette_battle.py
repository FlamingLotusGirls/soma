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
        self.colorChangesInProgress = []
        self.buttonColors = [self.palette[0], self.palette[-1]]
        self.buttonDown = [False]*2
        self.axonChaseStartTime = None

    def initPalette(self, model):
        self.palette = self.paletteLibrary.getPalette()
        self.colors = [random.choice(self.palette) for i in range(model.numLEDs)]

    def render(self, model, params, frame):
        if self.axonChaseStartTime:
            self.renderAxonChase(model, params, frame)
        else:
            self.renderRestingState(model, params, frame)

    def renderRestingState(self, model, params, frame):
        if not self.lastFrameTime:
            self.lastFrameTime = params.time
        delta = params.time - self.lastFrameTime
        self.lastFrameTime = params.time

        self.waitTimes -= delta

        timedOut = numpy.where(self.waitTimes < 0)

        for i in range(len(params.buttonState)):
            if(params.buttonState[i] and not self.buttonDown[i]):
                self.buttonDown[i] = True
                self.initiateColorChange(self.buttonColors[i], params.time, model)
            if not params.buttonState[i]:
                self.buttonDown[i] = False

        for colorChange in self.colorChangesInProgress:
            if colorChange['fadeOut']:
                self.colors[colorChange['index']] = self.fadeToColor(self.colors[colorChange['index']], [1,1,1])
                if numpy.array_equal(self.colors[colorChange['index']], [1,1,1]):
                    colorChange['fadeOut'] = False

            else:
                self.colors[colorChange['index']] = self.fadeToColor(self.colors[colorChange['index']], colorChange['color'])
                if numpy.array_equal(self.colors[colorChange['index']], colorChange['color']):
                    self.colorChangesInProgress.remove(colorChange)

                
        for i in timedOut[0]:
            self.colors[i] = random.choice(self.palette)
            self.waitTimes[i] = random.expovariate(self.characteristicTime)

        frame[:] = self.colors

    def initiateColorChange(self, color, time, model):
        indices = random.sample(model.lowerIndices, len(model.lowerIndices))
        indexToChange = -1
        for i in range(len(indices)):
            if (self.colors[indices[i]] != color).any():
                if indices[i] not in [colorChange['index'] for colorChange in self.colorChangesInProgress]:
                    indexToChange = i
                    break
        if indexToChange > -1:
            # self.colors[indices[indexToChange]] = color
            print indices[indexToChange]
            #self.colors[indices[indexToChange]] = color
            self.colorChangesInProgress.append({'color': color, 'fadeOut': True, 'index': indices[indexToChange]})
        else:
            self.axonChaseStartTime = time
            print 'YOU WIN'

    def renderAxonChase(self, color, params, frame):
        pass

    def fadeToColor(self, fromColor, toColor):
        stepSize = 0.008
        diff = numpy.subtract(toColor, fromColor)
        maxDifference = max(numpy.absolute(diff))
        if maxDifference > stepSize:
            amountToAdd = numpy.multiply(stepSize/maxDifference, diff)
            return numpy.add(fromColor, amountToAdd)
        else:
            return toColor



