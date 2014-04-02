from __future__ import print_function
import numpy
import time
import traceback
import colorsys
import random

class EffectParameters(object):
    """Inputs to the individual effect layers. Includes basics like the timestamp of the frame we're
       generating, as well as parameters that may be used to control individual layers in real-time.
       """

    time = 0
    targetFrameRate = 40.0  


class EffectLayer(object):
    """Abstract base class for one layer of an LED light effect. Layers operate on a shared framebuffer,
       adding their own contribution to the buffer and possibly blending or overlaying with data from
       prior layers.

       The 'frame' passed to each render() function is an array of LEDs in the same order as the
       IDs recognized by the 'model' object. Each LED is a 3-element list with the red, green, and
       blue components each as floating point values with a normalized brightness range of [0, 1].
       If a component is beyond this range, it will be clamped during conversion to the hardware
       color format.
       """

    transitionFadeTime = 1.0
    maximum_errors = 5

    def render(self, model, params, frame):
        raise NotImplementedError("Implement render() in your EffectLayer subclass")

    def safely_render(self, model, params, frame):
        if not hasattr(self, 'error_count'):
            self.error_count = 0
        try:
            if self.error_count < EffectLayer.maximum_errors:
                self.render(model, params, frame)
        except Exception as err:
            error_log = open('error.log','a')
            error_log.write(time.asctime(time.gmtime()) + " UTC" + " : ")
            traceback.print_exc(file=error_log)
            print("ERROR:", err, "in", self)
            self.error_count += 1
            if self.error_count >= EffectLayer.maximum_errors:
                print("Disabling", self, "for throwing too many errors")


########################################################
# Simple model-agnostic EffectLayer implementations and examples
########################################################


class MultiplierLayer(EffectLayer):
    """ Renders two layers in temporary frames, then adds the product of those frames
    to the frame passed into its render method
    """
    def __init__(self, layer1, layer2):
        self.layer1 = layer1
        self.layer2 = layer2        
        
    def render(self, model, params, frame):
        temp1 = numpy.zeros(frame.shape)
        temp2 = numpy.zeros(frame.shape)
        self.layer1.render(model, params, temp1)
        self.layer2.render(model, params, temp2)
        numpy.multiply(temp1, temp2, temp1)
        numpy.add(frame, temp1, frame)


class BlinkyLayer(EffectLayer):
    """Test our timing accuracy: Just blink everything on and off every other frame."""

    on = False

    def render(self, model, params, frame):
        self.on = not self.on
        frame[:] += self.on


class ColorBlinkyLayer(EffectLayer):
    on = False
    def render(self, model, params, frame):
        self.on = not self.on
        color = numpy.array(colorsys.hsv_to_rgb(random.random(),1,1))
        if self.on:
            frame[:] += color


class SnowstormLayer(EffectLayer):
    transitionFadeTime = 1.0
    def render(self, model, params, frame):
        numpy.add(frame, numpy.random.rand(model.numLEDs, 1), frame)


class WhiteOutLayer(EffectLayer):
    """ Sets everything to white """

    transitionFadeTime = 0.5
    def render(self, model, params, frame):
        frame += numpy.ones(frame.shape)
            

class GammaLayer(EffectLayer):
    """Apply a gamma correction to the brightness, to adjust for the eye's nonlinear sensitivity."""

    def __init__(self, gamma):
        # Build a lookup table
        self.lutX = numpy.arange(0, 1, 0.01)
        self.lutY = numpy.power(self.lutX, gamma)

    def render(self, model, params, frame):
        frame[:] = numpy.interp(frame.reshape(-1), self.lutX, self.lutY).reshape(frame.shape)
