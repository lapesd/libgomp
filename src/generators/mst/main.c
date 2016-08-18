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
#include <stdlib.h>
#include <string.h>

#include <util.h>

/**
 * @brief Program arguments.
 */
struct
{
	const char *input;  /**< Input workload file.           */
	int nregions;       /**< Number of sampling intervals. */
	int npoints;        /**< Number of points.             */
} args = { NULL, 0, 0 };
 
/**
 * @brief Prints program usage and exits.
 */
static void usage(void)
{
	printf("Usage: mst_gen [options]\n");
	printf("Brief: MST clustering kernel input data generator\n");
	printf("Options:\n");
	printf("  --help              Prints this information and exits\n");
	printf("  --nregions <number> Number of regions\n");
	printf("  --input <filename>  Input workload file\n");
	printf("  --npoints <number>  Number of data points\n");
	
	exit(EXIT_SUCCESS);
}

/*============================================================================*
 *                              Array Shuffling                               *
 *============================================================================*/

/**
 * @brief Shuffles and array.
 * 
 * @param a    Target array.
 * @param n    Size of target array.
 * @param seed Seed for shuffling.
 */
static void array_shuffle(unsigned *a, unsigned n, int seed)
{
	/* Let us be totally random. */
	srand(seed);
	
	/* Shuffle array. */
	for (unsigned i = 0; i < n - 1; i++)
	{
		unsigned j; /* Shuffle index.  */
		unsigned t; /* Temporary data. */
		
		j = i + rand()/(RAND_MAX/(n - i) + 1);
			
		t = a[i];
		a[i] = a[j];
		a[j] = t;
	}
}


/*============================================================================*
 *                             Arguments Parsing                              *
 *============================================================================*/

/**
 * @brief Checks program arguments.
 */
static void chkargs(void)
{
	/* Check arguments. */
	if (args.npoints == 0)
		error("invalid number of data points");
	else if (args.nregions == 0)
		error("invalid number of regions");
	else if (args.input == NULL)
		error("missing input workload file");
}

/**
 * @brief Reads command line arguments.
 * 
 * @param argc Number of arguments.
 * @param argv Arguments.
 */
static void readargs(long argc, const char **argv)
{
	/* Parse command line arguments. */
	for (long i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "--nregions"))
			args.nregions = atoi(argv[++i]);
		else if (!strcmp(argv[i], "--npoints"))
			args.npoints = atoi(argv[++i]);
		else if (!strcmp(argv[i], "--input"))
			args.input = argv[++i];
		else if (!strcmp(argv[i], "--help"))
			usage();
	}
	
	/* Check arguments. */
	chkargs();
}

/**
 * @brief Reads input file
 * 
 * @param input Input filename.
 * @param ntasks Number of tasks.
 */
static unsigned *readfile(const char *input, unsigned ntasks)
{
	FILE *fp;
	unsigned *tasks;
	
	tasks = smalloc(ntasks*sizeof(unsigned));
	
	fp = fopen(input, "r");
	assert(fp != NULL);
	
	/* Read file. */
	for (unsigned i = 0; i < ntasks; i++)
	{
		if (fscanf(fp, "%u", &tasks[i]) == EOF)
		{
			if (feof(fp))
				error("unexpected end of file");
			else if (ferror(fp))
				error("cannot read file");
			else
				error("unknown error");
			break;
		}
	}
	
	/* I/O error. */
	if (ferror(fp))
		error("cannot read input file");
	
	fclose(fp);
	
	return (tasks);
}

/**
 * @brief MST clustering kernel input data generator.
 */
int main(int argc, const char **argv)
{
	unsigned *regions; /* Region load.                 */
	int k;             /* Residual number of points.   */
	unsigned total;    /* Total region load.           */
	double *densities; /* Number of points per region. */
	int *x;            /* X coordinates.               */

	readargs(argc, argv);
	
	x = smalloc(args.npoints*sizeof(int));

	/* Read input workload. */	
	regions = readfile(args.input, args.nregions);
	array_shuffle(regions, args.nregions, 1);

	/* Compute densities. */
	densities = smalloc(args.nregions*sizeof(double));
	total = 0;
	for (int i = 0; i < args.nregions; i++)
		total += regions[i];
	for (int i = 0; i < args.nregions; i++)
		densities[i] = regions[i]/((double)total);
	
	/* Compute x coordinates. */
	k = 0;
	printf("%d\n", args.npoints);
	for (int i = 0; i < args.nregions; i++)
	{
		int n;
		
		n = densities[i]*args.npoints;
		
		for (int j = 0; j < n; j++)
			x[k++] = i + 1;
	}
	
	/* Residual points. */
	for (int i = k; i < args.npoints; i++)
		x[i] = args.nregions;

	/* Dump points. */
	for (int i = 0; i < args.npoints; i++)
	{
		double y;
			
		y = rand()/((double)RAND_MAX);			
		printf("%.10lf %.10lf\n", (double) x[i], y);
	}
	
	/* House keeping. */
	free(x);
	free(densities);
	free(regions);
	
	return (EXIT_SUCCESS);
}
