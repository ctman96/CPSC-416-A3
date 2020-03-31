all: tmanager tworker cmd 

CLIBS=-pthread
CC=gcc
CPPFLAGS=
CFLAGS=-g -Werror-implicit-function-declaration -pedantic -std=gnu99

tworker: transaction_msg.h tworker.h msg.h tworker.c
	$(CC) $(CFLAGS) -o tworker tworker.c

test_tmanager: transaction_msg.h test_tmanager.c
	$(CC) $(CFLAGS) -o test_tmanager test_tmanager.c

tmanager: transaction_msg.h tmanager.h tmanager.c tmanager_begin.c tmanager_join.c tmanager_commit.c tmanager_send_message.c
	$(CC) $(CFLAGS) -o tmanager tmanager.c tmanager_begin.c tmanager_join.c tmanager_commit.c tmanager_send_message.c tmanager_poll.c

test: msg.h tworker.h test.c
	$(CC) $(CFLAGS) -o test test.c

cmd: cmd.c msg.h
	$(CC) $(CFLAGS) -o cmd cmd.c

cleanlogs:
	rm -f *.log

cleanobjs:
	rm -f *.data

clean:
	rm -f *.o
	rm -f tmanager tworker cmd test_tmanager dumpObject

scrub: cleanlogs cleanobjs clean


