#
# Copyright(C) 2016 Pedro H. Penna <pedrohenriquepenna@gmail.com>
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

# Tweaks on toolchain.
CFLAGS += -I include

# Executable file.
EXEC = benchmark

# Builds everything.
all: static dynamic guided srr was

# Builds static version.
static:
	$(CC) *.c $(CFLAGS) -D_SCHEDULE_STATIC_ -o $(BINDIR)/$(EXEC).static $(LIBS)

# Builds dynamic version.
dynamic:
	$(CC) *.c $(CFLAGS) -D_SCHEDULE_DYNAMIC_ -o $(BINDIR)/$(EXEC).dynamic $(LIBS)

# Builds guided version.
guided:
	$(CC) *.c $(CFLAGS) -D_SCHEDULE_GUIDED_ -o $(BINDIR)/$(EXEC).guided $(LIBS)

# Builds srr.
srr:
	$(CC) *.c $(CFLAGS) -D_SCHEDULE_SRR_ -o $(BINDIR)/$(EXEC).srr $(LIBS)

# Builds was
was:
	$(CC) *.c $(CFLAGS) -D_SCHEDULE_WAS_ -o $(BINDIR)/$(EXEC).was $(LIBS)

# Cleans compilation files.
clean:
	rm -f $(BINDIR)/$(EXEC).*
