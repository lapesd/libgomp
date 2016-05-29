#
# Copyright(C) 2015-2016 Pedro H. Penna <pedrohenriquepenna@gmail.com
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#  
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#  
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
# MA 02110-1301, USA.
#

#
# Rodinia Benchmark Suite
#   - BP: backpropagation training
#   + BFS: breadth-first search
#   + CFD: 3D euler equation solver
#

# Directories.
export BINDIR = $(CURDIR)/bin

# Toolchain.
export CC = gcc
export CXX = g++

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
	cd openmp/bfs && $(MAKE) all
	cd openmp/cfd && $(MAKE) all
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
	cd openmp/bfs && $(MAKE) clean
