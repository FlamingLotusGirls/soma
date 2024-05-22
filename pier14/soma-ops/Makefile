OPC_SERVER_DIR=../soma/pier14/openpixelcontrol

all:
	$(MAKE) -C $(OPC_SERVER_DIR)

install: all
	install -p -o root -g root -m 755 $(OPC_SERVER_DIR)/soma_server	/usr/local/bin/soma-server
	install -p -o root -g root -m 755 bin/init.d-ubrain-clock	/etc/init.d/ubrain-clock
	install -p -o root -g root -m 755 bin/ubrain-daemon		/usr/local/bin
	install -p -o root -g root -m 755 bin/ubrain-get-time		/usr/local/bin
	install -p -o root -g root -m 755 bin/launch-opc-client		/usr/local/bin
	install -p -o root -g root -m 755 bin/launch-opc-server		/usr/local/bin
	install -p -o root -g root -m 755 bin/usb-watch			/usr/local/bin
	install -p -o root -g root -m 755 bin/power			/usr/local/bin
	install -p -o root -g root -m 755 bin/console			/usr/local/bin
	install -p -o root -g root -m 755 bin/opc-watchdog		/usr/local/bin
	install -p -o root -g root -m 644 conf/opc-client.service	/etc/systemd/system
	install -p -o root -g root -m 644 conf/opc-server.service	/etc/systemd/system
	install -p -o root -g root -m 644 conf/opc-watchdog.service	/etc/systemd/system
	install -p -o root -g root -m 644 conf/ubrain-daemon.service	/etc/systemd/system
	install -p -o root -g root -m 644 conf/crontab			/etc/cron.d/soma
	install -p -o root -g root -m 644 conf/rsyslog-logrotate.txt	/etc/logrotate.d/rsyslog
	systemctl --system daemon-reload
	systemctl enable opc-server.service
	systemctl enable opc-client.service
	systemctl enable opc-watchdog.service
	systemctl enable ubrain-daemon.service
	update-rc.d ubrain-clock defaults 1
