import time
from threading import Thread

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