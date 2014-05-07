import math
import random
import numpy
from effectlayer import *

class RandomPhaseLayer(EffectLayer):

    def __init__(self, model):
        self.frequency = 0.5 # base frequency of blinking in Hz
        self.period = 2  # seconds
        self.phase = [0]*model.numLEDs
        for i in range(model.numLEDs):
            self.phase[i] = random.random()
        self.color = numpy.array((0.7, 0.9, 0.8))
        self.randomness = 1
        self.lifecycle = "start"
        self.chaseBlinks = -1


    def render(self, model, params, frame):
        if self.lifecycle == "start":
            if params.buttonState[0] or params.buttonState[1]:
                self.lifecycle = "buttonDown"
                print "button down!"
            else:
                self.computeDodecaValues(params.time, 1, model.lowerIndices, frame)
                self.computeDodecaValues(params.time, 1, model.upperIndices, frame)
        
        if self.lifecycle == "buttonDown":
            if not (params.buttonState[0] or params.buttonState[1]):
                self.lifecycle = "decay"
                print "button back up!"
            else:
                self.randomness -= 0.005
                if self.randomness < 0:
                    self.randomness = 0
                    self.lifecycle = "chase"
                    print "starting chase!"
                    self.chaseStartTime = params.time
                self.computeDodecaValues(params.time, self.randomness, model.lowerIndices, frame)
                self.computeDodecaValues(params.time, 1, model.upperIndices, frame)

        if self.lifecycle == "chase":

            # synchronize start of chase cycle with minimum intensity of lower dodeca
            lowerDodecaCyclePosition = (params.time % self.period)
            if self.chaseBlinks < 0:
                if lowerDodecaCyclePosition < 0.95:
                    print "chase ready!"
                    self.chaseBlinks = 0
            elif self.chaseBlinks == 0:
                if lowerDodecaCyclePosition > 0.95:
                    self.chaseStartTime = params.time
                    print "launch the chase!"
                    self.chaseBlinks = 1
            else:
                self.computeAxonValues(params.time, model.axonIndices, frame)

            self.computeDodecaValues(params.time, 0, model.lowerIndices, frame)
            self.computeDodecaValues(params.time, 1, model.upperIndices, frame)




        # if params.buttonState[0]:
        #     self.randomness -= 0.005
        #     if self.randomness < 0:
        #         self.randomness = 0
        #         params.trigger = True
        # else:
        #     params.trigger = False
        #     self.randomness += 0.01
        #     if self.randomness > 1:
        #         self.randomness = 1

        # self.computeValues(params.time, self.randomness, model.lowerIndices, frame)
        # self.computeValues(params.time, 1, model.upperIndices, frame)


    def computeDodecaValues(self, time, randomness, indices, frame): 
        for i in indices:
            frame[i] += self.color * (math.sin(math.pi*(time*self.frequency + randomness*self.phase[i])))**2

    def computeAxonValues(self, time, indices, frame):
        for i in indices:
            frame[i] += self.color * (math.sin(math.pi*(time*self.frequency)))**2

        
