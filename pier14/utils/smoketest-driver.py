#!/usr/bin/python

import argparse
from subprocess import call

parser = argparse.ArgumentParser(description='Description of your program')
parser.add_argument('-p','--port', help='path to serial port', required=False)
args = vars(parser.parse_args())

command = ['./send.pl', '-v']
if args['port']: 
	command.append('--device ' + args['port'])

command.append(' --loop 0.75 ')

sequence = [
        ('00', '00', '00','00'),
        ('ff', '00', '00','00'),
        ('00', '01', '00','00'),
        ('00', '00', '01','00'),
        ('00', '00', '00','00'),
        ('ff', '00', '00','00'),
        ('00', 'ff', '00','00'),
        ('00', '00', 'ff','00'),
        ('00', '00', '00','00'),
        ('ff', 'ff', 'ff','00'),
        ('00', '00', '00','00')
    ]

for frame in sequence:
	sequence_string = ""
	sequence_string += "rgblatch,0xff," 
	sequence_string += frame[0] + '_' + frame[1] + '_' + frame[2]
	command.append(sequence_string)

call(command)