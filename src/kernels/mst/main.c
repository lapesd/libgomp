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
#include <omp.h>
#include <string.h>
#include <papi.h>
#include <stdio.h>

#include "util.h"
#include "mst.h"

static int trace = 0;

unsigned __ntasks;
unsigned *__tasks;

#define NREGIONS 32
unsigned *readfile(const char *filename, int ntasks)
{
	FILE *infile;
	unsigned *data;
	
	infile = fopen(filename, "r");
	assert(infile != NULL);
	
	data = smalloc(ntasks*sizeof(unsigned));
	
	for (int i = 0; i < ntasks; i++)
		assert (fscanf(infile, "%u", &data[i]) == 1);
		
	/* House keeping. */
	fclose(infile);
		
	return (data);
}

#if defined(_SCHEDULE_SRR_)
extern void omp_set_workload(unsigned *, unsigned);
#endif


int main(int argc, char **argv)
{
	int n;
	double start, end;
	struct point *data[NREGIONS];
#ifdef _PROFILE_
	int events[4] = { PAPI_L1_DCM, PAPI_L2_DCM, PAPI_L2_DCA, PAPI_L3_DCA };
	long long hwcounters[4];
#endif
	unsigned *densities;
#ifdef _SCHEDULE_ORACLE_
	unsigned *taskmap = readfile(argv[3], NREGIONS);
#endif

	densities = readfile(argv[2], NREGIONS);

	__tasks=smalloc(NREGIONS*sizeof(unsigned));

	((void) argc);
	
	n = atoi(argv[1]);
	
	for (int k = 0; k < NREGIONS; k++)
	{
		data[k] = smalloc(densities[k]*sizeof(struct point));
		
		/* Initialize data set. */
		for (unsigned i = 0; i < densities[k]; i++)
		{
			data[k][i].x = rand()%10000;
			data[k][i].y = rand()%10000;
		}
	}
#ifdef _PROFILE_
	if (PAPI_start_counters(events, 4) != PAPI_OK)
		error("failed to setup PAPI");
#endif
	
	start = omp_get_wtime();
#if defined(_SCHEDULE_STATIC_)
	#pragma omp parallel for schedule(static) num_threads(n) default(shared)
#elif defined(_SCHEDULE_DYNAMIC_)
	#pragma omp parallel for schedule(dynamic) num_threads(n) default(shared)
#elif defined(_SCHEDULE_SRR_)
	memcpy(__tasks, densities, NREGIONS*sizeof(unsigned));
	__ntasks = NREGIONS;
	omp_set_workload(__tasks, __ntasks);
	#pragma omp parallel for schedule(runtime) num_threads(n) default(shared)
#elif defined(_SCHEDULE_ORACLE_)
	memcpy(__tasks, taskmap, NREGIONS*sizeof(unsigned));
	__ntasks = NREGIONS;
	#pragma omp parallel for schedule(runtime) num_threads(n) default(shared)
#endif
	for (int i = 0; i < NREGIONS; i++)
	{
		if (trace)
		{
			#pragma omp critical
			printf("thread %d timestamp %lf\n", omp_get_thread_num(), omp_get_wtime());
		}
		mst(data[i], densities[i]);
		if (trace)
		{
			#pragma omp critical
			printf("thread %d timestamp %lf\n", omp_get_thread_num(), omp_get_wtime());
		}
	}
	end = omp_get_wtime();

#ifdef _PROFILE_
	if (PAPI_stop_counters(hwcounters, sizeof(events)) != PAPI_OK)
		error("failed to read hardware counters");
#endif	
	printf("time: %lf\n", end - start);
#ifdef _PROFILE_
	printf("L1 Misses: %lld\n", hwcounters[0]);
	printf("L2 Misses: %lld\n", hwcounters[1]);
	printf("L2 Accesses: %lld\n", hwcounters[2]);
	printf("L3 Accesses: %lld\n", hwcounters[3]);
#endif	
	/* House keeping. */
	free(densities);
	for (int i = 0; i < NREGIONS; i++)
		free(data[i]);
	free(__tasks);
#ifdef _SCHEDULE_ORACLE_
	free(taskmap);
#endif
	
	return (EXIT_SUCCESS);
}
