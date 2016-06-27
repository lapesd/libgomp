/*
 * Copyright(C) 2015 Pedro H. Penna <pedrohenriquepenna@gmail.com>
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

#include <float.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <time.h>

#include <rng.h>
#include <util.h>

#include <benchmark.h>

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
#define RNG_POISSON  4 /**< Poisson.  */
/**@}*/

/**
 * @brief Parameters for pseudo-random number generators.
 */
/**@{*/
#define SKEWNESS 0.75
/**@}*/

/**
 * @brief Sorting order types.
 */
#define SORT_ASCENDING  1 /**< Ascending sort.  */
#define SORT_DESCENDING 2 /**< Descending sort. */
#define SORT_RANDOM     3 /**< Random sort.     */

/**
 * @brief Name of supported probability density functions.
 */
static const char *pdfnames[NR_PDFS] = {
	"beta",     /* Beta.     */
	"gamma",    /* Gammma.   */
	"gaussian", /* Gaussian. */
	"poisson"   /* Poisson.  */
};

/**
 * @brief Name of supported kernel types.
 */
static const char *kernelnames[NR_KERNELS] = {
	"linear",    /* Linear.    */
	"logarithm", /* Logarithm. */
	"quadratic", /* Quadratic. */
	"cubic"      /* Cubic.     */
};

/**
 * @name Program Parameters
 */
static struct
{
	unsigned nthreads;  /**< Number of threads.                     */
	unsigned ntasks;    /**< Number of tasks.                       */
	unsigned chunksize; /**< Chunk size for the dynamic scheduling. */
	unsigned load;      /**< Kernel load.                           */
	unsigned pdfid;     /**< Probability density function.          */
	int kernel;         /**< Kernel type.                           */
	double skewness;    /**< Probability density function skewness. */
	int sort;           /**< Sorting order.                         */
} args = { 0, 0, 0, 0, 0, 0, 0.0, 0 };

/**
 * @brief Prints program usage and exits.
 * 
 * @details Prints program usage and exits gracefully.
 */
static void usage(void)
{
	printf("Usage: scheduler [options] <scheduler>\n");
	printf("Brief: loop scheduler simulator\n");
	printf("Options:\n");
	printf("  --help                Display this message\n");
	printf("  --kernel <name>       Kernel type\n");
	printf("           linear       Linear O(n)\n");
	printf("           logarithm    Logarithm O(n log n)\n");
	printf("           quadratic    Quadratic O(n^2)\n");
	printf("           cubic        Cubic O(n^3)\n");
	printf("  --load <num>          kernel load\n");
	printf("  --nthreads <num>      Number of threads\n");
	printf("  --niterations <num>   Number of loop iterations\n");
	printf("  --pdf <name>          Probability density function for random numbers.\n");
	printf("        beta              a = 0.5 and b = 0.5\n");
	printf("        gamma             a = 1.0 and b = 2.0 \n");
	printf("        gaussian          x = 0.0 and std = 1.0\n");
	printf("        poisson                                \n");
	printf("  --skewness <num>      Skewness for probability density function\n");
	printf("  --sort <type>         Loop sorting\n");
	printf("         ascending      Ascending order\n");
	printf("         descending     Descending order\n");
	printf("         random         Random order\n");

	exit(EXIT_SUCCESS);
}

/*============================================================================*
 *                                Get Routines                                *
 *============================================================================*/

/**
 * @brief Gets tasks sorting type.
 * 
 * @param sortname Tasks sorting name.
 * 
 * @param Tasks sorting type.
 */
static int getsort(const char *sortname)
{
	if (!strcmp(sortname, "ascending"))
		return (SORT_ASCENDING);
	else if (!strcmp(sortname, "descending"))
		return (SORT_DESCENDING);
	else if (!strcmp(sortname, "random"))
		return (SORT_RANDOM);
	
	error("unsupported tasks sorting type");
	
	/* Never gets here. */
	return (-1);
}

/**
 * @brief Gets PDF id.
 * 
 * @param pdfname PDF name.
 * 
 * @returns PDF id.
 */
static int getpdf(const char *pdfname)
{
	for (int i = 0; i < NR_PDFS; i++)
	{
		if (!strcmp(pdfname, pdfnames[i]))
			return (i + 1);
	}
	
	error("unsupported probability density function");
	
	/* Never gets here. */
	return (-1);
}

/**
 * @brief Gets kernel type.
 * 
 * @param kernelname Kernel name.
 * 
 * @returns Kernel type.
 */
static int getkernel(const char *kernelname)
{
	for (int i = 0; i < NR_KERNELS; i++)
	{
		if (!strcmp(kernelname, kernelnames[i]))
			return (i + 1);
	}
	
	error("unsupported kernel type");
	
	/* Never gets here. */
	return (-1);
}

/*============================================================================*
 *                             Argument Checking                              *
 *============================================================================*/

/**
 * @brief Checks program arguments.
 */
static void chkargs(const char *sortname, const char *pdfname, const char *kernelname)
{
	/* Check arguments. */
	if (args.nthreads == 0)
		error("invalid number of threads");
	else if (args.ntasks == 0)
		error("invalid number of loop iterations");
	else if (args.skewness == 0.0)
		error("invalid skewness for probability density function");
	else if (args.load == 0)
		error("invalid kernel load");
	else if (sortname == NULL)
		error("invalid tasks sorting type");
	else if (pdfname == NULL)
		error("invalid probability density function");
	else if (kernelname == NULL)
		error("invalid kernel type");
}

/*============================================================================*
 *                              Argument Reading                              *
 *============================================================================*/

/**
 * @brief Reads command line arguments.
 * 
 * @param argc Argument count.
 * @param argv Argument list.
 */
static void readargs(int argc, const char **argv)
{
	const char *pdfname = NULL;
	const char *sortname = NULL;
	const char *kernelname = NULL;
	
	/* Parse command line arguments. */
	for (int i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "--niterations"))
			args.ntasks = atoi(argv[i + 1]);
		else if (!strcmp(argv[i], "--kernel"))
			kernelname = argv[i + 1];
		else if (!strcmp(argv[i], "--load"))
			args.load = atof(argv[i + 1]);
		else if (!strcmp(argv[i], "--nthreads"))
			args.nthreads = atof(argv[i + 1]);
		else if (!strcmp(argv[i], "--pdf"))
			pdfname = argv[i + 1];
		else if (!strcmp(argv[i], "--help"))
			usage();
		else if (!strcmp(argv[i], "--skewness"))
			args.skewness = atof(argv[i + 1]);
		else if (!strcmp(argv[i], "--sort"))
			sortname = argv[i + 1];
	}
	
	/* Check arguments. */
	chkargs(sortname, pdfname, kernelname);
	
	/* Get secondary argument parameters. */
	args.sort = getsort(sortname);
	args.pdfid = getpdf(pdfname);
	args.kernel = getkernel(kernelname);
}

/**
 * @brief Builds tasks histogram.
 * 
 * @param pdf         Probability density functions.
 * @param ntasks Number of tasks.
 * @param skewness    Skewness for probability density function.
 * 
 * @returns tasks histogram.
 */
static double *histogram_create(unsigned pdf, unsigned ntasks, double skewness)
{
	double *h = NULL;
	
	/* Generate input data. */
	switch (pdf)
	{
		/* Beta distribution. */
		case RNG_BETA:
			h = beta(ntasks, skewness);
			break;
			
		/* Gamma distribution. */
		case RNG_GAMMA:
			h = gamma(ntasks, skewness);
			break;
			
		/* Gaussian distribution. */
		case RNG_GAUSSIAN:
			h = gaussian(ntasks, skewness);
			break;
			
		/* Poisson distribution. */
		case RNG_POISSON:
			h = poisson(ntasks, skewness);
			break;
			
		/* Shouldn't happen. */
		default:
			error("unsupported probability density function");
			break;
	}
	
	return (h);
}

/**
 * @brief Greater than.
 * 
 * @param a1 First element.
 * @param a2 Second element.
 * 
 * @returns One if @p a1 is greater than @p a2 and minus one otherwise.
 */
static int greater(const void *a1, const void *a2)
{
	return ((*((unsigned *)a1) > *((unsigned *)a2)) ? 1 : -1);
}

/**
 * @brief Less than.
 * 
 * @param a1 First element.
 * @param a2 Second element.
 * 
 * @returns One if @p a1 is less than @p a2 and minus one otherwise.
 */
static int less(const void *a1, const void *a2)
{
	return ((*((unsigned *)a1) < *((unsigned *)a2)) ? 1 : -1);
}

/**
 * @brief Shuffles and array.
 * 
 * @param a Target array.
 * @param n Size of target array.
 */
static void array_shuffle(unsigned *a, unsigned n)
{
	/* Let us be totally random. */
	srand(time(NULL));
	
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

/**
 * @brief Sorts tasks.
 * 
 * @param tasks  Target tasks.
 * @param ntasks Number of tasks.
 * @param type   Sorting type.
 */
static void tasks_sort(unsigned *tasks, unsigned ntasks, int type)
{
	/* Random sort. */
	if (type == SORT_RANDOM)
		array_shuffle(tasks, ntasks);

	/* Ascending sort. */
	else if (type == SORT_ASCENDING)
		qsort(tasks, ntasks, sizeof(unsigned), greater);

	/* Descending sort. */
	else
		qsort(tasks, ntasks, sizeof(unsigned), less);
}

/**
 * @brief Create tasks.
 * 
 * @param h tasks histogram.
 * @param ntasks Number of tasks.
 * 
 * @returns Tasks.
 */
static unsigned *tasks_create(const double *h, unsigned ntasks)
{
	unsigned *tasks;
	const unsigned FACTOR = 100000000;
	
	tasks = smalloc(ntasks*sizeof(unsigned));
	
	for (unsigned i = 0; i < ntasks; i++)
	{
		double x;
		
		x = h[i]*FACTOR;
		
		/* Check for corner cases. */
		if (x < 0)
			error("bad multiplying factor");
		
		tasks[i] = ceil(x);
	}
	
	return (tasks);
}

/**
 * @brief Loop scheduler simulator.
 */
int main(int argc, const const char **argv)
{
	double *h;       /* Data tasks histogram. */
	unsigned *tasks; /* tasks tasks.          */
	
	readargs(argc, argv);
	
	/* Build synthetic tasks */
	h = histogram_create(args.pdfid, args.ntasks, args.skewness);
	tasks = tasks_create(h, args.ntasks);
	tasks_sort(tasks, args.ntasks, args.sort);

	/* Run synthetic benchmark. */
	for (int i = 0; i < NITERATIONS; i++)
		benchmark(tasks, args.ntasks, args.nthreads, args.kernel, args.load);
		
	/* House keeping. */
	free(h);;
	free(tasks);
	
	return (EXIT_SUCCESS);
}
