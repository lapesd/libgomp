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
#define RNG_UNIFORM  4 /**< Uniform.  */
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
 * @brief Number of supported kernels.
 */
#define NR_KERNELS 4

/**
 * @brief Name of supported kernel types.
 */
/**@{*/
#define KERNEL_LINEAR    1
#define KERNEL_LOGARITHM 2
#define KERNEL_QUADRATIC 3
#define KERNEL_CUBIC     4
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
	unsigned nthreads;     /**< Number of threads.                     */
	unsigned niterations;  /**< Number of tasks.                       */
	unsigned chunksize;    /**< Chunk size for the dynamic scheduling. */
	unsigned load;         /**< Kernel load.                           */
	unsigned pdfid;        /**< Probability density function.          */
	int kernelid;          /**< Kernel type.                           */
	double skewness;       /**< Probability density function skewness. */
	int sort;              /**< Sorting order.                         */
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
	printf("  --pdf <name>          Probability desity function for random numbers.\n");
	printf("        beta              a = 0.5 and b = 0.5\n");
	printf("        gamma             a = 1.0 and b = 2.0 \n");
	printf("        gaussian          x = 0.0 and std = 1.0\n");
	printf("        uniform           a = 0.0 and b = 1.0\n");
	printf("  --skewness <num>      Skewness for probability density function\n");
	printf("  --sort <type>         Task sorting\n");
	printf("         ascending      Ascending order\n");
	printf("         descending     Descending order\n");
	printf("         random         Random order\n");

	exit(EXIT_SUCCESS);
}

/**
 * @brief Reads command line arguments.
 * 
 * @details Reads command line arguments.
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
			args.niterations = atoi(argv[i + 1]);
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
	if (args.nthreads == 0)
		error("invalid number of threads");
	else if (args.niterations == 0)
		error("invalid number of loop iterations");
	else if (args.skewness == 0.0)
		error("invalid skewness for probability density function");
	else if (args.load == 0)
		error("invalid kernel load");
	else if (sortname != NULL)
	{
		if (!strcmp(sortname, "ascending"))
			args.sort = SORT_ASCENDING;
		else if (!strcmp(sortname, "descending"))
			args.sort = SORT_DESCENDING;
		else if (!strcmp(sortname, "random"))
			args.sort = SORT_RANDOM;
		else
			error("unsupported sorting order");
	}
	if (pdfname != NULL)
	{
		for (int i = 0; i < NR_PDFS; i++)
		{
			if (!strcmp(pdfname, pdfnames[i]))
			{
				args.pdfid = i + 1;
				goto out1;
			}
		}
	}
	error("unsupported probability density function"); 
	
out1:
	if (kernelname)
	{
		for (int i = 0; i < NR_KERNELS; i++)
		{
			if (!strcmp(kernelname, kernelnames[i]))
			{
				args.kernelid = i + 1;
				goto out;
			}
		}
	}
	error("unsupported kernel type");

out:
	return;
}

/**
 * @brief Builds workload histogram.
 * 
 * @param pdf         Probability density functions.
 * @param niterations Number of iterations in the workload.
 * @param skewness    Skew ness for probability density function.
 * 
 * @returns Workload histogram.
 */
static double *histogram_create(unsigned pdf, unsigned niterations, double skewness)
{
	double *h;
	
	/* Generate input data. */
	switch (pdf)
	{
		/* Beta distribution. */
		case RNG_BETA:
			h = beta(niterations, skewness);
			break;
			
		/* Gamma distribution. */
		case RNG_GAMMA:
			h = gamma(niterations, skewness);
			break;
			
		/* Gaussian distribution. */
		case RNG_GAUSSIAN:
			h = gaussian(niterations, skewness);
			break;
			
		/* Fall trough. */
		default:
			
		/* Uniform distribution. */
		case RNG_UNIFORM:
			h = uniform(niterations, skewness);
			break;
	}
	
	return (h);
}

/**
 * @brief Swaps two unsigned integers.
 * 
 * @param u1 First unsigned integer.
 * @param u2 Second unsigned integer. 
 */
static inline void swap(unsigned *u1, unsigned *u2)
{
	unsigned t;
	
	t = *u1;
	*u1 = *u2;
	*u2 = t;
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
	{
		for (unsigned i = 0; i < ntasks - 1; i++)
		{
			unsigned j;
			
			j = i + rand()/(RAND_MAX/(ntasks - i) + 1);
			
			swap(&tasks[i], &tasks[j]);
		}
	}

	/* Ascending sort. */
	else if (type == SORT_ASCENDING)
	{	
		for (unsigned i = 0; i < ntasks; i++)
		{
			for (unsigned j = i + 1; j < ntasks; j++)
			{
				if (tasks[j] < tasks[i])
					swap(&tasks[i], &tasks[j]);
			}
		}
	}

	/* Descending sort. */
	else
	{
		for (unsigned i = 0; i < ntasks; i++)
		{
			for (unsigned j = i + 1; j < ntasks; j++)
			{
				if (tasks[j] > tasks[i])
					swap(&tasks[i], &tasks[j]);
			}
		}
	}
}

/**
 * @brief Create tasks.
 * 
 * @brief h Workload histogram.
 * @param kernel Kernel type.
 * 
 * @returns Tasks.
 */
static unsigned *tasks_create(double *h, unsigned niterations, int kernel)
{
	unsigned *tasks;
	const int FACTOR = 100000000;
	
	tasks = smalloc(niterations*sizeof(unsigned));
	
	for (unsigned i = 0; i < niterations; i++)
	{
		double x;
		
		x = h[i]*FACTOR;
		
		/* Check for corner cases. */
		if (x < 0)
			error("bad multiplying factor");
		
		switch (kernel)
		{
			/* Logarithm kernel. */
			case KERNEL_LOGARITHM:
				x = x*(log(x)/log(2));
				break;
				
			/* Quadratic kernel. */
			case KERNEL_QUADRATIC:
				x = pow(x, 2);
				break;
				
			/* Cubic kernel. */
			case KERNEL_CUBIC:
				x = pow(x, 3);
				break;
				
			/* Linear kernel. */
			case KERNEL_LINEAR:
			default:
				break;
		}
		tasks[i] = ceil(x);
	}
	
	return (tasks);
}

/**
 * @brief Loop scheduler simulator.
 */
int main(int argc, const const char **argv)
{
	double *h;       /* Data workload histogram. */
	unsigned *tasks; /* Workload tasks.          */
	
	readargs(argc, argv);
	
	/* Build synthetic workload */
	h = histogram_create(args.pdfid, args.niterations, args.skewness);
	tasks = tasks_create(h, args.niterations, args.kernelid);
	tasks_sort(tasks, args.niterations, args.sort);

	/* Run synthetic benchmark. */
	for (int i = 0; i < NITERATIONS; i++)
		benchmark(tasks, args.niterations, args.nthreads, args.load);
		
	/* House keeping. */
	free(h);;
	free(tasks);
	
	return (EXIT_SUCCESS);
}
