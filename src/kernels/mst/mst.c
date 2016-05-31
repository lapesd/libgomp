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

#include "util.h"
#include "mst.h"
#include "pqueue.h"


inline static int distance(struct point p0, struct point p1)
{
	return ((p0.x - p1.x)*(p0.x - p1.x) + (p0.y - p1.y)*(p0.y - p1.y));
}

void mst(struct point *data, int n)
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

void mst(struct point *data)
{
	
	
	
#if defined(_SCHEDULE_STATIC_)
	#pragma omp parallel for schedule(static)
#elif defined(_SCHEDULE_GUIDED_)
	#pragma omp parallel for schedule(guided)
#elif defined(_SCHEDULE_DYNAMIC_)
	#pragma omp parallel for schedule(dynamic)
#elif defined(_SCHEDULE_SRR_)
	memcpy(__tasks, densities, NREGIONS*sizeof(unsigned));
	__ntasks = NREGIONS;
	omp_set_workload(__tasks, __ntasks);
	#pragma omp parallel for schedule(runtime) num_threads(n) default(shared)
#endif
	for(int i = 0; i < NREGIONS; i++)
		mst(data[i], densities[i]);
}
