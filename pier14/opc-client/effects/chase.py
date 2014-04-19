import numpy as np
from effectlayer import EffectLayer
from model import SomaModel
import sys

CYCLE_TIME = 2
OFFSET = 0
TRIGGER_THRESHOLD = 0.05
COLOR = (0,0,1)

class AxonChaseLayer(EffectLayer):

    def __init__(self, cycle_time=CYCLE_TIME, offset=OFFSET, color=COLOR, trigger_threshold=TRIGGER_THRESHOLD):
        self.color = np.array(color)
        self.cycle_time = cycle_time
        self.offset = offset
        self.trigger_threshold = trigger_threshold

        # Extract and normalize axon point positions in one dimension
        model = SomaModel()
        x_coords = [model.nodes[i][1] for i in model.axonIndices]
        min_ = min(x_coords)
        max_ = max(x_coords)
        self.normalized_x_coords = [(coord - min_) / (max_ - min_) for coord in x_coords]

    def render(self, model, params, frame):
        phi = self.phi(params)

        for i, x in zip(model.axonIndices, self.normalized_x_coords):
            # node = model.nodes[i]
            dist = abs(x - phi)
            if dist < self.trigger_threshold:
                print "%d %f %f" % (i, x, dist)
                frame[i] = self.color * (1-dist)

        return frame

    def phi(self, params):
        """ 
        Converts current time + time offset into phi (oscillatory phase parameter in range [0,1]) 
        """
        phi = ((params.time + self.offset) % self.cycle_time)/self.cycle_time + 0.01
        return phi
