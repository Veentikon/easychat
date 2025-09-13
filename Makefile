CC = gcc
CFLAGS = -Wall -pthread

all: chatserver chatclient

chatserver: chatserver.o
	$(CC) $(CFLAGS) -o chatserver chatserver.o

chatclient: chatclient.o
	$(CC) $(CFLAGS) -o chatclient chatclient.o

chatserver.o: chatserver.c chatroom.h
	$(CC) $(CFLAGS) -c chatserver.c

chatclient.o: chatclient.c
	$(CC) $(CFLAGS) -c chatclient.c

# remove intermediary .o files to clean space from clutter
clean:
	rm -f *.o chatserver chatclient
