#!/bin/bash

# Manually simulate button presses on the sculpture.

echo "Ready"

while true
do
	read -s i
	echo -n "Pressed "
	touch /var/run/soma/buttonA

	read -s i
	echo "Released"
	rm -f /var/run/soma/buttonA
done
