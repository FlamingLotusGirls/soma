import math
import random
import numpy
from effectlayer import *

class RandomPhaseLayer(EffectLayer):

    def __init__(self, model):
        self.frequency = 0.5 
        self.phase = [0]*model.numLEDs
        for i in range(model.numLEDs):
            self.phase[i] = random.random()
        self.color = numpy.array((0.7, 0.9, 0.8))
        self.randomness = 1


    def render(self, model, params, frame):
        if params.buttonState[0]:
            self.randomness -= 0.005
            # self.randomness = cachedValue-params.buttonTimeSinceStateChange[0]/10
            if self.randomness < 0:
                self.randomness = 0
        else:
            self.randomness += 0.01
            # self.randomness = cachedValue+params.buttonTimeSinceStateChange[0]/10
            if self.randomness > 1:
                self.randomness = 1

        for i in range(model.numLEDs):
            frame[i] += self.color * (math.sin(math.pi*(params.time*self.frequency + self.randomness*self.phase[i])))**2
            
        
