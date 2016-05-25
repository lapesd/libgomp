#
# Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
#

# Directories.
export ROOTDIR=$PWD
export RESULTSDIR=$ROOTDIR/results
export BINDIR=$ROOTDIR/bin
export MPPADIR=/usr/local/k1tools

# Problem size.
export CLASS=standard

# Create results directory.
mkdir -p $RESULTSDIR

echo "Problem size = $CLASS"
for kernel in fast fn gf is km lu tsp; do
	for nprocs in {1..16}; do
	    echo "  >> running $kernel with $nprocs cluster(s)."
	    $MPPADIR/bin/k1-power -- $MPPADIR/bin/k1-jtag-runner --multibinary=$BINDIR/$kernel.img --exec-multibin=IODDR0:master -- --verbose --class $CLASS --nclusters $nprocs &>> $RESULTSDIR/$kernel-$CLASS-$nprocs.mppa
	done
done
