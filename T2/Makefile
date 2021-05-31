all: build

build: so_stdio.o
	gcc -shared -o libso_stdio.so so_stdio.o

so_stdio.o: so_stdio.c
	gcc -fPIC -c so_stdio.c

clean:
	rm -f so_stdio.o libso_stdio.so
