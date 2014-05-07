import numpy as np
from effectlayer import EffectLayer
from model import SomaModel
import sys

CYCLE_TIME = 2
OFFSET = 0
DISTANCE_THRESHOLD = 0.05
COLOR = (1,1,1)

class AxonChaseLayer(EffectLayer):

    def __init__(self, cycle_time=CYCLE_TIME, offset=OFFSET, color=COLOR, distance_threshold=DISTANCE_THRESHOLD, segments=['axon'], direction='forward', blend_mode='replace'):
        self.color = np.array(color)
        self.cycle_time = cycle_time
        self.offset = offset
        self.distance_threshold = distance_threshold
        if direction not in ('forward','reverse'):
            direction = 'forward'
        self.direction = direction

        model = SomaModel()
        #select indices
        indices = []
        if 'axon' in segments or 'all' in segments:
            indices += model.axonIndices 
        if 'upper' in segments or 'all' in segments:
            indices += model.upperIndices 
        if 'lower' in segments or 'all' in segments:
            indices += model.lowerIndices 
        self.indices = indices
        print indices


        # Extract and normalize axon point positions in one dimension
        x_coords = [model.nodes[i][1] for i in self.indices]
        min_ = min(x_coords)
        max_ = max(x_coords)
        self.normalized_x_coords = [(coord - min_) / (max_ - min_) for coord in x_coords]

    def render(self, model, params, frame):
        phi = self.phi(params)
        if self.direction == 'forward':
            phi = 1 - phi

        for i, x in zip(self.indices, self.normalized_x_coords):
            # node = model.nodes[i]
            dist = abs(x - phi)
            if dist < self.distance_threshold:
                # print "%d %f %f" % (i, x, dist)
                frame[i] = self.color * (1-dist)

        return frame

    def phi(self, params):
        """ 
        Converts current time + time offset into phi (oscillatory phase parameter in range [0,1]) 
        """
        phi = ((params.time + self.offset) % self.cycle_time)/self.cycle_time + 0.01
        return phi
