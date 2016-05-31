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

#
# $1: Number of threads.
# $2: Is simultaneous multithreading (SMT) enabled?
# $3: Workload size.
#

BINDIR=$PWD/bin
LIBDIR=$PWD/src/libgomp/libgomp/build/.libs

#
# Maps threads on the cores.
#
# $1 Number of threads.
#
function map_threads
{
	# Build thread map.
	if [ $2 == "true" ]; then
		for (( i=0; i < $1; i++ )); do
			AFFINITY[$i]=$((2*$i))
		done
	else
		for (( i=0; i < $1; i++ )); do
			AFFINITY[$i]=$i
		done
	fi
	
	export OMP_NUM_THREADS=$1
	export GOMP_CPU_AFFINITY="${map[@]}"
}


map_threads $1 $2

for kernel in mst; do
	for pdf in beta gamma gaussian uniform; do
		for strategy in static dynamic guided srr; do
			echo "== Running $strategy $pdf"
			for (( nthreads=0; nthreads <= $1; nthreads++ )); do
				OMP_NUM_THREADS=$1        \
				LD_LIBRARY_PATH=$LIBDIR   \
				OMP_SCHEDULE="$strategy"  \
				$BINDIR/$kernel.$strategy data/mst-$pdf-$3.txt
			done
		done
	done
done
