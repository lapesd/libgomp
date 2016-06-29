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

#include <util.h>

/**
 * @brief Builds a Beta histogram.
 * 
 * @param nclasses Number of classes.
 * @param skewness Sampling skewness.
 * 
 * @returns A Beta histogram.
 */
double *beta(int nclasses, double skewness)
{
	double sum;        /* PDF sum.         */
	double freq;       /* Class frequency. */
	double *histogram; /* Histogram.       */
	
	/* Sanity check. */
	assert(nclasses > 0);
	assert(skewness > 0.0);
	assert(skewness < 1.0);

	histogram = smalloc(nclasses*sizeof(double));

	/* Build histogram. */
	freq = 0.5; sum = 0.0;
	for (int i = 0; i < nclasses/2; i++)
	{
		if (i < (2*nclasses)/16)
			freq *= skewness;
		else if (i < (4*nclasses)/16)
			freq *= 0.85;
		else if (i < (6*nclasses)/16)
			freq *= 0.90;
		
		histogram[i] = freq;
		histogram[nclasses - i - 1] = freq;
		
		sum += freq + freq;
	}
	
	/* Normalize. */
	for (int i = 0; i < nclasses; i++)
		histogram[i] /= sum;
	
	return (histogram);
}
