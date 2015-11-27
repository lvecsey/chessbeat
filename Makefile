
CC=gcc

CFLAGS=-I$(HOME)/src/version-control/git/json-c

LDFLAGS=-L$(HOME)/src/version-control/git/json-c/.libs
LIBS=-ljson-c -lcurl -lrt -lpthread

all : chessbeat

chessbeat : chessbeat.o readfile.o
	$(CC) -o $@ $^ $(LDFLAGS) $(LIBS)

