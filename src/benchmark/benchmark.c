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

#include <omp.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <profile.h>
#include <util.h>

#if defined(_SCHEDULE_SRR_)
extern void omp_set_workload(unsigned *, unsigned);
#endif

/**
 * @brief Benchmark kernel.
 */
void kernel(int n, int load)
{
	int sum = 0;
	
	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < load; j++)
			sum++;		
	}
}

/**
 * @brief Synthetic benchmark. 
 */
void benchmark(
	const unsigned *tasks,
	unsigned ntasks,
	unsigned nthreads,
	unsigned load)
{
	unsigned loads[nthreads];
	
#if defined(_SCHEDULE_SRR_)
	unsigned *_tasks;
	_tasks = smalloc(ntasks*sizeof(unsigned));
	memcpy(_tasks, tasks, ntasks*sizeof(unsigned));
#endif

	memset(loads, 0, nthreads*sizeof(unsigned));
	
	profile_start();

	/* Sort Each bucket. */
#if defined(_SCHEDULE_STATIC_)
	#pragma omp parallel for schedule(static)
#elif defined(_SCHEDULE_GUIDED_)
	#pragma omp parallel for schedule(guided)
#elif defined(_SCHEDULE_DYNAMIC_)
	#pragma omp parallel for schedule(dynamic)
#elif defined(_SCHEDULE_SRR_)
	omp_set_workload(_tasks, ntasks);
	#pragma omp parallel for schedule(runtime)
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
	for (unsigned i = 0; i < nthreads; i++)
		printf("thread %d: %u\n", i, loads[i]);
		
	/* House  keeping. */
#if defined(_SCHEDULE_SRR_)
	free(_tasks);
#endif
}
