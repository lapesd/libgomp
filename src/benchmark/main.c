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
	const char *input;  /**< Input data file.                       */
	unsigned nthreads;  /**< Number of threads.                     */
	unsigned ntasks;    /**< Number of tasks.                       */
	unsigned chunksize; /**< Chunk size for the dynamic scheduling. */
	unsigned load;      /**< Kernel load.                           */
} args = { NULL, 0, 0, 0, 1};

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
	printf("  --input <filename>    Input workload file\n");
	printf("  --load <num>          kernel load\n");
	printf("  --nthreads <num>      Number of threads\n");

	exit(EXIT_SUCCESS);
}

/*============================================================================*
 *                             Argument Checking                              *
 *============================================================================*/

/**
 * @brief Checks program arguments.
 */
static void chkargs(void)
{
	/* Check arguments. */
	if (args.nthreads == 0)
		error("invalid number of threads");
	else if (args.load == 0)
		error("invalid kernel load");
	else if (args.input == NULL)
		error("missing input workload file");
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
	/* Parse command line arguments. */
	for (int i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "--load"))
			args.load = atol(argv[++i]);
		else if (!strcmp(argv[i], "--nthreads"))
			args.nthreads = atof(argv[++i]);
		else if (!strcmp(argv[i], "--help"))
			usage();
		else if (!strcmp(argv[i], "--input"))
			args.input = argv[++i];
	}

	chkargs();
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
