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

all: static guided dynamic smart-round-robin

# Builds static kernels.
static:
	cd is && $(MAKE) static
#	cd knn && $(MAKE) static
#	cd mst && $(MAKE) static

# Builds dynamic kernels.
dynamic:
	cd is && $(MAKE) dynamic
#	cd knn && $(MAKE) dynamic
#	cd mst && $(MAKE) dynamic

# Builds guided kernels.
guided:
	cd is && $(MAKE) guided
#	cd knn && $(MAKE) guided
#	cd mst && $(MAKE) guided

# Builds srr kernels.
smart-round-robin:
	cd is && $(MAKE) srr
#	cd knn && $(MAKE) srr
#	cd mst && $(MAKE) srr

# Cleans compilation files.
clean:
	cd is && $(MAKE) clean
#	cd knn && $(MAKE) clean
#	cd mst && $(MAKE) clean

