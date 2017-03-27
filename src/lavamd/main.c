/*
 * Copyright(C) 2017 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * CAP Benchmarks is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of the
 * License, or (at your option) any later version.
 * 
 * CAP Benchmarks is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with CAP Benchmarks. If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <util.h>

extern void kernel(int *, int, int);

/*============================================================================*
 * Argument Checking                                                          *
 *============================================================================*/

/**
 * @brief Program arguments.
 */
static struct
{
	const char *input; /**< Input data file.           */
	int nthreads;      /**< Number of working threads. */
	int ntasks;        /**< Number of tasks.           */
} args = { NULL, 0, 0};

/**
 * @brief Prints program usage and exits.
 */
static void usage(void)
{
	printf("Usage: lava [options]\n");
	printf("Brief: LavaMD Kernel\n");
	printf("Options:\n");
	printf("  --help                Display this information\n");
	printf("  --input <filename>    Input workload file\n");
	printf("  --nthreads <number> Set the number of working threads\n");
	
	exit(EXIT_SUCCESS);
}

/**
 * @brief Checks program arguments.
 */
static void checkargs(void)
{
	if (args.nthreads == 0)
		error("invalid number of threads");
	else if (args.input == NULL)
		error("missing input workload file");
}

/**
 * @brief Reads program arguments and exits.
 */
static void readargs(int argc, const char **argv)
{
	for (int i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "--nthreads"))
			args.nthreads = atoi(argv[++i]);
		else if (!strcmp(argv[i], "--input"))
			args.input = argv[++i];
		else 
			usage();
	}

	checkargs();
}

/*============================================================================*
 * Input/Output                                                               *
 *============================================================================*/

/**
 * @brief Reads input file
 * 
 * @param input Input filename.
 */
static int *readfile(const char *input)
{
	FILE *fp;
	int *tasks;
	
	fp = fopen(input, "r");
	assert(fp != NULL);

	assert(fscanf(fp , "%d", &args.ntasks) == 1);

	tasks = smalloc(args.ntasks*sizeof(int));
	
	/* Read file. */
	for (int i = 0; i < args.ntasks; i++)
	{
		if (fscanf(fp, "%d", &tasks[i]) == EOF)
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

/*============================================================================*
 * Main                                                                       *
 *============================================================================*/

int main(int argc, const char **argv)
{
	int *tasks;

	readargs(argc, argv);

	tasks = readfile(args.input);

	kernel(tasks, args.ntasks, args.nthreads);

	/* House keeping. */
	free(tasks);

	return (EXIT_SUCCESS);
}
