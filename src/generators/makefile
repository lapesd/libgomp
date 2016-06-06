#
# Copyright(C) 2016 Pedro H. Penna <pedrohenriquepenna@gmail.com
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

# Tweaks on toolchain.
CFLAGS += -I include

# Common C source files.
SRC = $(wildcard common/*.c)

# Common Object files.
OBJ = $(SRC:.c=.o)

# Executable files.
EXEC_IS = is_gen
EXEC_MST = mst_gen

# Conflicts.
.PHONY: is mst

# Builds common object files for data generators.
all: is mst

# Builds is kernel input data generator.
is: $(OBJ)
	$(CC) $(CFLAGS) is/*.c -o $(BINDIR)/$(EXEC_IS) $(OBJ) $(LIBS)

# Builds mst clustering kernel input data generator.
mst: $(OBJ)
	$(CC) $(CFLAGS) mst/*.c -o $(BINDIR)/$(EXEC_MST) $(OBJ) $(LIBS)

# Builds object file from C source file.
%.o: %.c
	$(CC) $< $(CFLAGS) -c -o $@

# Cleans compilation files.
clean:
	rm -f $(OBJ)
	rm -f $(BINDIR)/$(EXEC_MST)
	rm -f $(BINDIR)/$(EXEC_IS)

