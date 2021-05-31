CC = cl
CPPFLAGS = /DDEBUG /DLOG_LEVEL=LOG_DEBUG
CFLAGS = /nologo /W3 /EHsc $(CPPFLAGS)
LIBS = wsock32.lib mswsock.lib ws2_32.lib
LINK = link

build: aws.exe

aws.exe: aws.obj sock_util.obj http_parser.obj
	$(LINK) /nologo /out:$@ $** $(LIBS)

aws.obj: aws.c sock_util.h debug.h util.h

sock_util.obj: sock_util.c sock_util.h debug.h util.h
	$(CC) $(CFLAGS) /c sock_util.c  /Fo$@

http_parser.obj: http_parser.c http_parser.h
	$(CC) $(CFLAGS) /TP /c http_parser.c /Fo$@

clean:
	del /Q /S *.obj
	del /Q /S aws.exe
