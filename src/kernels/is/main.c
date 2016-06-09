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

#include <util.h>
#include <is.h>

/**
 * @brief Reads input file. 
 * 
 * @param filename Input filename.
 * @param nnumbers Number of numbers (output parameter).
 * 
 * @returns The integer numbers.
 */
static int *readinput(const char *filename, long *nnumbers)
{
	long _nnumbers; /* Local nnumbers.  */
	FILE *infile;   /* Input file.      */
	int *numbers;   /* Integer numbers. */
	
	/* Sanity check. */
	assert(filename != NULL);
	assert(nnumbers != NULL);
	
	/* Open input file. */
	infile = fopen(filename, "r");
	assert(infile != NULL);
	
	/* Allocate numbers array. */
	assert(fscanf(infile, "%ld", &_nnumbers) == 1);
	numbers = smalloc(_nnumbers*sizeof(int));
	*nnumbers = _nnumbers;
	
	/* Read points. */
	for (int i = 0; i < _nnumbers; i++)
		assert(fscanf(infile, "%d", &numbers[i]) == 1);

	/* House keeping. */
	fclose(infile);

	return (numbers);
}

/**
 * @brief Prints program usage and exits.
 */
static void usage(void)
{
	printf("Usage: is input\n");
	printf("Brief: IS kernel\n");
	exit(EXIT_SUCCESS);
}

/**
 * @brief MST clustering kernel front end.
 */
int main(int argc, char **argv)
{
	long nnumbers; /* Number of numbers. */
	int *numbers;  /* Numbers to sort.   */
	
	/* Wrong usage. */
	if (argc < 2)
		usage();
		
	/* Generate input data for MST. */
	numbers = readinput(argv[1], &nnumbers);
	
	integer_sort(numbers, nnumbers);

	/* House keeping. */
	free(numbers);
	
	return (EXIT_SUCCESS);
}
