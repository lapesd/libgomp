#
# Copyright(C) 2015 Pedro H. Penna <pedrohenriquepenna@gmail.com
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

# Common C source files.
SRC = $(wildcard $(CURDIR)/common/*.c)

# Common Object files.
OBJ = $(SRC:.c=.o)

# Tweaks on toolchain.
LIBS := $(OBJ) $(LIBS)

# Phony list.
.PHONY: benchmark
.PHONY: generators
.PHONY: kernels
.PHONY: libgomp

# Builds everything.
all: benchmark kernels generators

# Builds benchmark.
benchmark: $(OBJ)
	cd benchmark && $(MAKE) all

# Builds kernels.
kernels: $(OBJ)
	cd kernels && $(MAKE) all

# Builds input data generators.
generators: $(OBJ)
	cd generators && $(MAKE) all
	
# Builds common objects.
%.o: %.c
	$(CC) $< $(CFLAGS) -c -o $@

# Builds libgomp.
libgomp:
	cd libgomp/libgomp/build ; \
	CFLAGS="" LIBS="" PREFIX="" ../configure --disable-multilib ; \
	$(MAKE)

# Cleans libgomp.
libgomp-clean:
	cd libgomp/libgomp/build ; \
	$(MAKE) distclean
	
# Cleans everything.
clean:
	rm -f $(OBJ)
	cd benchmark && $(MAKE) clean
	cd generators && $(MAKE) clean
	cd kernels && $(MAKE) clean
