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
INDIR=$PWD/input
LIBDIR=$PWD/src/libgomp/libgomp/build/.libs

# Load adjust accordingly.
LOAD=4000000000000000000

# Number of threads.
NTHREADS=$1

# Number of iterations.
NITERATIONS=$3

# Scheduling strategies.
STRATEGIES=(static dynamic srr)

# Workloads.
WORKLOAD=(gamma gaussian)

# Kernels
KERNELS=(linear logarithm quadratic)

# Workload sorting.
SORT=random

# Skewness
SKEWNESS=(0.750 0.775 0.800 0.825 0.850 0.875 0.900)

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
	>> $CSVDIR/$1-cycles.csv
	
	grep "thread" $1.tmp \
	| cut -d" " -f 3           \
	>> $CSVDIR/$1-workload.csv
}

#
# Parses the benchmark.
#  $1 Probability density function.
#  $2 Skewness.
#  $3 Kernel type.
#  $4 Number of iterations.
#  $5 Scheduling strategy.
#  $6 Number of threads.
#
function parse_benchmark
{
	extract_variables benchmark-$1-$2-$3-$4-$5-$6
}

#===============================================================================
#                                 RUN ROUTINES
#===============================================================================

#
# Run synthetic benchmark.
#  $1 Probability density function.
#  $2 Skewness.
#  $3 Kernel type.
#  $4 Number of iterations.
#  $5 Scheduling strategy.
#  $6 Number of threads.
#
function run_benchmark
{
	echo "  Benchmark with $6 thread(s)"
	$BINDIR/benchmark.$5                 \
		--input $INDIR/$1-$4-$2-$3.csv   \
		--load $LOAD                     \
		--nthreads $6                    \
		--niterations $4                 \
		--sort $SORT                     
#	2>> benchmark-$1-$2-$3-$4-$5-$6.tmp
}

#===============================================================================
#                                 MAIN ROUTINE
#===============================================================================

mkdir -p $CSVDIR

map_threads $1 $2

for strategy in "${STRATEGIES[@]}";
do
	for skewness in "${SKEWNESS[@]}";
	do
		for pdf in "${WORKLOAD[@]}";
		do
			for kernel in "${KERNELS[@]}";
			do
				echo "== Running $strategy $skewness $workload"
				export LD_LIBRARY_PATH=$LIBDIR
				export OMP_SCHEDULE="$strategy"
				export OMP_NUM_THREADS=$1
				run_benchmark $pdf $skewness $kernel $NITERATIONS $strategy $NTHREADS
#				parse_benchmark $pdf $skewness $kernel $NITERATIONS $strategy $NTHREADS
#				rm -f *.tmp
			done
		done
	done
done
