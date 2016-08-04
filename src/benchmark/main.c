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

#include <rng.h>
#include <util.h>

#include <benchmark.h>

/**
 * @brief Sorting order types.
 */
#define SORT_ASCENDING  1 /**< Ascending sort.  */
#define SORT_DESCENDING 2 /**< Descending sort. */
#define SORT_RANDOM     3 /**< Random sort.     */

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
	int sort;           /**< Sorting order.                         */
} args = { NULL, 0, 0, 0, 0, 0 };

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
	printf("  --niterations <num>   Number of loop iterations\n");
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

/*============================================================================*
 *                             Argument Checking                              *
 *============================================================================*/

/**
 * @brief Checks program arguments.
 */
static void chkargs(const char *sortname)
{
	/* Check arguments. */
	if (args.nthreads == 0)
		error("invalid number of threads");
	else if (args.ntasks == 0)
		error("invalid number of loop iterations");
	else if (args.load == 0)
		error("invalid kernel load");
	else if (sortname == NULL)
		error("invalid tasks sorting type");
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
	const char *sortname = NULL;
	
	/* Parse command line arguments. */
	for (int i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "--niterations"))
			args.ntasks = atoi(argv[++i]);
		else if (!strcmp(argv[i], "--load"))
			args.load = atol(argv[++i]);
		else if (!strcmp(argv[i], "--nthreads"))
			args.nthreads = atof(argv[++i]);
		else if (!strcmp(argv[i], "--help"))
			usage();
		else if (!strcmp(argv[i], "--sort"))
			sortname = argv[++i];
		else if (!strcmp(argv[i], "--input"))
			args.input = argv[++i];
	}
	
	/* Check arguments. */
	chkargs(sortname);
	
	/* Get secondary argument parameters. */
	args.sort = getsort(sortname);
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

/**
 * @brief Sorts tasks.
 * 
 * @param tasks  Target tasks.
 * @param ntasks Number of tasks.
 * @param type   Sorting type.
 * @param seed   Seed for shuffling.
 */
static void tasks_sort(unsigned *tasks, unsigned ntasks, int type, int seed)
{
	/* Random sort. */
	if (type == SORT_RANDOM)
		array_shuffle(tasks, ntasks, seed);

	/* Ascending sort. */
	else if (type == SORT_ASCENDING)
		qsort(tasks, ntasks, sizeof(unsigned), greater);

	/* Descending sort. */
	else
		qsort(tasks, ntasks, sizeof(unsigned), less);
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
 * @brief Loop scheduler simulator.
 */
int main(int argc, const const char **argv)
{
	unsigned *tasks;
	
	readargs(argc, argv);
	
	/* Run synthetic benchmark. */
	for (int j = 0; j < NITERATIONS; j++)
	{
		tasks = readfile(args.input, args.ntasks);

		tasks_sort(tasks, args.ntasks, args.sort, 1);
		benchmark(tasks, args.ntasks, args.nthreads, args.load);

		free(tasks);
	}
	
	return (EXIT_SUCCESS);
}
