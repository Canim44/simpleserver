CFLAGS = -Wall -g
CPPFLAGS = -Wall -g

all: simpled ssSet ssGet ssDigest ssRun

csapp.o: csapp.h csapp.c

simpled: simpled.c csapp.o 
	gcc $(CPPFLAGS) simpled.c csapp.o -lpthread -o simpled

ssSet: ssSet.c csapp.o 
	gcc $(CPPFLAGS) ssSet.c csapp.o -lpthread -o ssSet

ssGet: ssGet.c csapp.o
	gcc $(CPPFLAGS) ssGet.c csapp.o -lpthread -o ssGet

ssDigest: ssDigest.c csapp.o
	gcc $(CPPFLAGS) ssDigest.c csapp.o -lpthread -o ssDigest

ssRun: ssRun.c csapp.o
	gcc $(CPPFLAGS) ssRun.c csapp.o -lpthread -o ssRun

.PHONY: clean
clean:
	/bin/rm -rf csapp.h csapp.c *.o simpled ssSet ssGet ssDigest ssRun
