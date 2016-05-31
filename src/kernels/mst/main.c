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
#include <omp.h>
#include <string.h>
#include <stdio.h>

#include <util.h>
#include <mst.h>

/**
 * @brief Reads input file. 
 */
static int *readinput(const char *filename, int *nregions)
{
	FILE *infile;   /* Input file. */
	int *densities; /* Densities.  */
	
	/* Sanity check. */
	assert(filename != NULL);
	assert(ndensities != NULL);
	
	/* Open input file. */
	infile = fopen(filename, "r");
	assert(infile != NULL);
	
	/* Allocate densities array. */
	assert(fscanf(infile, "%d", nregions) == 1);
	densities = smalloc(nregions*sizeof(int));
	
	/* Read densities. */
	for (int i = 0; i < nregions; i++)
		assert(fscanf(infile, "%d", &densities[i]) == 1);
		
	/* House keeping. */
	fclose(infile);

	return (densities);
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
 * @brief Generates input points.
 */
static struct points *generate_points(int densities, int nregions, int *npoints)
{
	
}

/**
 * @brief MST clustering kernel front end.
 */
int main(int argc, char **argv)
{
	int nregions;         /* Number of regions. */
	int *densities;       /* Densities.         */
	int npoints;          /* Number of points.  */
	struct point *points; /* Points to cluster. */
	
	/* Wrong usage. */
	if (argc < 2)
		usage();
		
	/* Generate input data for MST. */
	densities = readinput(argv[1], &nregions);
	points = generate_points(densities, nregions, &npoints);

	mst(points, npoints);

	/* House keeping. */
	free(points);
	free(densities);
	
	return (EXIT_SUCCESS);
}
