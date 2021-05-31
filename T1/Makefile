all: build

build: so-cpp.c HashMap.c helpers.c
	cl /MD /W3 so-cpp.c HashMap.c helpers.c

clean:
	del so-cpp.exe *.obj
