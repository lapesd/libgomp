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
# #3: Number of loop iterations.
#

# Directories.
BINDIR=$PWD/bin
CSVDIR=$PWD/csv
LIBDIR=$PWD/src/libgomp/libgomp/build/.libs

# Load adjust accordingly.
LOAD=500000000

# Number of iterations.
NITERATIONS=$3

# Kernel type.
KERNEL_TYPE=linear

# Scheduling strategies.
STRATEGIES=(static dynamic srr)

# Workloads.
WORKLOAD=(gamma gaussian)

# Skewness
SKEWNESS=(0.50 0.65 0.80)

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
	export GOMP_CPU_AFFINITY="${AFFINITY[@]}"
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
	grep "Total Cycles" $1.tmp \
	| cut -d" " -f 3           \
	>> $CSVDIR/$1-cycles.tmp
	
	grep "thread" $1.tmp \
	| cut -d" " -f 3           \
	>> $CSVDIR/$1-workload.tmp
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
# Parses the benchmark.
#  $1 Scheduling strategy.
#  $2 Number of threads.
#  $3 Workload.
#  $4 Skewness.
#
function parse_benchmark
{
	extract_variables benchmark-$3-$4-$NITERATIONS-$1-$2
	
	build_csv benchmark-$3-$4-$NITERATIONS-$1
}

#===============================================================================
#                                 RUN ROUTINES
#===============================================================================

#
# Run synthetic benchmark.
#  $1 Scheduling strategy.
#  $2 Number of threads.
#  $3 Workload.
#  $4 Skewness
#
function run_benchmark
{
	echo "  Benchmark with $2 thread(s)"
	$BINDIR/benchmark.$1 \
		--kernel $KERNEL_TYPE      \
		--load $LOAD               \
		--nthreads $2              \
		--niterations $NITERATIONS \
		--pdf $3                   \
		--skewness $4              \
	2>> benchmark-$3-$4-$NITERATIONS-$1-$2.tmp
}

#===============================================================================
#                                 MAIN ROUTINE
#===============================================================================

mkdir -p $CSVDIR

map_threads $1 $2

for strategy in "${STRATEGIES[@]}"; do
	for skewness in "${SKEWNESS[@]}"; do
		for workload in "${WORKLOAD[@]}"; do
			echo "== Running $strategy"
			for (( nthreads=2; nthreads <= $1; nthreads++ )); do
				export LD_LIBRARY_PATH=$LIBDIR
				export OMP_SCHEDULE="$strategy"
				export OMP_NUM_THREADS=$nthreads
				run_benchmark $strategy $nthreads $workload $skewness
			done
			parse_benchmark $strategy $1 $workload $skewness
			rm -f *.tmp
		done
	done
done
