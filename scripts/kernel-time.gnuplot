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

set datafile separator ";"

FREQUENCY=1000000000

set style line 1 lc rgb "red" lw 4 pt 5 ps 1.5
set style line 2 lc rgb "#00008B" lw 4 pt 7 ps 1.5
set style line 3 lc rgb "#228B22" lw 4 pt 11 ps 2.1

set terminal postscript eps enhanced color

set print "guided.tmp"
do for [i=1:nthreads] {
	stats guided using (column(i)) nooutput
	print STATS_mean/FREQUENCY
}

set print "dynamic.tmp"
do for [i=1:nthreads] {
	stats dynamic using (column(i)) nooutput
	print STATS_mean/FREQUENCY
}

set print "srr.tmp"
do for [i=1:nthreads] {
	stats srr using (column(i)) nooutput
	print STATS_mean/FREQUENCY
}

set output "kernel-time.eps"

# Axis
set xlabel font ", 18" "Number of Threads"
set xtics 2,1,16
set ylabel font ", 18" "Time (s)"

# Grid
set grid xtics
set grid ytics

# Legend
set key height 1 width 1 box lw 1

# Plot
plot "guided.tmp" using ($0+2):1 with linespoints ls 1 title "Guided",   \
     "dynamic.tmp" using ($0+2):1 with linespoints ls 2 title "Dynamic", \
     "srr.tmp" using ($0+2):1 with linespoints ls 3 title "SRR"
