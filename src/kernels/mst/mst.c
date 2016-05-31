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

#include "util.h"
#include "mst.h"
#include "pqueue.h"

/**
 * @brief Number of regions.
 * 
 * @details Numberof regions. Adjust this to improve cache locality.
 */
#define NR_REGIONS 32

inline static int distance(struct point p0, struct point p1)
{
	return ((p0.x - p1.x)*(p0.x - p1.x) + (p0.y - p1.y)*(p0.y - p1.y));
}

/**
 * @brief MST kernel.
 * 
 * @param data Data points.
 * @param n    Number of points.
 */
static void mst(struct point *data, int n)
{
	int *from;
	int *cost;
	struct pqueue *frontier;
	
	from = smalloc(n*sizeof(int));
	cost = smalloc(n*sizeof(int));
		
	frontier = pqueue_create(n);
	
	/* Initialize dataset. */
	for (int i = 0; i < n; i++)
	{
		from[i] = -1;
		cost[i] = INT_MAX;
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
	return ((((struct point *)p1)->x > ((struct point *)p2)->y) ? 1 : -1);
}

/**
 * @brief MST clustering kernel.
 * 
 * @param points  Data points to cluster.
 * @param npoints Number of points.
 */
void mst_clustering(struct point *points, int npoints)
{
	double range;                  /* Region range.                    */
	double xmin, xmax;             /* Min. and max for x.              */
	int densities[NR_REGIONS + 1]; /* Number of points in each region. */
	
	/* Sort points according to x coordinate. */
	qsort(points, npoints, sizeof(struct point), point_cmp);
	
	/* Get maximum and minimum. */
	xmin = points[0].x;
	xmax = points[npoints - 1].x;
	
	/* Compute densities. */
	range = (xmax - xmin)/NR_REGIONS;
	memset(densities, 0, NR_REGIONS*sizeof(int));
	for (int i = 0; i < npoints; i++)
		densities[((int)ceil(points[i].x/range)) + 1]++;
	
#if defined(_SCHEDULE_STATIC_)
	#pragma omp parallel for schedule(static)
#elif defined(_SCHEDULE_GUIDED_)
	#pragma omp parallel for schedule(guided)
#elif defined(_SCHEDULE_DYNAMIC_)
	#pragma omp parallel for schedule(dynamic)
#endif
	for(int i = 1; i <= NR_REGIONS; i++)
		mst(&points[densities[i - 1]], densities[i]);
}
