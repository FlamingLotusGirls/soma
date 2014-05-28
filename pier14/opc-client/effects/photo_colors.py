import math
import random
import numpy
from effectlayer import *
import png

class PhotoColorsLayer(EffectLayer):

    def __init__(self, model):
        reader = png.Reader("images/light_fixture.png")
        print "reading photo"
        photo = reader.read_flat()
        print "width", photo[0]
        print "height", photo[1]
        print "metadata", photo[3]
        metadata = photo[3]
        if metadata["alpha"]:
            self.stepSize = 4
        else:
            self.stepSize = 3
        self.photoSize = photo[0] * photo[1]
        self.pixels = photo[2]
        self.offset = 0

    def getPixel(self, position):
        position = position % self.photoSize
        r = self.pixels[position*self.stepSize]
        g = self.pixels[position*self.stepSize + 1]
        b = self.pixels[position*self.stepSize + 2]
        return numpy.array([r/255., g/255., b/255.]) 

    def render(self, model, params, frame):
        for i in range(model.numLEDs):
            frame[i] = self.getPixel(self.offset + i)
        self.offset += 1