# Directories.
export BINDIR = $(CURDIR)/bin

# Toolchain.
export CC = gcc

# Toolchain configuration.
export CFLAGS += -fopenmp
export CFLAGS += -O3

# Libraries.
export LIBS += $(CURDIR)/../../libgomp/libgomp/build/.libs/libgomp.a
export LIBS += -lm

all: OMP

OMP:
	mkdir -p  $(BINDIR)
	cd openmp/backprop && $(MAKE) all
#	cd openmp/bfs && $(MAKE)
#	cd openmp/cfd && $(MAKE)
#	cd openmp/heartwall && $(MAKE)
#	cd openmp/hotspot && $(MAKE)
#	cd openmp/kmeans/kmeans_openmp && $(MAKE)
#	cd openmp/lavaMD && $(MAKE)
#	cd openmp/leukocyte && $(MAKE)
#	cd openmp/lud && $(MAKE)
#	cd openmp/nn && $(MAKE)
#	cd openmp/nw && $(MAKE)
#	cd openmp/srad/srad_v1 && $(MAKE)
#	cd openmp/srad/srad_v2 && $(MAKE)
#	cd openmp/streamcluster && $(MAKE)
#	cd openmp/particlefilter && $(MAKE)
#	cd openmp/pathfinder && $(MAKE)
#	cd openmp/mummergpu && $(MAKE)
 
clean: OMP_clean

OMP_clean:
	cd openmp/backprop && $(MAKE) clean
