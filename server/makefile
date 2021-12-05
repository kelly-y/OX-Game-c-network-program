SHELL = /bin/bash
CC = gcc

all: server
server: server.c game.o
	${CC} server.c game.o -o server
game.o: game.h game.c
	${CC} -c game.c

exe:
	./server

clean:
	rm -f server *.o