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

#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>

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
#define RNG_BETA_A          0.5
#define RNG_BETA_B          0.5
#define RNG_GAMMA_A         1.0
#define RNG_GAMMA_B         2.0
#define RNG_GAUSSIAN_STDDEV 1.0
#define RNG_GAUSSIAN_MEAN   0.0
#define RNG_UNIFORM_MIN     0.0
#define RNG_UNIFORM_MAX     1.0
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
	int npoints;         /**< Number of points.                     */
	const char *pdfname; /**< Name of probability density function. */
	int pdfid;           /**< ID of probability density function.   */
} args = {0, 0, NULL, 0};
 
/**
 * @brief Prints program usage and exits.
 */
static void usage(void)
{
	printf("Usage: mst_gen [options]\n");
	printf("Brief: MST clustering kernel input data generator\n");
	printf("Options:\n");
	printf("  --help                Prints this information and exits\n");
	printf("  --nintervals <number> Number of sampling intervals\n");
	printf("  --npoints <number>    Number of data points\n");
	printf("  --pdf <name>          Probability desity function for random numbers\n");
	
	exit(EXIT_SUCCESS);
}

/**
 * @brief Prints and error message and exits.
 * 
 * @param msg Error message.
 */
static void error(const char *msg)
{
	fprintf(stderr, "error: %s\n", msg);
	exit(EXIT_FAILURE);
}

/**
 * @brief Safe maloc().
 * 
 * @param n Number of bytes to allocate.
 * 
 * @returns A pointer to the allocated memory block.
 */
void *smalloc(size_t n)
{
	void *p;
	
	p = malloc(n);
	assert(p != NULL);
	
	return (p);
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
		else if (!strcmp(argv[i], "--npoints"))
			args.npoints = atoi(argv[i + 1]);
		else if (!strcmp(argv[i], "--pdf"))
			args.pdfname = argv[i + 1];
		else if (!strcmp(argv[i], "--help"))
			usage();
	}
	
	/* Check arguments. */
	if (args.nintervals < 1)
		error("invalid number of sampling intervals");
	if (args.npoints < 1)
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
 * @brief Builds a beta historgram.
 * 
 * @param nsamples   Number of samples.
 * @param nintervals Number of sampling intervals.
 * 
 * @returns A beta histogram.
 */
double *beta(int nsamples, int nintervals)
{
	int k;
	int residual;
	int *histogram;
	double *x;
	
	/* TODO: sanity check. */

	histogram = smalloc(nintervals*sizeof(int));
	x = smalloc(nsamples*sizeof(double));

	residual = 0;
	for (int i = 0; i < nintervals/2; i++)
	{
		int freq = nsamples/(1 << (i + 2));
		
		residual += freq;
		histogram[i] = freq;
		histogram[nintervals - i - 1] = freq;
	}
	residual = nsamples - (residual*2);
	
	if (residual > 0)
	{
		histogram[nintervals/2 - 1] += residual/2;
		histogram[nintervals/2 + 0] += residual/2;
	}
	
	
	/* Generate input data. */
	k = 0;
	for (int i = 0; i < nintervals; i++)
	{
		for (int j = 0; j < histogram[i]; j++)
			x[k++] = i;
	}
	
	/* House keeping. */
	free(histogram);
	
	return (x);
}

/**
 * @brief Builds a gamma historgram.
 * 
 * @param nsamples   Number of samples.
 * @param nintervals Number of sampling intervals.
 * 
 * @returns A gamma histogram.
 */
double *gamma(int nsamples, int nintervals)
{
	int k;
	int residual;
	int *histogram;
	double *x;
	
	/* TODO: sanity check. */

	histogram = smalloc(nintervals*sizeof(int));
	x = smalloc(nsamples*sizeof(double));

	k = nsamples;
	residual = 0;
	for (int i = 0; i < nintervals; i++)
	{
		int freq = k /= 2;
		
		residual += freq;
		histogram[i] = freq;
	}
	residual = nsamples - residual;
	
	if (residual > 0)
	{
		histogram[nintervals - 1] += residual/2;
		histogram[nintervals - 2] += residual/2;
	}
	
	/* Generate input data. */
	k = 0;
	for (int i = 0; i < nintervals; i++)
	{
		for (int j = 0; j < histogram[i]; j++)
			x[k++] = i;
	}
	
	/* House keeping. */
	free(histogram);
	
	return (x);
}

/**
 * @brief Builds a Gaussian histogram.
 * 
 * @param nsamples   Number of samples.
 * @param nintervals Number of sampling intervals.
 * 
 * @returns A Gaussian histogram.
 */
double *gaussian(int nsamples, int nintervals)
{
	int k;
	int residual;
	int *histogram;
	double *x;
	
	/* Sanity check. */
	assert(nsamples > 0);
	assert(nintervals > 0);

	histogram = smalloc(nintervals*sizeof(int));
	x = smalloc(nsamples*sizeof(double));

	residual = 0;
	for (int i = 0; i < nintervals/2; i++)
	{
		int freq = nsamples/(1 << (i + 2));
		
		residual += freq;
		histogram[nintervals/2 - i - 1] = freq;
		histogram[nintervals/2 + i + 0] = freq;
	}
	residual = nsamples - (residual*2);
	
	if (residual > 0)
	{
		histogram[0] += residual/2;
		histogram[nintervals - 1] += residual/2;
	}
	
	
	/* Generate input data. */
	k = 0;
	for (int i = 0; i < nintervals; i++)
	{
		for (int j = 0; j < histogram[i]; j++)
			x[k++] = i;
	}
	
	/* House keeping. */
	free(histogram);
	
	return (x);
}

/**
 * @brief MST kernel input data generator.
 */
int main(int argc, const char **argv)
{
	gsl_rng *r;            /* Pseudo-random number generator.      */
	const gsl_rng_type *T; /* Pseudo-random number generator type. */
	double *x;             /* X-numbers.                           */
	
	readargs(argc, argv);
	
	/* Setup random number generator. */
	gsl_rng_env_setup();
	T = gsl_rng_default;
	r = gsl_rng_alloc(T);
	
	/* Generate input data. */
	switch (args.pdfid)
	{
		/* Beta distribution. */
		case RNG_BETA:
		/* Uniform distribution. */
		case RNG_UNIFORM:
			x = beta(args.npoints, args.nintervals);
			break;
			
		/* Gamma distribution. */
		case RNG_GAMMA:
			x = gamma(args.npoints, args.nintervals);
			break;
			
		/* Fall trough. */
		default:
			
		/* Gaussian distribution. */
		case RNG_GAUSSIAN:
			x = gaussian(args.npoints, args.nintervals);
			break;
	}
	
	/* Dump input data. */
	printf("%d\n", args.npoints);
	for (int i = 0; i < args.npoints; i++)
	{
		double y;
		
		y = gsl_ran_flat(r, RNG_UNIFORM_MIN, RNG_UNIFORM_MAX);
		
		printf("%.10lf %.10lf\n", x[i], y);
	}
	
	/* House keeping. */
	free(x);
	gsl_rng_free(r);
	
	return (EXIT_SUCCESS);
}
