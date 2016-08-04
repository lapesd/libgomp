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
# #3: Number of loop iterations.
# $1: Number of threads.
# $2: Is simultaneous multithreading (SMT) enabled?
#
NTASKS=$1   # Number of iterations.
NTHREADS=$2 # Number of threads.
SMT=$3      # Is simultaneous multithreading (SMT) enabled?

# Directories.
BINDIR=$PWD/bin
OUTDIR=$PWD/csv
INDIR=$PWD/input
LIBDIR=$PWD/src/libgomp/libgomp/build/.libs

source scripts/var.sh

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
	>> $OUTDIR/$1-cycles.csv
	
	grep "Thread" $1.tmp       \
	| cut -d" " -f 3           \
	>> $OUTDIR/$1-workload.csv
}

#
# Parses the benchmark.
#  $1 Scheduling strategy.
#  $2 Number of threads.
#  $3 Workload.
#  $4 Skewness.
#  $5 Kernel.
#  $6 Sorting
#
function parse_benchmark
{
	extract_variables benchmark-$3-$4-$6-$NTASKS-$5-$1-$2
}

#===============================================================================
#                                 RUN ROUTINES
#===============================================================================

#
# Run synthetic benchmark.
#  $1 Scheduling strategy.
#  $2 Number of threads.
#  $3 Workload.
#  $4 Skewness.
#  $5 Kernel.
#  $6 Sorting.
#
function run_benchmark
{
	echo "  Benchmark with $2 thread(s)"
	$BINDIR/benchmark.$5                    \
		--input $INDIR/$3-$NTASKS-$4-$5.csv \
		--load $LOAD                        \
		--nthreads $2                       \
		--ntasks $NTASKS                    \
		--sort $SORT                        \
	2>> benchmark-$3-$4-$6-$NTASKS-$5-$1-$2.tmp
}

#===============================================================================
#                                 MAIN ROUTINE
#===============================================================================

mkdir -p $OUTDIR

map_threads $1 $SMT

for strategy in "${STRATEGIES[@]}";
do
	for pdf in "${WORKLOAD[@]}";
	do
		for skewness in "${SKEWNESS[@]}";
		do
			for kernel in "${KERNELS[@]}";
			do
				echo "== Running $strategy $skewness $workload"
				export LD_LIBRARY_PATH=$LIBDIR
				export OMP_SCHEDULE="$strategy"
				export OMP_NUM_THREADS=$1
				run_benchmark $strategy $NTHREADS $workload $skewness $kernel $sorting
				parse_benchmark $strategy $NTHREADS $workload $skewness $kernel $sorting

				# House keeping.
				rm -f *.tmp
			done
		done
	done
done

for pdf in "${WORKLOAD[@]}";
do
	# Move files.
	mkdir -p $OUTDIR/$pdf/cycles $OUTDIR/$pdf/workload
	mv $OUTDIR/*-$pdf-*-cycles.csv $OUTDIR/$pdf/cycles
	mv $OUTDIR/*-$pdf-*-workload.csv $OUTDIR/$pdf/workload
	
	for strategy in "${STRATEGIES[@]}";
	do
		for kernel in "${KERNELS[@]}";
		do
			for sorting in "${SORT[@]}";
			do
				# Header.
				echo ${SKEWNESS[@]} \
					> $OUTDIR/$pdf/cycles/benchmark-$pdf-$sorting-$NTASKS-$kernel-$strategy-$NTHREADS-cycles.csv
				
				# Data.
				paste -d " "                                                                                         \
					$OUTDIR/$pdf/cycles/benchmark-$pdf-0.???-$sorting-$NTASKS-$kernel-$strategy-$NTHREADS-cycles.csv \
					>> $OUTDIR/$pdf/cycles/benchmark-$pdf-$sorting-$NTASKS-$kernel-$strategy-$NTHREADS-cycles.csv
				
				# House keeping.
				rm -f $OUTDIR/$pdf/cycles/benchmark-$pdf-0.???-$sorting-$NTASKS-$kernel-$strategy-$NTHREADS-cycles.csv
			done
		done
	done
done
