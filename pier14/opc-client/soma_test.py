from model import SomaModel
from renderer import Renderer
from controller import AnimationController
from effectlayer import *
from playlist import Playlist
from threads import PlaylistAdvanceThread
from random import random
from math import *


# Just a test to make sure that I can reference sections of the new model
class SomaTestLayer(EffectLayer):
    lower = True
    def render(self, model, params, frame):
        if random() > 0.9:
            self.lower = not self.lower
        if self.lower:
            frame[model.lowerIndices] = [1,.5,.5]
        else:
            frame[model.upperIndices] = 1


# A sine wave in the X direction
class SineWaveLayer(EffectLayer):
    startTime = None

    def __init__(self, period = 1.0, color = (1, 1, 1)):
        self.period = period
        self.color = numpy.array(color)

    def render(self, model, params, frame):
        if not self.startTime:
            self.startTime = params.time

        elapsedTime = (params.time - self.startTime)

        # model.nodes[0] is the X coordinates in [0,1] range
        # elapsedTime / self.period will increment by 1/period every second
        radians = (model.nodes[:,0] + elapsedTime / self.period) * 2 * pi

        cosines = numpy.cos(radians)

        # the frame is Nx3 (R/G/B values for each of the N LEDs).
        # reshape(-1,1) converts cosines from a 1D to an Nx1 2D array so it can be
        # multplied with the color array to yield a Nx3 array
        frame[:] = cosines.reshape(-1,1) * self.color


if __name__ == '__main__':

    # model = SomaModel('../../cad/SomaPointParsing/input_points.json')
    model = SomaModel('test-points.json')

    # a playlist. each entry in a playlist can contain one or more effect layers
    # (if more than one, they are all rendered into the same frame...mixing method 
    # is determined by individual effect layers' render implementations)
    playlist = Playlist([
        [
            SineWaveLayer(color = (0.2, 0.5, 1)),
            # SomaTestLayer(),
            # ColorBlinkyLayer(),
        ],
        # [
        #     SnowstormLayer(),
        # ],
    ])

    # master parameters, used in rendering and updated by playlist advancer thread
    masterParams = EffectParameters()

    # the renderer manages a playlist (or dict of multiple playlists), as well as transitions
    # and gamma correction
    renderer = Renderer(playlists={'all': playlist}, gamma=2.2)

    # the controller manages the animation loop - creates frames, calls into the renderer
    # at appropriate intervals, updates the time stored in master params, and sends frames
    # out over OPC
    controller = AnimationController(model, renderer, masterParams)

    # a thread that periodically advances the active playlist within the renderer.
    # TODO: example to demonstrate swapping between multiple playlists with custom fades
    advancer = PlaylistAdvanceThread(renderer, switchInterval=10)
    advancer.start()

    # go!
    controller.drawingLoop()
