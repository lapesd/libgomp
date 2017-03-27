/*
 * Copyright(C) 2017 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * CAP Benchmarks is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 * 
 * CAP Benchmarks is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with CAP Benchmarks. If not, see <http://www.gnu.org/licenses/>.
 */

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <limits.h>

/**
 * @brief Dumps execution statistics.
 *
 * @param respvar  Response variable.
 * @param nthreads Number of threads.
 * @param prefix   Prefix for response variable.
 */
void dump(const double *respvar, int nthreads, const char *prefix)
{
	double min, max, total;
	double mean, stddev;

	min = UINT_MAX; max = 0;
	total = 0; mean = 0.0; stddev = 0.0;

	/* Compute min, max, total. */
	for (int i = 0; i < nthreads; i++)
	{
		double wtotal;

		wtotal = respvar[i];

		if (min > wtotal)
			min = wtotal;
		if (max < wtotal)
			max = wtotal;

		total += wtotal;

		printf("thread_%d_%s: %lf\n", i, prefix, wtotal);
	}

	/* Compute mean. */
	mean = ((double) total)/nthreads;

	/* Compute stddev. */
	for (int i = 0; i < nthreads; i++)
		stddev += pow(respvar[i] - mean, 2);
	stddev = sqrt(stddev/(nthreads));

	/* Print statistics. */
	printf("%s_min: %lf\n", prefix, min);
	printf("%s_max: %lf\n", prefix, max);
	printf("%s_mean: %lf\n", prefix, mean);
	printf("%s_stddev: %lf\n", prefix, 100*stddev/mean);
	printf("%s_imbalance: %lf\n", prefix, 100*(max - min)/((double) total));
	printf("%s_speeddown: %lf\n", prefix, max/((double) min));
	printf("%s_cost: %lf\n", prefix, nthreads*max);
}
