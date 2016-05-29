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

CFLAGS += -Wno-unused-result

EXEC = bfs

# Builds everything.
all: profile static

# Builds static version.
static:
	$(CXX) $(CFLAGS) -D_SCHEDULE_STATIC_ *.cpp -o $(BINDIR)/$(EXEC).static $(LIBS)

# Builds profile version.
profile:
	$(CXX) $(CFLAGS) -D_SCHEDULE_PROFILE_ *.cpp -o $(BINDIR)/$(EXEC).profile $(LIBS)

# Cleans compilation files.
clean:
	rm -f $(BINDIR)/$(EXEC).*
