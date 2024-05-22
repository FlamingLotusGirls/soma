install:
	apt-get install python-numpy python-png

	install -p -o root -g root -m 755 opc-client		/usr/local/bin
	install -p -o root -g root -m 755 opc-server		/usr/local/bin
	install -p -o root -g root -m 755 opc-watchdog		/usr/local/bin

	install -p -o root -g root -m 644 opc-client.service	/etc/systemd/system
	install -p -o root -g root -m 644 opc-server.service	/etc/systemd/system
	install -p -o root -g root -m 644 opc-watchdog.service	/etc/systemd/system

	install -p -o root -g root -m 755 rc.local		/etc

	systemctl --system daemon-reload
	systemctl enable opc-server.service
	systemctl enable opc-client.service
	systemctl enable opc-watchdog.service
