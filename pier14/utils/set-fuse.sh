#!/bin/bash

set -x
avrdude -c avrispv2 -P /dev/tty.usbmodem00079721 -p m88p       \
	    -U  hfuse:w:0xdc:m     	\
	    -U  lfuse:w:0xe7:m     	\
	    -U  efuse:w:0x01:m
