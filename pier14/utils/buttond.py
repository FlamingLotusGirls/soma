#!/usr/bin/env python
import os
import serial # use pyserial
import re

SERIAL_DEVICE = '/dev/ttyO1'
MSG_PATTERN = re.compile('(A|B)(0|1)')
BUTTON_DEV_PATH = '/dev/button'

uart = serial.Serial(SERIAL_DEVICE, baudrate=9600)

if not os.path.exists(BUTTON_DEV_PATH):
    os.mkdir(BUTTON_DEV_PATH)

def touch(fname, times=None):
    with open(fname, 'a'):
        os.utime(fname, times)

def handle_message(msg):
    match = MSG_PATTERN.match(msg)
    if not match:
        # Not a valid message
        print "%s is not a valid message" % msg
        return 1
    button_id = match.group(1)
    button_state = match.group(2)
    state_file = os.path.join(BUTTON_DEV_PATH, button_id)
    if button_state == '1' and not os.path.exists( state_file ):
        touch(state_file)
        print "CREATED %s" % state_file
    elif button_state == '0' and os.path.exists( state_file):
        os.unlink(state_file)
        print "UNLINKED %s" % state_file




if __name__ == '__main__':
    while True:
        char = uart.read(1)
        if char == '!':
            msg = ""
        elif char == '.':
            handle_message(msg)
        elif char == '\n':
            continue
        else:
            msg += char
