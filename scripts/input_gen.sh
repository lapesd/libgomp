#
# Copyright(C) 2015-2016 Pedro H. Penna <pedrohenriquepenna@gmail.com>
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

for generator in mst_gen; do
	for pdf in beta gamma; do
		for npoints in 65536 131072 262144 524288 1048576; do
			for nintervals in 8 16 32 ; do
				bin/$generator               \
					--pdf $pdf               \
					--npoints $npoints       \
					--nintervals $nintervals \
				> data/mst-$pdf-$npoints-$nintervals.txt; 
			done
		done
	done
done
