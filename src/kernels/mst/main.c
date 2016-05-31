/*
 * Copyright(C) 2016 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include <assert.h>
#include <stdio.h>

#include <util.h>
#include <mst.h>

/**
 * @brief Reads input file. 
 * 
 * @param filename Input filename.
 * @param npoints  Number of points (output parameter).
 * 
 * @returns The data points.
 */
static struct point *readinput(const char *filename, int *npoints)
{
	int _npoints;         /* Local npoints. */
	FILE *infile;         /* Input file.    */
	struct point *points; /* Data points.   */
	
	/* Sanity check. */
	assert(filename != NULL);
	assert(npoints != NULL);
	
	/* Open input file. */
	infile = fopen(filename, "r");
	assert(infile != NULL);
	
	/* Allocate densities array. */
	assert(fscanf(infile, "%d", &_npoints) == 1);
	points = smalloc(_npoints*sizeof(int));
	*npoints = _npoints;
	
	/* Read points. */
	for (int i = 0; i < _npoints; i++)
		assert(fscanf(infile, "%lf %lf", &points[i].x, &points[i].y) == 2);
		
	/* House keeping. */
	fclose(infile);

	return (points);
}

/**
 * @brief Prints program usage and exits.
 */
static void usage(void)
{
	printf("Usage: mst input\n");
	exit(EXIT_SUCCESS);
}

/**
 * @brief MST clustering kernel front end.
 */
int main(int argc, char **argv)
{
	int npoints;          /* Number of points.  */
	struct point *points; /* Points to cluster. */
	
	/* Wrong usage. */
	if (argc < 2)
		usage();
		
	/* Generate input data for MST. */
	points = readinput(argv[1], &npoints);

	mst_clustering(points, npoints);

	/* House keeping. */
	free(points);
	
	return (EXIT_SUCCESS);
}
