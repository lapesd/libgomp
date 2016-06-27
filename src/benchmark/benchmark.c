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

#include <profile.h>
#include <util.h>

#include <benchmark.h>

#if defined(_SCHEDULE_SRR_)
extern void omp_set_workload(unsigned *, unsigned);
#endif

/*============================================================================*
 *                                  Kernels                                   *
 *============================================================================*/

/**
 * @brief Constant kernel.
 * 
 * @param n    Number of operations.
 * @param load Load of an operation.
 * 
 * @returns A dummy result.
 */
static long kernel_constant(unsigned n, long load)
{
	long sum = 0;
 
	for (unsigned i = 0; i < n; i++)
	{
		for (long j = 0; j < load; j++)
			sum += i + j;
	}

	return (sum);
}

/**
 * @brief Linear kernel.
 * 
 * @param n    Number of operations.
 * @param load Load of an operation.
 * 
 * @returns A dummy result.
 */
static long kernel_linear(unsigned n, long load)
{
	long sum = 0;
	
	for (unsigned i = 0; i < n; i++)
		sum += kernel_constant(1, load) + i;
	
	return (sum);
}

/**
 * @brief Logarithmic kernel.
 * 
 * @param n    Number of operations.
 * @param load Load of an operation.
 * 
 * @returns A dummy result.
 */
static long kernel_logarithmic(unsigned n, long load)
{
	long sum = 0;

	for (unsigned i = 1; i < n; i *= 2)
		sum += kernel_linear(n, load) + i;

	return (sum);
}

/**
 * @brief Quadratic kernel.
 * 
 * @param n    Number of operations.
 * @param load Load of an operation.
 * 
 * @returns A dummy result.
 */
static long kernel_quadratic(unsigned n, long load)
{
	long sum = 0;

	for (unsigned i = 0; i < n; i++)
		sum += kernel_linear(n, load) + i;

	return (sum);
}

/**
 * @brief Cubic kernel.
 * 
 * @param n    Number of operations.
 * @param load Load of an operation.
 * 
 * @returns A dummy result.
 */
static long kernel_cubic(unsigned n, long load)
{
	long sum = 0;

	for (unsigned i = 0; i < n; i++)
		sum += kernel_quadratic(n, load) + i;

	return (sum);
}

/**
 * @brief Kernels table.
 */
static long (*kernels[NR_KERNELS + 1])(unsigned, long) = {
	kernel_constant,    /* Constant kernel O(1).          */
	kernel_linear,      /* Linear kernel O(n).            */
	kernel_logarithmic, /* Logarithmic kernel O(n log n). */
	kernel_quadratic,   /* Quadratic kernel O(n^2).       */
	kernel_cubic        /* Cubic kernel O(n^3).           */
};

/*============================================================================*
 *                                 Benchmark                                  *
 *============================================================================*/

/**
 * @brief Synthetic benchmark.
 * 
 * @param tasks    Tasks.
 * @param ntasks   Number of tasks.
 * @param nthreads Number of threads.
 * @param ktype    Benchmark kernel type.
 * @param load     Load for constant kernel.
 */
void benchmark(
	const unsigned *tasks,
	unsigned ntasks,
	int nthreads,
	int ktype,
	long load)
{
	unsigned loads[nthreads];       /* Workload assigned to threads. */
	long (*kernel)(unsigned, long); /* Benchmark kernel.             */

	/* Sanity check. */
	assert(tasks != NULL);
	assert(nthreads > 0);
	assert((ktype > 0) && (ktype < NR_KERNELS));
	assert(load > 0);

	kernel = kernels[ktype];

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
	#pragma omp parallel for schedule(static) num_threads(nthreads)
#elif defined(_SCHEDULE_GUIDED_)
	#pragma omp parallel for schedule(guided) num_threads(nthreads)
#elif defined(_SCHEDULE_DYNAMIC_)
	#pragma omp parallel for schedule(dynamic) num_threads(nthreads)
#elif defined(_SCHEDULE_SRR_)
	omp_set_workload(_tasks, ntasks);
	#pragma omp parallel for schedule(runtime) num_threads(nthreads)
#endif
	for (unsigned i = 0; i < ntasks; i++)
	{
		int tid = omp_get_thread_num();

		loads[tid] += tasks[i];
		
		kernel(tasks[i], load);
	}
	
	profile_end();
	profile_dump();
	
	/* Print statistics. */
	for (int i = 0; i < nthreads; i++)
		fprintf(stderr, "thread %d: %u\n", i, loads[i]);
		
	/* House  keeping. */
#if defined(_SCHEDULE_SRR_)
	free(_tasks);
#endif
}
