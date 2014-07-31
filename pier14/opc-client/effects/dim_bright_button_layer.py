from effectlayer import EffectLayer 
from colorsys import hsv_to_rgb, rgb_to_hsv

def set_brightness(color, clamp_factor, scaling_factor):
    hsv = rgb_to_hsv(*color)
    shifted_brightness = hsv[2] - 0.5
    shifted_brightness *= clamp_factor
    hsv = ( hsv[0], hsv[1], (shifted_brightness + 0.5)*scaling_factor )
    return hsv_to_rgb(*hsv)

class DimBrightButtonLayer(EffectLayer):
    """
    This layer dims the brightness when the button is held down, and creates a bright pulse when released
    """
    def __init__(self):
        self.clamp_factor = 0.5
        self.brightness = 1

    def render(self, model, params, frame):
        
        if params.buttonState[0] or params.buttonState[1]: 
            self.brightness -= 0.03
            if self.brightness < 0.2:
                self.brightness = 0.2
        else:
            self.brightness = 1

        for i in range(model.numLEDs):
            frame[i] = set_brightness(frame[i], self.clamp_factor, self.brightness)



