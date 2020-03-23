all: tmanager tworker cmd 

CLIBS=-pthread
CC=gcc
CPPFLAGS=
CFLAGS=-g -Werror-implicit-function-declaration -pedantic -std=gnu99

tworker: tworker.h msg.h tworker.c
	$(CC) $(CFLAGS) -o tworker tworker.c

TMANAGEROBJS=tmanager.o
tmanager: $(TMANAGEROBJS)
	$(CC) $(CFLAGS) -o tmanager $(TMANAGEROBJS)

cmd: cmd.c msg.h
	$(CC) $(CFLAGS) -o cmd cmd.c

cleanlogs:
	rm -f *.log

cleanobjs:
	rm -f *.data

clean:
	rm -f *.o
	rm -f tmanager tworker cmd dumpObject

scrub: cleanlogs cleanobjs clean


