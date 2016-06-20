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
# $2: Number of iterations.
#

# Directories.
CSVDIR=$PWD/csv

# PDFs.
PDFS=(beta gamma gaussian uniform)

# Scheduling strategies.
STRATEGIES=(static dynamic guided srr)

#
# Make plots on gnuplot.
#   $1 Kernel.
#   $2 Workload PDF.
#   $3 Number of iterations.
#   $4 Number of threads.
#
function make_plots
{
	# Time plots.
	gnuplot \
		-e "static='$CSVDIR/$1-$2-$3-static-cycles.csv'"    \
		-e "guided='$CSVDIR/$1-$2-$3-guided-cycles.csv'"    \
		-e "dynamic='$CSVDIR/$1-$2-$3-dynamic-cycles.csv'"  \
		-e "srr='$CSVDIR/$1-$2-$3-srr-cycles.csv'"          \
		-e "nthreads='$5'"                                  \
		scripts/kernel-time.gnuplot 
}

#
# Parses benchmark kernel.
#  $1 Workload PDF.
#  $2 Number of iterations.
#  $3 Number of threads.
#
function plot_benchmark
{
	make_plots benchmark $1 $2 $3
}


for pdf in "${PDFS[@]}"; do
	for strategy in "${STRATEGIES[@]}"; do
		plot_benchmark $pdf $2 $1
	done
done


rm -f *.tmp
