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
	"beta"      /* Beta.     */
	"gamma",    /* Gammma.   */
	"gaussian", /* Gaussian. */
	"uniform",  /* Uniform.  */
};

/**
 * @brief Program arguments.
 */
struct
{
	int npoints;         /**< Number of points.                     */
	const char *pdfname; /**< Name of probability density function. */
	int pdfid;           /**< ID of probability density function.   */
} args = {0, NULL, 0};
 
/**
 * @brief Prints program usage and exits.
 */
static void usage(void)
{
	printf("Usage: mst_gen [options]\n");
	printf("Brief: MST clustering kernel input data generator\n");
	printf("Options:\n");
	printf("  --help             Prints this information and exits\n");
	printf("  --npoints <number> Number of data points\n");
	printf("  --pdf <name>       Probability desity function for random numbers\n");
	
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
		if (!strcmp(argv[i], "--ntasks"))
			args.npoints = atoi(argv[i + 1]);
		else if (!strcmp(argv[i], "--pdf"))
			args.pdfname = argv[i + 1];
		else if (!strcmp(argv[i], "--help"))
			usage();
	}
	
	/* Check arguments. */
	if (args.npoints < 0)
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
 * @brief MST kernel input data generator.
 */
int main(int argc, const char **argv)
{
	gsl_rng *r;            /* Pseudo-random number generator.      */
	const gsl_rng_type *T; /* Pseudo-random number generator type. */
	
	readargs(argc, argv);
	
	/* Setup random number generator. */
	gsl_rng_env_setup();
	T = gsl_rng_default;
	r = gsl_rng_alloc(T);
	
	/* Generate input data. */
	for (int i = 0; i < args.npoints; i++)
	{
		double x, y;
		
		y = gsl_ran_flat(r, RNG_UNIFORM_MIN, RNG_UNIFORM_MAX);
		
		switch (args.pdfid)
		{
			/* Beta distribution. */
			case RNG_BETA:
				x = gsl_ran_beta(r, RNG_BETA_A, RNG_BETA_B);
				break;
			/* Gamma distribution. */
			case RNG_GAMMA:
				x = gsl_ran_gamma(r, RNG_GAMMA_A, RNG_GAMMA_B);
				break;
			/* Gaussian distribution. */
			case RNG_GAUSSIAN:
				x = gsl_ran_gaussian(r, RNG_GAUSSIAN_STDDEV) + RNG_GAUSSIAN_MEAN;
				break;
			/* Uniform distribution. */
			case RNG_UNIFORM:
				x = gsl_ran_flat(r, RNG_UNIFORM_MIN, RNG_UNIFORM_MAX);
				break;
		}
		
		printf("%lf %lf", x, y);
	}
	
	/* House keeping. */		
	gsl_rng_free(r);
	
	return (EXIT_SUCCESS);
}
