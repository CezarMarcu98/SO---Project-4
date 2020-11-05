CC = gcc
CFLAGS = -Wall -g
LIBS = -pthread -lscheduler -L.


build: libscheduler.so

libscheduler.so: so_scheduler.o help.o
	$(CC) -shared $^ -o $@ 

so_scheduler.o: so_scheduler.c
	$(CC) $(CFLAGS) -fPIC -c $<

help.o: help.c
	$(CC) $(CFLAGS) -fPIC -c $<

.PHONY: clean
clean:
	rm -f *.o *~ libscheduler.so test