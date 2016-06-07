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
#

# Directories.
CSVDIR=$PWD/csv

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

#
# Make plots on gnuplot.
#   $1 Kernel.
#   $2 Workload PDF.
#   $3 Workload size.
#   $4 Workload skewness.
#   $5 Number of threads.
#
function make_plots
{
	# Time plots.
	gnuplot \
		-e "static='$/$1-$2-$3-$4-static-cycles.csv'"    \
		-e "guided='$CSVDIR/$1-$2-$3-$4-guided-cycles.csv'"    \
		-e "dynamic='$CSVDIR/$1-$2-$3-$4-dynamic-cycles.csv'"  \
		-e "srr='$CSVDIR/$1-$2-$3-$4-srr-cycles.csv'"          \
		-e "nthreads=$5"                                       \
		scripts/kernel-time.gnuplot 
}

#
# Parses the IS kernel.
#  $1 Workload PDF.
#  $2 Workload skewness.
#  $3 Number of threads.
#
function plot_is
{
	for nnumbers in "${IS_SIZE[@]}"; do
		make_plots is $1 $nnumbers $2 $3
	done
}

#
# Parses the MST clustering kernel.
#  $1 Workload PDF.
#  $2 Workload skewness.
#  $3 Number of threads.
#
function plot_mst
{
	for npoints in "${MST_SIZE[@]}"; do
		make_plots mst $1 $npoints $2 $3
	done
}

for pdf in "${PDFS[@]}"; do
	for strategy in "${STRATEGIES[@]}"; do
		for skewness in "${SKEWNESS[@]}"; do
			plot_is $pdf $skewness $1
			plot_mst $pdf $skewness $1
		done
	done
done


rm -f *.tmp
