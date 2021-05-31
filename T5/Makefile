CPPFLAGS = -DDEBUG -DLOG_LEVEL=LOG_DEBUG
CFLAGS = -Wall -g

.PHONY: all clean

build: aws
aws: aws.o sock_util.o http_parser.o
	gcc aws.o sock_util.o http_parser.o -o aws -laio

aws.o: aws.c sock_util.h debug.h util.h
	gcc -c aws.c -laio

sock_util.o: sock_util.c sock_util.h debug.h util.h

http_parser.o: http_parser.c http_parser.h
	gcc $(CPPFLAGS) $(CFLAGS) -c -o http_parser.o http_parser.c
clean:
	-rm -f *.o
	-rm -f aws