#!/bin/bash

SERIAL_PORT = ARGV[1]
cd $(dirname $0)

./send.pl --device /dev/tty.usbserial-A602HV6D -v --loop 0.75 \
	rgblatch,0xff,70_70_70	\
	
