# C compiler
CC = gcc
CC_FLAGS = -g -fopenmp  -O2


backprop: backprop.o facetrain.o imagenet.o backprop_kernel.o 
	$(CC) $(CC_FLAGS) backprop.o facetrain.o imagenet.o backprop_kernel.o -o $(BINDIR)/backprop -lm

%.o: %.[ch]
	$(CC) $(CC_FLAGS) $< -c

clean:
	rm -f *.o $(BINDIR)/backprop
