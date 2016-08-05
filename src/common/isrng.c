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
#include <rng.h>

/**
 * @brief Parameters for pseudo-random number generators.
 */
/**@{*/
#define SKEWNESS 0.75
/**@}*/

/**
 * @brief Input data generator for the IS kernel.
 */
int *isrng(int nclasses, long *nnumbers, int pdfid)
{
	long n, k;
	double *h;
	int *numbers;
	
	/* Sanity check. */
	assert(nclasses > 0);
	assert(nnumbers != NULL);
	assert((pdfid > 0) && (pdfid < NR_PDFS));
	
	/* Generate input data. */
	switch (pdfid)
	{
		/* Beta distribution. */
		case RNG_BETA:
			h = beta(nclasses, SKEWNESS);
			break;
			
		/* Gamma distribution. */
		case RNG_GAMMA:
			h = gamma(nclasses, SKEWNESS);
			break;
			
		/* Gaussian distribution. */
		case RNG_GAUSSIAN:
			h = gaussian(nclasses, SKEWNESS);
			break;
			
		/* Fall trough. */
		default:
			
		/* Poisson distribution. */
		case RNG_POISSON:
			h = poisson(nclasses, SKEWNESS);
			break;
	}
	
	
	/* Adjust number of numbers. */
	n = 0;
	for (int i = 0; i < nclasses; i++)
		n += ceil(*nnumbers*h[i]);
	
	numbers = smalloc(n*sizeof(int));
	
	/* Generate input data. */
	k = 0;
	for (int i = 0; i < nclasses; i++)
	{
		long _nnumbers = ceil((*nnumbers)*h[i]);
		
		for (long j = 0; j < _nnumbers; j++)
			numbers[k++] = i;
	}
	*nnumbers = n;
	
	/* House keeping. */
	free(h);
	
	return (numbers);
}
