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

#include <assert.h>
#include <float.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <time.h>

#include <util.h>

#include <benchmark.h>

/**
 * @name Program Parameters
 */
static struct
{
	const char *input;                    /**< Input data file.                       */
	unsigned nthreads;                    /**< Number of threads.                     */
	unsigned ntasks;                      /**< Number of tasks.                       */
	unsigned chunksize;                   /**< Chunk size for the dynamic scheduling. */
	unsigned load;                        /**< Kernel load.                           */
	void (*kernel)(unsigned *, unsigned); /**< Application kernel.       */
} args = { NULL, 0, 0, 0, 1, NULL };

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
	printf("  --kernel <name>       Kernel complexity.\n");
	printf("           linear          Linear kernel\n");
	printf("           logarithmic     Logarithm kernel\n");
	printf("           quadratic       Quadratic kernel\n");
	printf("  --input <filename>    Input workload file\n");
	printf("  --load <num>          kernel load\n");
	printf("  --nthreads <num>      Number of threads\n");

	exit(EXIT_SUCCESS);
}

/*============================================================================*
 * KERNELS                                                                    *
 *============================================================================*/

/**
 * @brief Applies a linear kernel in a workload
 *
 * @param w     Target workload.
 * @param wsize Size of target workload.
 */
static void kernel_linear(unsigned *w, unsigned wsize)
{
	((void) w);
	((void) wsize);
}

/**
 * @brief Applies a logarithmic kernel in a workload
 *
 * @param w     Target workload.
 * @param wsize Size of target workload.
 */
static void kernel_logarithmic(unsigned *w, unsigned wsize)
{
	for (unsigned i = 0; i < wsize; i++)
	{
		unsigned load;

		load = w[i];
		w[i] = floor(load*(log(load)/log(2.0)));
	}
}

/**
 * @brief Applies a quadratic kernel in a workload
 *
 * @param w     Target workload.
 * @param wsize Size of target workload.
 */
static void kernel_quadratic(unsigned *w, unsigned wsize)
{
	for (unsigned i = 0; i < wsize; i++)
	{
		unsigned load;

		load = w[i];
		w[i] = load*load;
	}
}

/*============================================================================*
 *                             Argument Checking                              *
 *============================================================================*/

/**
 * @brief Checks program arguments.
 *
 * @param kernelname Application kernel name.
 */
static void chkargs(const char *kernelname)
{
	/* Check arguments. */
	if (args.nthreads == 0)
		error("invalid number of threads");
	else if (args.load == 0)
		error("invalid kernel load");
	else if (args.input == NULL)
		error("missing input workload file");
	if (kernelname == NULL)
		error("missing kernel name");
}

/*============================================================================*
 *                              Argument Reading                              *
 *============================================================================*/

/**
 * @brief Gets application kernel.
 *
 * @param kernelname Kernel name.
 *
 * @returns Application kernel.
 */
static void (*get_kernel(const char *kernelname))(unsigned *, unsigned)
{
	if (!strcmp(kernelname, "linear"))
		return (kernel_linear);
	if (!strcmp(kernelname, "logarithmic"))
		return (kernel_logarithmic);
	if (!strcmp(kernelname, "quadratic"))
		return (kernel_quadratic);

	error("unsupported application kernel");

	/* Never gets here. */
	return (NULL);
}

/**
 * @brief Reads command line arguments.
 * 
 * @param argc Argument count.
 * @param argv Argument list.
 */
static void readargs(int argc, const char **argv)
{
	const char *kernelname = NULL;

	/* Parse command line arguments. */
	for (int i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "--load"))
			args.load = atol(argv[++i]);
		else if (!strcmp(argv[i], "--nthreads"))
			args.nthreads = atof(argv[++i]);
		else if (!strcmp(argv[i], "--help"))
			usage();
		else if (!strcmp(argv[i], "--kernel"))
			kernelname = argv[++i];
		else if (!strcmp(argv[i], "--input"))
			args.input = argv[++i];
	}

	chkargs(kernelname);

	args.kernel = get_kernel(kernelname);
}

/**
 * @brief Reads input file
 * 
 * @param input Input filename.
 */
static unsigned *readfile(const char *input)
{
	FILE *fp;
	unsigned *tasks;
	
	fp = fopen(input, "r");
	assert(fp != NULL);

	assert(fscanf(fp , "%u", &args.ntasks) == 1);

	tasks = smalloc(args.ntasks*sizeof(unsigned));
	
	/* Read file. */
	for (unsigned i = 0; i < args.ntasks; i++)
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

	args.kernel(tasks, args.ntasks);

	return (tasks);
}

/**
 * @brief Loop scheduler simulator.
 */
int main(int argc, const const char **argv)
{
	unsigned *tasks;
	
	readargs(argc, argv);
	
	tasks = readfile(args.input);

	benchmark(tasks, args.ntasks, args.nthreads, args.load);

	free(tasks);
	
	return (EXIT_SUCCESS);
}
