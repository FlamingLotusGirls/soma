CC = gcc
CFLAGS = -Wall -Werror

all: soma_server

soma_server: soma_server.c opc/opc_server.c

run: soma_server
	./soma_server -f config.txt -l /dev/ttyUSB*

clean:
	rm -f soma_server

anyway: clean all
