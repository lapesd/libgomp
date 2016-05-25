/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * kmeans.c - kmeans() implementation.
 */

#include <global.h>
#include <math.h>
#include <omp.h>
#include <string.h>
#include <util.h>
#include "km.h"

/* Kmeans data. */
float mindistance;   /* Minimum distance.             */
int npoints;         /* Number of points.             */
vector_t *data;      /* Data being clustered.         */
int ncentroids;      /* Number of clusters.           */
int *map;            /* Map of clusters.              */
vector_t *centroids; /* Data centroids.               */
vector_t *tmp;       /* Temporary centroids.          */
int *dirty;          /* Dirty centroid?               */
int *too_far;        /* Are there any points too far? */
int *has_changed;    /* has any centroid change?      */

/*
 * Populates clusters.
 */
static void populate(void)
{
	int tid;        /* Thread ID.          */
	int i, j;       /* Loop indexes.       */
	float tmp;      /* Auxiliary variable. */
	float distance; /* Distance.           */
	
	memset(too_far, 0, nthreads*sizeof(int));
	
	/* Iterate over data points. */
	#pragma omp parallel private(i, j, tmp, distance, tid) default(shared)
	{
		tid = omp_get_thread_num();
		
		#pragma omp for
		for (i = 0; i < npoints; i++)
		{	
			distance = vector_distance(centroids[map[i]], data[i]);
			
			/* Look for closest cluster. */
			for (j = 0; j < ncentroids; j++)
			{	
				/* Point is in this cluster. */
				if (j == map[i])
					continue;
					
				tmp = vector_distance(centroids[j], data[i]);
				
				/* Found. */
				if (tmp < distance)
				{
					map[i] = j;
					distance = tmp;
					
					#pragma omp critical
					dirty[j] = 1;
				}
			}
			
			/* Cluster is too far away. */
			if (distance > mindistance)
				too_far[tid] = 1;
		}
	}
}

/*
 * Computes cluster's centroids.
 */
static void compute_centroids(void)
{
	int tid;        /* Thread ID.          */
	int i, j;       /* Loop indexes.       */
	int population; /* Cluster population. */
	
	memset(has_changed, 0, nthreads*sizeof(int));
	
	/* Compute means. */
	#pragma omp parallel private(i, j, population, tid) default(shared) 
	{	
		tid = omp_get_thread_num();
		
		#pragma omp for
		for (i = 0; i < ncentroids; i++)
		{
			/* Cluster did not change. */
			if (!dirty[i])
				continue;
				
			/* Initialize temporary vector.*/
			vector_assign(tmp[tid], centroids[i]);
			vector_clear(centroids[i]);
			
			/* Compute cluster's mean. */
			population = 0;
			for (j = 0; j < npoints; j++)
			{
				/* Not a member of this cluster. */
				if (map[j] != i)
					continue;			
				
				vector_add(centroids[i], data[j]);
					
				population++;
			}		
			if (population > 1)
				vector_mult(centroids[i], 1.0/population);
			
			has_changed[tid] = 1;
		}
	}
	
	memset(dirty, 0, ncentroids*sizeof(int));
}

/*
 * Clusters data. 
 */
int *kmeans(vector_t *_data, int _npoints, int _ncentroids, float _mindistance)
{
	int i, j;  /* Loop index. */
	int again; /* Loop again? */
	
	/* Setup parameters. */
	data = _data;
	npoints = _npoints;
	ncentroids = _ncentroids;
	mindistance = _mindistance;
	
	/* Create auxiliary structures. */
	map  = scalloc(npoints, sizeof(int));
	too_far = smalloc(nthreads*sizeof(int));
	has_changed = smalloc(nthreads*sizeof(int));
	dirty = smalloc(ncentroids*sizeof(int));
	centroids = smalloc(ncentroids*sizeof(vector_t));
	for (i = 0; i < ncentroids; i++)
		centroids[i] = vector_create(vector_size(data[0]));
	tmp = smalloc(nthreads*sizeof(vector_t));
	for (i = 0; i < nthreads; i++)
		tmp[i] = vector_create(vector_size(data[0]));
	
	/* Initialize mapping. */
	for (i = 0; i < npoints; i++)
		map[i] = -1;

	/* Initialize centroids. */
	for (i = 0; i < ncentroids; i++)
	{
		dirty[i] = 1;
		j = randnum()%npoints;
		vector_assign(centroids[i], data[j]);
		map[j] = i;
	}

	/* Map unmapped data points. */
	for (i = 0; i < npoints; i++)
	{
		if (map[i] < 0)
			map[i] = randnum()%ncentroids;
	}
	
	/* Cluster data. */
	do
	{
		populate();
		compute_centroids();

		/* Check if we need to loop. */
		for (i = 0; i < nthreads; i++)
		{
			/* We will need another iteration. */
			if (too_far[i] && has_changed[i])
				break;		
		}

		again = (i < nthreads) ? 1 : 0;

	} while (again);
	
	/* House keeping.  */
	for (i = 0; i < ncentroids; i++)
		vector_destroy(centroids[i]);
	free(centroids);
	for (i = 0; i < nthreads; i++)
		vector_destroy(tmp[i]);
	free(tmp);
	free(too_far);
	free(has_changed);
	free(dirty);
	
	return (map);
}
