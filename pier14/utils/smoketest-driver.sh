#!/bin/bash

SERIAL_PORT = ARGV[1]

# this is for older systems that have working python and serial
# cd $(dirname $0)
#
# ./send.pl --device /dev/tty.usbserial-A602HV6D -v --loop 0.75 \
#	rgblatch,0xff,70_70_70	\

while true
do
	python send.py --device $SERIAL_PORT --color 707070
	sleep 1
	python send.py --device $SERIAL_PORT --color ff0000
	sleep 1
	python send.py --device $SERIAL_PORT --color 00ff00
	sleep 1
	python send.py --device $SERIAL_PORT --color 0000ff
	sleep 1
done
	
