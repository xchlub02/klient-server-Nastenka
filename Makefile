CC = gcc
CFLAGS = -Wall -Wextra -pedantic
PFLAGS= stringFunctions.c listFunctions.c
FILES = isaclient.c isaserver.c

all: clean isaclient isaserver



isaserver:
	$(CC) $(CFLAGS) -o isaserver isaserver.c stringFunctions.c listFunctions.c


isaclient:
	$(CC) $(CFLAGS) -o isaclient isaclient.c stringFunctions.c

clean:
	rm -f *.o *.out isaclient isaserver *~
