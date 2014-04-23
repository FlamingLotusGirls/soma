import time
from threading import Thread
import sys
import tty
import select 

class PlaylistAdvanceThread(Thread):
    """
    Advances the active playlist periodically.
    """
    def __init__(self, renderer, switchInterval):
        Thread.__init__(self)
        self.daemon = True
        self.renderer = renderer
        self.switchInterval = switchInterval
        
    def run(self):
        lastActive = time.time()
        while True:
            if time.time() - lastActive > self.switchInterval:
                self.renderer.advanceCurrentPlaylist()
                lastActive = time.time()
            time.sleep(0.05)


# shamelessly stolen from the internets
class _GetchUnix:
    def __init__(self):
        import tty, sys

    def __call__(self):
        import sys, tty, termios
        fd = sys.stdin.fileno()
        old_settings = termios.tcgetattr(fd)
        try:
            if sys.stdin in select.select([sys.stdin], [], [], 0.01)[0]:
                print "here"
                tty.setraw(sys.stdin.fileno())
                ch = sys.stdin.read(1)
            else:
                ch = 0
        finally:
            termios.tcsetattr(fd, termios.TCSADRAIN, old_settings)
        return ch


class KeyboardMonitorThread(Thread):

    def __init__(self, params):
        Thread.__init__(self)
        self.daemon = True
        self.params = params

        self.getter = _GetchUnix()

        self.buttonDown1 = 0
        self.buttonDown2 = 0


    def run(self):
        while True:
            
            test = self.getter()
            #test = sys.stdin.readline()
            print "this is test: " + str(test)
            if test == "q":
                break
            # elif test == "k":
            #     #button one down
            #     if self.params.button1 == 0:
            #         self.buttonDown1 = time.time()
            #     else:
            #         self.params.button1 = time.time() - self.buttonDown1
            # elif test == "i":
            #     #button one up
            #     print self.params.button1
            #     self.params.button1 = 0
            #     self.buttonDown1 = 0

            
            time.sleep(0.05)