CC=gcc
CFLAGS=`pkg-config --cflags glib-2.0 json-glib-1.0` -Wall -g
LIBS=`pkg-config --libs glib-2.0 json-glib-1.0`

all: libcinet.so.1.0

test: test.o
	$(CC) -L. -o test test.o -lcinet $(LIBS)

test.o: test.c
	$(CC) -I. $(CFLAGS) -c -o test.o test.c

libcinet.so.1.0: cinet.h cinet.c cinetmsgs.h
	$(CC) -I. $(CFLAGS) -fPIC -c -o cinet.o cinet.c
	$(CC) -shared -Wl,-soname,libcinet.so.1 -o libcinet.so.1.0 cinet.o $(LIBS)

install: libcinet.so.1.0
	install libcinet.so.1.0 /usr/lib/
	ln -sf /usr/lib/libcinet.so.1.0 /usr/lib/libcinet.so.1
	ln -sf /usr/lib/libcinet.so.1 /usr/lib/libcinet.so
	cp cinet.h cinetmsgs.h /usr/include

clean:
	rm -f libcinet.so.1.0 test test.o cinet.o
