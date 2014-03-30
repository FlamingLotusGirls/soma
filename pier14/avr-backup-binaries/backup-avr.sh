#!/bin/bash
# vim:set ts=4 sw=4 ai et:

if [ -z "$1" ]
then
    echo "Usage: $0 <prefix>" 2>&1
    exit 1
fi

prefix=$1

if test -d $prefix
then
    echo "Directory '$prefix' already exists, aborting" 2>&1
    exit
else
    mkdir $prefix
fi

avrdude -c usbtiny -P usb -p m88p       \
	-U  hfuse:r:$prefix/hfuse.txt:h     \
	-U  lfuse:r:$prefix/lfuse.txt:h     \
	-U  efuse:r:$prefix/efuse.txt:h     \
	-U eeprom:r:$prefix/eeprom.bin:r    \
	-U  flash:r:$prefix/flash.bin:r
