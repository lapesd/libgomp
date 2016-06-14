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

# Executable files.
EXEC_HISTOGRAM = histogram
EXEC_IS = is_gen
EXEC_MST = mst_gen

# Conflicts.
.PHONY: is mst histogram

# Builds common object files for data generators.

# Builds histogram generator.
histogram:
	$(CC) $(CFLAGS) histogram/*.c -o $(BINDIR)/$(EXEC_HISTOGRAM) $(LIBS)
all: is histogram

# Builds is kernel input data generator.
is:
	$(CC) $(CFLAGS) is/*.c -o $(BINDIR)/$(EXEC_IS) $(LIBS)

# Builds mst clustering kernel input data generator.
mst:
	$(CC) $(CFLAGS) mst/*.c -o $(BINDIR)/$(EXEC_MST) $(LIBS)

# Cleans compilation files.
clean:
	rm -f $(BINDIR)/$(EXEC_MST)
	rm -f $(BINDIR)/$(EXEC_IS)

