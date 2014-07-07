# import math
import random
import numpy
# from effectlayer import *
# from effects.color_palette_library import *
# import random

class ColorPaletteLibrary:

    def __init__(self):
        self.palettes = [
            [0x441133, 0xCC3344, 0xBB6699, 0xFF6655, 0xFFDDAA],
            [0x33AA99, 0x66EE88, 0x00DDFF, 0x22FFFF, 0xFFEE55],
            [0x3C3364, 0x3253CF, 0x7E7FBE, 0x728CE4, 0x9EB8FF],
            [0x4C2A14, 0xEC5A00, 0xFFA200, 0xFFFF00, 0xFFFFC3],
            [0x264854, 0x684B92, 0xFF4800, 0x4474F9, 0xE7FF91],
            [0x422E1E, 0xAE9B6F, 0xABA48D, 0x9A8669, 0xECD197],
            [0x517506, 0x68C51C, 0xBEB940, 0xDEE7F0, 0xDDE657],
            [0x334D5C, 0x45B29D, 0xEFC94C, 0xE27A3F, 0xDF4949]
        ]
        
    def getPalette(self):
        randomPalette = random.choice(self.palettes)
        return self.generatePalette(randomPalette)

    def generatePalette(self, hexValues):
        def convertColor(val):
            r = (val & 0xff0000) >> 16
            g = (val & 0x00ff00) >> 8
            b = (val & 0x0000ff)
            return numpy.array([r/255., g/255., b/255.]) 

        return [ convertColor(val) for val in hexValues ] 

