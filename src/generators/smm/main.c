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

#include <util.h>

/**
 * @brief Program arguments.
 */
struct
{
	const char *input; /**< Input workload file.       */
	int ntasks;        /**< Number of workload tasks. */
	int n;             /**< Matrix size.               */
} args = { NULL, 0, 0 };
 
/**
 * @brief Prints program usage and exits.
 */
static void usage(void)
{
	printf("Usage: smm.gen [options]\n");
	printf("Brief: SMM kernel input data generator\n");
	printf("Options:\n");
	printf("  --help             Prints this information and exits\n");
	printf("  --ntasks <number>  Number of tasks\n");
	printf("  --input <filename> Input workload file\n");
	printf("  --msize <number>   Matrix size\n");
	
	exit(EXIT_SUCCESS);
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
	if (args.n == 0)
		error("invalid matrix size");
	else if (args.ntasks == 0)
		error("invalid number of tasks");
	else if (args.input == NULL)
		error("missing input workload file");
	else if (args.n % args.ntasks)
		error("bad matrix size");
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
		if (!strcmp(argv[i], "--ntasks"))
			args.ntasks = atoi(argv[++i]);
		else if (!strcmp(argv[i], "--msize"))
			args.n = atoi(argv[++i]);
		else if (!strcmp(argv[i], "--input"))
			args.input = argv[++i];
		else if (!strcmp(argv[i], "--help"))
			usage();
	}
	
	/* Check arguments. */
	chkargs();
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

/**
 * @brief Reads input file
 * 
 * @param input Input filename.
 * @param ntasks Number of tasks.
 *
 * @returns Workload.
 */
static double *readfile(const char *input, unsigned ntasks)
{
	FILE *fp;
	unsigned greatest;
	unsigned *tasks;
	double *workload;
	
	tasks = smalloc(ntasks*sizeof(unsigned));
	workload = smalloc(ntasks*sizeof(double));

	fp = fopen(input, "r");
	assert(fp != NULL);
	
	/* Read file. */
	greatest = 0;
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

		if (tasks[i] > greatest)
			greatest = tasks[i];
	}
	
	/* I/O error. */
	if (ferror(fp))
		error("cannot read input file");

	array_shuffle(tasks, ntasks, 307);

	for (unsigned i = 0; i < ntasks; i++)
		workload[i] = tasks[i]/((double) greatest);

	/* House keeping. */
	fclose(fp);
	free(tasks);

	return (workload);
}

/**
 * @brief Dumps a matrix.
 *
 * @param n         Matrix dimension.
 * @param is_sparse Dump a sparse matrix?
 * @param workload  Workload.
 * @param ntasks    Workload size.
 */
static void matrix_dump(int n, int is_sparse, double *workload, int ntasks)
{
	double *m;

	m = smalloc(n*n*sizeof(double));

	/* Generate matrix. */
	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < n; j++)
			m[i*n + j] = ((double)rand())/RAND_MAX + 0.1;
	}

	/* Sparse matrix. */
	if (is_sparse)
	{
		for (int k = 0; k < ntasks; k++)
		{
			int nzeros;

			nzeros = n*(n/ntasks) - floor(n*(n/ntasks)*workload[k]);

			while (nzeros-- > 0)
			{
				int i, j;

				do
				{
					i = rand()%(n/ntasks);
					j = rand()%n;
				} while(m[(k*(n/ntasks) + i)*n + j] == 0.0);
				
				m[(k*(n/ntasks) + i)*n + j] = 0.0;
			}
		}

		/* Print matrix statistics. */
		for (int k = 0; k < ntasks; k++)
		{
			int count = 0;

			for (int i = 0; i < (n/ntasks); i++)
			{
				for (int j = 0; j < n; j++)
				{
					if (m[(k*(n/ntasks) + i)*n + j])
						count++;
				}
			}

			fprintf(stderr, "%d: %.2lf\n", k, 100*((double) count)/(n*(n/ntasks)));
		}
	}

	/* Dump matrix. */
	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < n; j++)
			printf("%.1lf ", m[i*n + j]);
		printf("\n");
	}


	/* House keeping. */
	free(m);
}

/**
 * @brief SMM kernel input data generator.
 */
int main(int argc, const char **argv)
{
	double *workload;

	readargs(argc, argv);

	workload = readfile(args.input, args.ntasks);

	/* Dump data. */
	printf("%d\n", args.n);
	matrix_dump(args.n, 1, workload, args.ntasks);
	matrix_dump(args.n, 0, workload, args.ntasks);

	/* House keeping. */
	free(workload);
	
	return (EXIT_SUCCESS);
}
