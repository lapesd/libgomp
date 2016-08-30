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
#include <stdlib.h>
#include <stdio.h>

#include <util.h>
#include <smm.h>

/**
 * Program arguments.
 */
static struct
{
	int n;     /*< Matrix dimension. */
	double *a; /*< Matrix A.         */
	double *b; /*< Matrix B.         */
} args = {0, NULL, NULL};

/**
 * @brief Reads a matrix from a file.
 *
 * @param infile Input file.
 * @param n      Matrix dimension.
 *
 * @returns The matrix.
 */
static double *matrix_read(FILE *infile, int n)
{
	double *m, *p;

	m = p = smalloc(args.n*args.n*sizeof(double));

	/* Read matrix. */
	for (int c = 0; c < 2; c++)
	{
		for (int i = 0; i < n; i++)
		{
			for (int j = 0; j < n; j++)
				assert(fscanf(infile, "%lf", p++) == 1);
		}
	}

	return (m);
}

/**
 * @brief Reads input file. 
 * 
 * @param filename Input filename.
 *
 * @todo Check for bad input file format. 
 */
static void readinput(const char *filename)
{
	FILE *infile;
	
	/* Sanity check. */
	assert(filename != NULL);
	
	/* Open input file. */
	infile = fopen(filename, "r");
	assert(infile != NULL);
	
	/* Read input matrices. */
	assert(fscanf(infile, "%d", &args.n) == 1);
	args.a = matrix_read(infile, args.n);
	args.b = smalloc(args.n*args.n*sizeof(double));

	/* House keeping. */
	fclose(infile);
}

/**
 * @brief Prints program usage and exits.
 */
static void usage(void)
{
	printf("Usage: smm <input>\n");
	printf("Brief: Sparse Matrix Multiplication kernell\n");

	exit(EXIT_SUCCESS);
}

/**
 * @brief Sparse Matrix Multiplication kernel.
 */
int main(int argc, const char **argv)
{
	/* Wrong usage. */
	if (argc < 2)
		usage();
		
	/* Read input data. */
	readinput(argv[1]);

	/* Run kernel. */
	smm_init(args.a, args.b, args.n);
	smm_run(0);
	smm_end();


	/* House keeping. */
	free(args.a);
	free(args.b);

	return (EXIT_SUCCESS);
}
