#
# Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
#

# Source files.
SRC = $(wildcard *.c)

# Executable file.
EXEC = tsp

# Builds kernel.
all: $(SRC)
	$(CC) -Wno-unused-parameter -DMT $(SRC) $(CFLAGS) -o $(BINDIR)/$(EXEC).intel $(LIBS)

# Cleans compilation files.
clean:
	rm -f $(BINDIR)/$(EXEC).intel
