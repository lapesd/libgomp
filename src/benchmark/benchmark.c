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
#include <omp.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>

#include <profile.h>
#include <util.h>

#include <benchmark.h>

#if defined(_SCHEDULE_SRR_)
extern void omp_set_workload(unsigned *, unsigned);
#endif

/*============================================================================*
 *                                   Kernel                                   *
 *============================================================================*/

/**
 * @brief Kernelernel.
 * 
 * @param n    Number of operations.
 * @param load Load of an operation.
 * 
 * @returns A dummy result.
 */
static long kernel(unsigned n, long load)
{
	long sum = 0;
 
	for (unsigned i = 0; i < n; i++)
	{
		for (long j = 0; j < load; j++)
			sum += i + j;
	}

	return (sum);
}

/*============================================================================*
 *                                 Benchmark                                  *
 *============================================================================*/

/**
 * @brief Dumps simulation statistics.
 *
 * @param respvar  Response variable.
 * @param nthreads Number of threads.
 * @param prefix   Prefix for response variable.
 */
static void benchmark_dump(const double *respvar, int nthreads, const char *prefix)
{
	unsigned min, max, total;
	double mean, stddev;

	min = UINT_MAX; max = 0;
	total = 0; mean = 0.0; stddev = 0.0;

	/* Compute min, max, total. */
	for (int i = 0; i < nthreads; i++)
	{
		unsigned wtotal;

		wtotal = respvar[i];

		if (min > wtotal)
			min = wtotal;
		if (max < wtotal)
			max = wtotal;

		total += wtotal;
	}

	/* Compute mean. */
	mean = ((double) total)/nthreads;

	/* Compute stddev. */
	for (int i = 0; i < nthreads; i++)
	{
		stddev += pow(respvar[i] - mean, 2);
	}
	stddev = sqrt(stddev/(nthreads));

	/* Print statistics. */
	printf("%s_min: %d\n", prefix, min);
	printf("%s_max: %d\n", prefix, max);
	printf("%s_mean: %lf\n", prefix, mean);
	printf("%s_stddev: %lf\n", prefix, 100*stddev/mean);
	printf("%s_imbalance: %lf\n", prefix, 100*(max - min)/((double) total));
	printf("%s_speeddown: %lf\n", prefix, max/((double) min));
}

/**
 * @brief Synthetic benchmark.
 * 
 * @param tasks    Tasks.
 * @param ntasks   Number of tasks.
 * @param nthreads Number of threads.
 * @param load     Load for constant kernel.
 */
void benchmark(
	const unsigned *tasks,
	unsigned ntasks,
	int nthreads,
	long load)
{
	long sum = 0;
	double loads[nthreads];
	double times[nthreads];

	memset(times, 0, nthreads*sizeof(double));
	
	/* Sanity check. */
	assert(tasks != NULL);
	assert(nthreads > 0);
	assert(load > 0);

	memset(loads, 0, nthreads*sizeof(unsigned));
	
	/* Predict workload. */
#if defined(_SCHEDULE_SRR_)
	unsigned *_tasks;
	_tasks = smalloc(ntasks*sizeof(unsigned));
	memcpy(_tasks, tasks, ntasks*sizeof(unsigned));
#endif
	
	profile_start();

	/* Sort Each bucket. */
#if defined(_SCHEDULE_STATIC_)
	#pragma omp parallel for schedule(static) num_threads(nthreads) reduction(+:sum)
#elif defined(_SCHEDULE_GUIDED_)
	#pragma omp parallel for schedule(guided) num_threads(nthreads) reduction(+:sum)
#elif defined(_SCHEDULE_DYNAMIC_)
	#pragma omp parallel for schedule(dynamic) num_threads(nthreads) reduction(+:sum)
#elif defined(_SCHEDULE_SRR_)
	omp_set_workload(_tasks, ntasks);
	#pragma omp parallel for schedule(runtime) num_threads(nthreads) reduction(+:sum)
#endif
	for (unsigned i = 0; i < ntasks; i++)
	{
		double start = 0.0;

		int tid = omp_get_thread_num();

		loads[tid] += tasks[i];
		
		start = omp_get_wtime();
		sum += kernel(tasks[i], load);
		times[tid] += omp_get_wtime() - start;
	}
	
	profile_end();
	profile_dump();
	
	benchmark_dump(loads, nthreads, "load");
	benchmark_dump(times, nthreads, "time");

	/* House  keeping. */
#if defined(_SCHEDULE_SRR_)
	free(_tasks);
#endif
}
