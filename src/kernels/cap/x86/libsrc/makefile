#
# Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
#

# Source files.
SRC = $(wildcard *.c)

# Object files.
OBJ = $(SRC:.c=.o)

# Builds library.
all: $(OBJ)
	$(AR) $(ARFLAGS) $(LIBDIR)/$(LIB).a $(OBJ)

# Builds object files.
%.o: %.c
	$(CC) $< $(CFLAGS) $(OPTIMIZATION) -c -o $@
	
# Cleans compilation files.
clean:
	rm -f $(OBJ)
	rm -f $(LIBDIR)/$(LIB).a
