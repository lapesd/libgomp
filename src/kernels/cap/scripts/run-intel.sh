#
# Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
#

# Directories.
export ROOTDIR=$PWD
export RESULTSDIR=$ROOTDIR/results
export BINDIR=$ROOTDIR/bin
export ITERATIONS=10

# Create results directory.
mkdir -p $RESULTSDIR

for kernel in fast fn gf is km lu tsp; do
	echo "running $kernel"
	
	# Weak scaling
	for (($i = 0; $i < $ITERATIONS; $i++)); do
		echo "running weak scaling test: iteration $it"
		for nprocs in 1 2 4 8 16; do
			for class in tiny small standard large huge; do
				likwid-powermeter $BINDIR/$kernel.intel --verbose --class $class --nthreads $nprocs &>> $RESULTSDIR/$kernel-$class-$nprocs.intel
			done
		done
	done
	
	# Strong scaling.
	for (($i = 0; $i < $ITERATIONS; $i++)); do
		echo "running strong scaling test: iteration $it"
		# 1, 2, 4, 8 and 16 omitted because they are already computed
		for nprocs in 3 5 6 7 9 10 11 12 13 14 15; do
			likwid-powermeter $BINDIR/$kernel.intel --verbose --class standard --nthreads $nprocs &>> $RESULTSDIR/$kernel-standard-$nprocs.intel
		done
	done
done
