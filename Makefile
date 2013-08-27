CC=gcc
CFLAGS=`pkg-config --cflags glib-2.0 json-glib-1.0` -Wall
LIBS=`pkg-config --libs glib-2.0 json-glib-1.0`

all: test

test: test.o cinet.o
	$(CC) -o test $(LIBS) test.o cinet.o

test.o: test.c
	$(CC) $(CFLAGS) -c -o test.o test.c

cinet.o: cinet.h cinet.c
	$(CC) $(CFLAGS) -c -o cinet.o cinet.c
