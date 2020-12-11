CC = gcc
CFLAGS = -Wall -g
EXEC = 537pfsim-fifo 537pfsim-lru 537pfsim-clock

program:
	$(CC) $(CFLAGS) -o 537pfsim-fifo main.c fifo.c funcs.c
	$(CC) $(CFLAGS) -o 537pfsim-lru main.c lru.c funcs.c
	$(CC) $(CFLAGS) -o 537pfsim-clock main.c clock.c funcs.c
	$(CC) $(CFLAGS) -o 537pfsim-clock main.c clock.c funcs.c

537pfsim-fifo: replacement.h funcs.h
	$(CC) $(CFLAGS) -o 537pfsim-fifo main.c fifo.c funcs.c

537pfsim-lru: replacement.h funcs.h
	$(CC) $(CFLAGS) -o 537pfsim-lru main.c lru.c funcs.c

537pfsim-clock: replacement.h funcs.h
	$(CC) $(CFLAGS) -o 537pfsim-clock main.c clock.c funcs.c
 
537pfsim-opt: replacement.h funcs.h
	$(CC) $(CFLAGS) -o 537pfsim-opt main.c opt.c funcs.c

clean:
	rm *.o $(EXEC)