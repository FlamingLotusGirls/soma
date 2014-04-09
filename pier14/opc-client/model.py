import json
import numpy

class Model(object):
    """
    Minimal base class for a model describing the layout of the LEDs in physical space. 
    """
    def __init__(self, num_leds):
        self.numLEDs = num_leds


class SomaModel(Model):

    def __init__(self, points_filename='points.json'):
        self.json = json.load(open(points_filename))
        
        # pull raw positions from JSON
        self.rawPoints = self._nodesFromJSON()

        Model.__init__(self, len(self.rawPoints))

        #sort points into regions
        self.regionIndicies = {}
        for i in range(len(self.json)):
            region = self.json[i]['region']
            idx_list = self.regionIndicies.setdefault(region, [])
            idx_list.append(i)


        # get indices of nodes in different portions of the sculpture
        self.axonIndices = self._getAxonIndices()
        self.lowerIndices = self._getLowerIndices()
        self.upperIndices = self._getUpperIndices()

        # Axis-aligned bounding box, for understanding the extent of the coordinate space.
        # The minimum and maximum are 3-vectors in the same coordinate space as self.nodes.
        self.minAABB = [ min(v[i] for v in self.rawPoints) for i in range(3) ]
        self.maxAABB = [ max(v[i] for v in self.rawPoints) for i in range(3) ]

        # # Scaled Nodes: It's easier to work with coordinates in the range [0, 1], so scale them according
        # # to the AABB we discovered above.
        self.nodes = numpy.array([[ (v[i] - self.minAABB[i]) / (self.maxAABB[i] - self.minAABB[i]) for i in range(3) ] for v in self.rawPoints])

        # self._testPrint()

    def _nodesFromJSON(self):
        points = []
        for val in self.json:
            points.append(val['point'])
        return numpy.array(points)

    def _getAxonIndices(self):
        return self.regionIndicies['Axon']

    def _getLowerIndices(self):
        return self.regionIndicies['Lower']

    def _getUpperIndices(self):
        return self.regionIndicies['Upper']

    def _testPrint(self):
        print self.upperIndices
        # print self.rawPoints[self.upperIndices]
