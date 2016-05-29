# C compiler
CC = gcc 
CFLAGS = -fopenmp -O3
LIBS = -lm

EXEC = backprop

all: profile static

static:
	$(CC) $(CFLAGS) *.c -o $(BINDIR)/$(EXEC).static $(LIBS)

profile:
	$(CC) $(CFLAGS) *.c -o $(BINDIR)/$(EXEC).profile $(LIBS)

clean:
	rm -f $(BINDIR)/$(EXEC).*
