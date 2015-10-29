#!/bin/bash

# Manually simulate button presses on the sculpture.

echo "Ready"

while true
do
	read -s i
	if test "$i" = "r"
	then
		echo "Restarting"
		sudo systemctl restart opc-client.service
		continue
	fi

	echo -n "$(date +%H:%M:%S)  Pressed "
	touch /var/run/soma/buttonA

	#read -s i
	sleep 0.5
	echo "Released"
	rm -f /var/run/soma/buttonA
done
