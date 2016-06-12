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
#include <math.h>

#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>

#include <util.h>
#include <rng.h>

/**
 * @brief Number of supported probability density functions.
 */
#define NR_PDFS 4

/**
 * @brief Supported probability density functions.
 */
/**@{*/
#define RNG_BETA     1 /**< Beta.     */
#define RNG_GAMMA    2 /**< Gamma.    */
#define RNG_GAUSSIAN 3 /**< Gaussian. */
#define RNG_UNIFORM  4 /**< Uniform.  */
/**@}*/

/**
 * @brief Parameters for pseudo-random number generators.
 */
/**@{*/
#define SKEWNESS 0.25
/**@}*/

/**
 * @brief Name of supported probability density functions.
 */
static const char *pdfnames[NR_PDFS] = {
	"beta",     /* Beta.     */
	"gamma",    /* Gammma.   */
	"gaussian", /* Gaussian. */
	"uniform"   /* Uniform.  */
};

/**
 * @brief Program arguments.
 */
struct
{
	int nintervals;      /**< Number of sampling intervals.         */
	long nnumbers;       /**< Number of points.                     */
	const char *pdfname; /**< Name of probability density function. */
	int pdfid;           /**< ID of probability density function.   */
} args = {0, 0, NULL, 0};
 
/**
 * @brief Prints program usage and exits.
 */
static void usage(void)
{
	printf("Usage: is_gen [options]\n");
	printf("Brief: IS kernel input data generator\n");
	printf("Options:\n");
	printf("  --help                Prints this information and exits\n");
	printf("  --nintervals <number> Number of sampling intervals\n");
	printf("  --nnumbers <number>   Number of  numbers\n");
	printf("  --pdf <name>          Probability desity function for random numbers.\n");
	printf("        beta              a = 0.5 and b = 0.5\n");
	printf("        gamma             a = 1.0 and b = 2.0 \n");
	printf("        gaussian          x = 0.0 and std = 1.0\n");
	printf("        uniform           a = 0.0 and b = 1.0\n");
	
	exit(EXIT_SUCCESS);
}

/**
 * @brief Reads command line arguments.
 * 
 * @param argc Number of arguments.
 * @param argv Arguments.
 */
static void readargs(int argc, const char **argv)
{
	/* Parse command line arguments. */
	for (int i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "--nintervals"))
			args.nintervals = atoi(argv[i + 1]);
		else if (!strcmp(argv[i], "--nnumbers"))
			args.nnumbers = atol(argv[i + 1]);
		else if (!strcmp(argv[i], "--pdf"))
			args.pdfname = argv[i + 1];
		else if (!strcmp(argv[i], "--help"))
			usage();
	}
	
	/* Check arguments. */
	if (args.nintervals < 1)
		error("invalid number of sampling intervals");
	if (args.nnumbers < 1)
		error("invalid number of tasks");
	if (args.pdfname == NULL)
		error("unsupported probability density function");
	for (int i = 0; i < NR_PDFS; i++)
	{
		if (!strcmp(args.pdfname, pdfnames[i]))
		{
			args.pdfid = i + 1;
			goto out;
		}
	}
	error("unsupported probability density function");
	

out:
	return;
}

/**
 * @brief IS kernel input data generator.
 */
int main(int argc, const char **argv)
{
	long n;
	double *h;
	
	readargs(argc, argv);
	
	/* Generate input data. */
	switch (args.pdfid)
	{
		/* Beta distribution. */
		case RNG_BETA:
			h = beta(args.nintervals, SKEWNESS);
			break;
			
		/* Gamma distribution. */
		case RNG_GAMMA:
			h = gamma(args.nintervals, SKEWNESS);
			break;
			
		/* Gaussian distribution. */
		case RNG_GAUSSIAN:
			h = gaussian(args.nintervals, SKEWNESS);
			break;
			
		/* Fall trough. */
		default:
			
		/* Uniform distribution. */
		case RNG_UNIFORM:
			h = uniform(args.nintervals, SKEWNESS);
			break;
	}
	
	/* Adjust number of numbers. */
	n = 0;
	for (int i = 0; i < args.nintervals; i++)
		n += ceil(args.nnumbers*h[i]);
	
	/* Dump input data. */
	printf("%ld\n", n);
	for (int i = 0; i < args.nintervals; i++)
	{
		int nnumbers = ceil(args.nnumbers*h[i]);
		
		for (int j = 0; j < nnumbers; j++)
			printf("%d\n", i);
	}
	
	/* House keeping. */
	free(h);
	
	return (EXIT_SUCCESS);
}
