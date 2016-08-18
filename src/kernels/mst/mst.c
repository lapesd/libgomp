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

#include <limits.h>
#include <string.h>
#include <math.h>
#include <omp.h>
#include <float.h>
#include <stdio.h>
#include <util.h>
#include <profile.h>
#include <mst.h>
#include <pqueue.h>

#if defined(_SCHEDULE_SRR_)
extern void omp_set_workload(unsigned *, unsigned);
#endif

/**
 * @brief Number of regions.
 * 
 * @details Number of regions. Adjust this to improve data locality.
 */
#define NR_REGIONS 48

inline static double distance(struct point p0, struct point p1)
{
	return (pow((p0.x - p1.x), 2) + pow((p0.y - p1.y), 2));
}

/**
 * @brief MST kernel.
 * 
 * @param data Data points.
 * @param n    Number of points.
 */
void mst(const struct point *data, int n)
{
	int *from;
	double *cost;
	struct pqueue *frontier;
	
	from = smalloc(n*sizeof(int));
	cost = smalloc(n*sizeof(double));
		
	frontier = pqueue_create(n);
	
	/* Initialize dataset. */
	for (int i = 0; i < n; i++)
	{
		from[i] = -1;
		cost[i] = DBL_MAX;
	}
	
	/* Start from point 0. */
	cost[0] = 0;
	for (int i = 1; i < n; i++)
	{
		from[i] = 0;
		cost[i] = distance(data[from[i]], data[i]);
		pqueue_insert(frontier, i, cost[i]);
	}
	
	/* Build minimum spanning tree. */
	while (!pqueue_empty(frontier))
	{
		int curr;
		
		curr = pqueue_remove(frontier);
		
		/* insert in MST */
		
		for (int i = 0; i < n; i++)
		{
			double tmp;
			
			if (i == curr)
				continue;
			
			if ((tmp = distance(data[curr], data[i])) < cost[curr])
			{
				cost[i] = tmp;
				from[i] = curr;
				pqueue_change(frontier, i, cost[i]);
			}
		}
	}
	
	/* House keeping. */
	free(cost);
	free(from);
	pqueue_destroy(frontier);
}

/**
 * @brief Compares two points.
 * 
 * @param p1 Point 1.
 * @param p2 Point 2.
 * 
 * @returns One if point 1 is greater than point 2, and an integer less than
 *          zero otherwise.
 */
static int point_cmp(const void *p1, const void *p2)
{
	return ((((struct point *)p1)->x > ((struct point *)p2)->x) ? 1 : -1);
}

/**
 * @brief MST clustering kernel.
 * 
 * @param points  Data points to cluster.
 * @param npoints Number of points.
 */
void mst_clustering(struct point *points, int npoints)
{
	double range;                   /* Region range.                    */
	double xmin, xmax;              /* Min. and max for x.              */
	unsigned densities[NR_REGIONS + 1]; /* Number of points in each region. */
	double time[24];
	unsigned workload[24];

	memset(time, 0, 24*sizeof(double));
	memset(workload, 0, 24*sizeof(unsigned));
	
	/* Sort points according to x coordinate. */
	qsort(points, npoints, sizeof(struct point), point_cmp);
	
	/* Get maximum and minimum. */
	xmin = points[0].x;
	xmax = points[npoints - 1].x;

	/* Compute densities. */
	range = fabs((xmax - xmin + 1)/NR_REGIONS);
	for(int i = 0; i <= NR_REGIONS; i++)
		densities[i] = 0;
	for (int i = 0; i < npoints; i++)
	{
		int j = (int)floor((points[i].x - xmin)/range) + 1;
		
		/* Fix rounding error. */
		if (j > NR_REGIONS)
			j = NR_REGIONS;
		
		densities[j]++;
	}

	profile_start();

#if defined(_SCHEDULE_STATIC_)
	#pragma omp parallel for schedule(static)
#elif defined(_SCHEDULE_GUIDED_)
	#pragma omp parallel for schedule(guided)
#elif defined(_SCHEDULE_DYNAMIC_)
	#pragma omp parallel for schedule(dynamic)
#elif defined(_SCHEDULE_SRR_)
	int _densities[NR_REGIONS];
	memcpy(_densities, &densities[1], NR_REGIONS*sizeof(int));
	omp_set_workload((unsigned *)&_densities[1], NR_REGIONS);
	#pragma omp parallel for schedule(runtime)
#endif
	for(int i = 1; i <= NR_REGIONS; i++)
	{
		double start, end;

		start = omp_get_wtime();

		mst(&points[densities[i - 1]], (int) densities[i]);

		end = omp_get_wtime();

		time[omp_get_thread_num()] += end - start;
		workload[omp_get_thread_num()] += densities[i];
	}
		
	profile_end();

	for (int i = 0; i < 24; i++)
		printf("Thread %d: %lf %u\n", i, time[i], workload[i]);
	
	profile_dump();
}
