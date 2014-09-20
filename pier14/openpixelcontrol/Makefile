PROJ = soma_server
OBJS = soma_server.o opc/opc_server.o

CC = gcc
CFLAGS = -Wall -Werror

all: $(PROJ)

$(PROJ): $(OBJS)

run: $(PROJ)
	./$(PROJ) -f config.txt -l /dev/ttyUSB*

clean:
	rm -f $(PROJ) $(OBJS)

anyway: clean all
