import time
import sys
import effectlayer

class InputGetter:
    def update(self, effectparams):
        pass


# Crappy curses-based keyboard button emulator. See init method for key codes.
class KeyboardInputGetter(InputGetter):
    def __init__(self, screen):
        # curses stuff...
        import curses
        screen.nodelay(True) # make getch calls non-blocking
        self.screen = screen

        self.buttonDownKeys = ["d", "k"]
        self.buttonUpKeys = ["e", "i"]
        self.buttonDown = [0, 0] # time that the button was pressed down, or 0 if it's up

        for i in range(len(self.buttonDownKeys)):
            sys.stderr.write("Button #%i down/up: %s/%s\n" % (i, self.buttonDownKeys[i], self.buttonUpKeys[i]))

    def update(self, effectparams):
            # get a character from the curses screen
            char = self.screen.getch()
            try:
                char = chr(char)
            except:
                pass

            for i in range(len(self.buttonDown)):
                # update button-down times stored in this object
                if char == self.buttonDownKeys[i]:
                    if self.buttonDown[i] == 0:
                        self.buttonDown[i] = time.time()
                elif char == self.buttonUpKeys[i]:
                    self.buttonDown[i] = 0
            
                # update button-press-elapsed times in params
                if self.buttonDown[i]:
                    effectparams.buttonTimes[i] = time.time() - self.buttonDown[i]
                    print "button #" + str(i) + ": " + str(effectparams.buttonTimes[i]) + "\n"
                else:
                    effectparams.buttonTimes[i] = 0


# Gets inputs from GPIO pins on Beaglebone Black
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
