build: libscheduler.so

libscheduler.so: so_scheduler.o
	gcc -shared so_scheduler.o -o libscheduler.so

so_scheduler.o: so_scheduler.c
	gcc -Wall -g -fPIC -c so_scheduler.c

clean:
	rm -f *.o libscheduler.so