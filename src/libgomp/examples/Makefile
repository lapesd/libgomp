CC=gcc
CFLAGS=-rdynamic -g -fopenmp

all: pedro-test binlpt-01

pedro-test:
	$(CC) $(CFLAGS) pedro-test.c -o pedro-test

# Trick for both linking against our modified libgomp AND use it at run time.
binlpt-01: binlpt-01.c ../build/omp.h
	$(CC) -c -I../build -g -O0 -fopenmp binlpt-01.c
	$(CC) binlpt-01.o -o binlpt-01 -Wl,-rpath=../build/.libs/ -L../build/.libs -lgomp

../build/omp.h:
	@echo "[error] Can't find ../build/omp.h. Please build the libgomp library before trying to compile any example program."
	@exit 1

clean:
	rm -f *.o pedro-test binlpt-01
