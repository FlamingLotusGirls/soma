#!/bin/bash

avrdude -c usbtiny -P usb -p m88p       \
	    -U  hfuse:w:0xdc:m     	\
	    -U  lfuse:w:0xe7:m     	\
	    -U  efuse:w:0x01:m
