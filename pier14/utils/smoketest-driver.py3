#!/usr/bin/python3

import time
import argparse
from subprocess import call

parser = argparse.ArgumentParser(description='Description of your program')
parser.add_argument('-p','--port', dest='port', type=str, help='full path to serial port', required=True)
args = vars(parser.parse_args())
if args == None:
        print(' Arguments incorrect')
        parser.print_help()
        exit()



sequence = [ '000000',
             '700000',
             '007000',
             '000070',
             '707000',
             '700070',
             '007070' ]

while True:
        for frame in sequence:
                command = ['python', 'send.py', '--port', args['port'], '--color', frame ]
                call(command)
                time.sleep(0.75)