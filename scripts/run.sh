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
# 

#
# $1: Number of threads.
# $2: Is simultaneous multithreading (SMT) enabled?
# $3: Number of iterations
#

# Directories.
BINDIR=$PWD/bin
DATADIR=$PWD/data
CSVDIR=$PWD/csv
LIBDIR=$PWD/src/libgomp/libgomp/build/.libs

# PDFs.
PDFS=(beta gamma gaussian uniform)

# IS kernel workload sizes.
IS_SIZE=(33554432 67108864 134217728 268435456 536870912)

# MST clustering kernel workload sizes.
MST_SIZE=(65536 131072 262144 524288 1048576)

# Workload skewness.
SKEWNESS=(8 16 32)

# Scheduling strategies.
STRATEGIES=(static dynamic guided srr)

#===============================================================================
#                              UTILITY ROUTINES
#===============================================================================

#
# Maps threads on the cores.
#
# $1 Number of threads.
# $2 Is simultaneous multithreading (SMT) enabled?
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

#===============================================================================
#                              PARSING ROUTINES
#===============================================================================

#
# Extracts variables from raw results.
#   $1 Filename prefix.
#
function extract_variables
{
	grep "L1 Misses" $1.tmp \
	| cut -d" " -f 3        \
	>> $CSVDIR/$1-L1.miss.tmp
	
	grep "L2 Misses" $1.tmp \
	| cut -d" " -f 3        \
	>> $CSVDIR/$1-L2.miss.tmp
	
	grep "L2 Accesses" $1.tmp \
	| cut -d" " -f 3          \
	>> $CSVDIR/$1-L2.access.tmp
	
	grep "L3 Accesses" $1.tmp \
	| cut -d" " -f 3          \
	>> $CSVDIR/$1-L3.access.tmp
	
	grep "Total Cycles" $1.tmp \
	| cut -d" " -f 3           \
	>> $CSVDIR/$1-cycles.tmp
}

#
# Builds a csv of a variable.
#   $1 Filename prefix.
#
function build_csv
{
	paste -d";"                  \
		$CSVDIR/$1-?-cycles.tmp  \
	> $CSVDIR/$1-cycles.csv
	
	rm -f $CSVDIR/$1-*.tmp
}

#
# Parses the IS kernel.
#  $1 Workload PDF.
#  $2 Workload skewness.
#  $3 Scheduling strategy.
#  $4 Number of threads.
#
function parse_is
{
	for nnumbers in "${IS_SIZE[@]}"; do
		for (( nthreads=1; nthreads <= $4; nthreads++ )); do
			extract_variables is-$1-$nnumbers-$2-$3-$nthreads
		done
	done
	
	build_csv is-$1-$nnumbers-$2-$3
}

#
# Parses the MST clustering kernel.
#  $1 Workload PDF.
#  $2 Workload skewness.
#  $3 Scheduling strategy.
#  $4 Number of threads.
#
function parse_mst
{
	for npoints in "${MST_SIZE[@]}"; do
		for (( nthreads=1; nthreads <= $4; nthreads++ )); do
			extract_variables mst-$1-$npoints-$2-$3-$nthreads
		done
	done
	
	build_csv mst-$1-$npoints-$2-$3
}

#===============================================================================
#                                 RUN ROUTINES
#===============================================================================

#
# Runs the IS kernel.
#  $1 Workload PDF.
#  $2 Workload skewness.
#  $3 Scheduling strategy.
#  $4 Number of threads.
#
function run_is
{
	echo "  IS  kernel with $4 thread(s)"
	for nnumbers in "${IS_SIZE[@]}"; do
		$BINDIR/is.$3 $DATADIR/is-$1-$nnumbers-$2.txt \
		2>> is-$1-$nnumbers-$2-$3-$4.tmp
	done
}

#
# Runs the MST clustering kernel.
#  $1 Workload PDF.
#  $2 Workload skewness.
#  $3 Scheduling strategy.
#  $4 Number of threads.
#
function run_mst
{
	echo "  MST kernel with $4 thread(s)"
	for npoints in "${MST_SIZE[@]}"; do
		$BINDIR/mst.$3 $DATADIR/mst-$1-$npoints-$2.txt \
		2>> mst-$1-$npoints-$2-$3-$4.tmp
	done
}

#===============================================================================
#                                 MAIN ROUTINE
#===============================================================================

mkdir -p $CSVDIR

map_threads $1 $2

for pdf in "${PDFS[@]}"; do
	for strategy in "${STRATEGIES[@]}"; do
		for skewness in "${SKEWNESS[@]}"; do
			echo "== Running $strategy $pdf $skewness"
			for (( niterations=0; niterations < $3; niterations++ )); do
				for (( nthreads=1; nthreads <= $1; nthreads++ )); do
					export LD_LIBRARY_PATH=$LIBDIR
					export OMP_SCHEDULE="$strategy"
					export OMP_NUM_THREADS=$nthreads
					run_is  $pdf $skewness $strategy $nthreads
					run_mst $pdf $skewness $strategy $nthreads
				done
			done
			parse_is  $pdf $skewness $strategy $1
			parse_mst $pdf $skewness $strategy $1
			rm  -f *.tmp
		done
	done
done
