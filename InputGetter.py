import time
import effectlayer

class InputGetter:
    def update(self, effectparams):
        pass

class GpioInputGetter(InputGetter):
    def __init__(self, gpios_list = (30,)):
        self.gpios_list = gpios_list
        self.button_previous_state = [False for gpio in gpios_list]
        self.button_on_set_time = [0.0 for gpio in gpios_list]
        gpio_export = open("/sys/class/gpio/export", "w")
        for gpio in gpios_list:
            gpio_export.write(str(gpio))
        gpio_export.close()

    def update(self, effectparams):
        if (not isinstance(effectparams, EffectParameters)):
            return
        if (not self.file):
            return

        for ix in range(0,len(self.gpios_list)):
            gpio = self.gpios_list[ix]
            filename = "/sys/class/gpio/gpio"+str(gpio)
            try:
                f = open(filename,"r")
                state = {'1': True, '0': False}[f.read()]
                f.close()
            except:
                continue
            effectparams.button_pressed_times[ix] = 0.0
            if (state):
                if (not self.button_previous_state):
                    self.button_on_set_time[ix] = time.time()
                else:
                    effectparams.button_pressed_times[ix] = time.time() - self.button_on_set_time[ix]
            self.button_previous_state = state
