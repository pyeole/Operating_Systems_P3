#
#
CC=gcc
CFLAGS=-g -w -pthread

# comment line below for Linux machines
#LIB= -lsocket -lnsl

all: master player

master:	master.o
	$(CC) $(CFLAGS) -o $@ master.o $(LIB)

player:	player.o
	$(CC) $(CFLAGS) -o $@ player.o $(LIB)

master.o:	master.c

player.o:	player.c 

clean:
	\rm -f master player master.o player.o
