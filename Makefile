all: tmanager tworker cmd 

CLIBS=-pthread
CC=gcc
CPPFLAGS=
CFLAGS=-g -Werror-implicit-function-declaration -pedantic -std=gnu99

tworker: tworker.h msg.h tworker.c
	$(CC) $(CFLAGS) -o tworker tworker.c

tmanager: transaction_msg.h tmanager.h tmanager.c tmanager_begin.c tmanager_join.c tmanager_commit.c tmanager_send_message.c
	$(CC) $(CFLAGS) -o tmanager tmanager.c tmanager_begin.c tmanager_join.c tmanager_commit.c tmanager_send_message.c

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


