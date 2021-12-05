SHELL = /bin/bash
CC = gcc

all: client
server: client.c
	${CC} client.c -o client

exe:
	./client 127.0.0.1

clean:
	rm -f client *.o