#!/bin/bash

SERIAL_PORT = ARGV[1]
cd $(dirname $0)

./send.pl --device /dev/tty.usbserial-A602HXJY -v --loop 0.75 \
	rgblatch,0xff,10_10_10	\
	rgblatch,0xff,00_00_00	\
