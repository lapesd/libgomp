/*
* Copyright(C) 2011-2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
* 
* master.c - Master process.
*/

#include <assert.h>
#include <global.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <timer.h>
#include <util.h>
#include <ipc.h>
#include "master.h"

/*
 * Wrapper to data_send(). 
 */
#define data_send(a, b, c)                   \
	{                                        \
		data_sent += c;                      \
		nsend++;                             \
		communication += data_send(a, b, c); \
	}                                        \

/*
 * Wrapper to data_receive(). 
 */
#define data_receive(a, b, c)                   \
	{                                           \
		data_received += c;                     \
		nreceive++;                             \
		communication += data_receive(a, b, c); \
	}                                           \


#define NUM_THREADS (NUM_CORES/NUM_CLUSTERS)

/* Size of arrays. */
#define CENTROIDS_SIZE ((ncentroids*dimension)+nclusters*(nclusters-1)*dimension)
#define POPULATION_SIZE (ncentroids + nclusters*(nclusters-1))
#define PCENTROIDS_SIZE (nclusters*ncentroids*dimension)
#define PPOPULATION_SIZE (nclusters*ncentroids)

/* K-means. */
static float mindistance; /* Minimum distance.          */
static int ncentroids;    /* Number of centroids.       */
static int npoints;       /* Number of data points.     */
static int dimension;     /* Dimension of data points.  */
static int *map;          /* Map of clusters.           */
static vector_t *data;    /* Data points.               */
static float *centroids;  /* Data centroids.            */
static int *population;   /* Population of centroids.   */
static float *pcentroids; /* Partial centroids.         */
static int *ppopulation;  /* Partial population.        */
static int *has_changed;  /* Has any centroid changed?  */
static int *too_far;      /* Are points too far?        */
static int *lnpoints;     /* Local number of points.    */
static int *lncentroids;  /* Local number of centroids. */


/*
 * Returns the ith centroid.
 */
#define CENTROID(i) \
	(&centroids[(i)*dimension])

/*
 * Sends work to slave processes.
 */
static void sendwork(void)
{
	int i, j;  /* Loop index.            */
	ssize_t n; /* Bytes to send/receive. */

	/* Distribute work among slave processes. */
	for (i = 0; i < nclusters; i++)
	{
		lnpoints[i] = ((i + 1) < nclusters) ? 
			npoints/nclusters :  npoints - i*(npoints/nclusters);
			
		lncentroids[i] = ((i + 1) < nclusters) ? 
			ncentroids/nclusters : ncentroids - i*(ncentroids/nclusters);
	}

	/* Send work to slave processes. */
	for (i = 0; i < nclusters; i++)
	{
		data_send(outfd[i], &lnpoints[i], sizeof(int));
		
		data_send(outfd[i], &nclusters, sizeof(int));
		
		data_send(outfd[i], &ncentroids, sizeof(int));
		
		data_send(outfd[i], &mindistance, sizeof(float));

		data_send(outfd[i], &dimension, sizeof(int));
		
		n = nclusters*sizeof(int);
		data_send(outfd[i], lncentroids, n);
		
		n = dimension*sizeof(float);
		for (j = 0; j < lnpoints[i]; j++)
		{
			
				data_send(outfd[i], data[i*(npoints/nclusters)+j]->elements, n);
		}
		
		n = ncentroids*dimension*sizeof(float);
		data_send(outfd[i], centroids, n);
		
		n = lnpoints[i]*sizeof(int);
		data_send(outfd[i], &map[i*(npoints/nclusters)], n);
	}
}

#define PCENTROID(i, j) \
(&pcentroids[((i)*ncentroids + (j))*dimension])

#define PPOPULATION(i, j) \
(&ppopulation[(i)*ncentroids + (j)])

/*
 * Synchronizes partial centroids.
 */
static void sync_pcentroids(void)
{
	int i, j;            /* Loop indexes.          */
	ssize_t n;           /* Bytes to send/receive. */
	uint64_t start, end; /* Timer.                 */
	
	/* Receive partial centroids. */
	n = ncentroids*dimension*sizeof(float);
	for (i = 0; i < nclusters; i++)
		data_receive(infd[i], PCENTROID(i, 0), n);

	/* 
	 * Send partial centroids to the
	 * slave process that is assigned to it.
	 */
	for (i = 0; i < nclusters; i++)
	{
		/* Build partial centroid. */
		start = timer_get();
		n = lncentroids[i]*dimension*sizeof(float);
		for (j = 0; j < nclusters; j++)
		{
			memcpy(CENTROID(j*lncentroids[i]), 
									PCENTROID(j, i*(ncentroids/nclusters)), n);
		}
		end = timer_get();
		master += timer_diff(start, end);

		n = nclusters*lncentroids[i]*dimension*sizeof(float);
		data_send(outfd[i], centroids, n);
	}
}

/*
 * Synchronizes partial population.
 */
static void sync_ppopulation(void)
{
	int i, j;            /* Loop indexes.          */
	ssize_t n;           /* Bytes to send/receive. */
	uint64_t start, end; /* Timer.                 */

	/* Receive temporary population. */
	n = ncentroids*sizeof(int);
	for (i = 0; i < nclusters; i++)
		data_receive(infd[i], PPOPULATION(i, 0), n);

	/* 
	 * Send partial population to the
	 * slave process that assigned to it.
	 */
	for (i = 0; i < nclusters; i++)
	{
		/* Build partial population. */
		start = timer_get();
		n = lncentroids[i]*sizeof(int);
		for (j = 0; j < nclusters; j++)
		{
			memcpy(&population[j*lncentroids[i]], 
								  PPOPULATION(j, i*(ncentroids/nclusters)), n);
		}
		end = timer_get();
		master += timer_diff(start, end);

		n = nclusters*lncentroids[i]*sizeof(int);
		data_send(outfd[i], population, n);
	}
}

/*
* Synchronizes centroids.
*/
static void sync_centroids(void)
{
	int i;     /* Loop index.            */
	ssize_t n; /* Bytes to send/receive. */

	/* Receive centroids. */
	for (i = 0; i < nclusters; i++)
	{
		n = lncentroids[i]*dimension*sizeof(float);
		
					data_receive(infd[i], CENTROID(i*(ncentroids/nclusters)), n);
	}

	/* Broadcast centroids. */
	n = ncentroids*dimension*sizeof(float);
	for (i = 0; i < nclusters; i++)
		data_send(outfd[i], centroids, n);
}

/*
* Synchronizes slaves' status.
*/
static void sync_status(void)
{
	int i;     /* Loop index.            */
	ssize_t n; /* Bytes to send/receive. */

	/* Receive data. */
	n = NUM_THREADS*sizeof(int);
	for (i = 0; i < nclusters; i++)
	{
		data_receive(infd[i], &has_changed[i*NUM_THREADS], n);
		data_receive(infd[i], &too_far[i*NUM_THREADS], n);
	}

	/* Broadcast data to slaves. */
	n = nclusters*NUM_THREADS*sizeof(int);
	for (i = 0; i < nclusters; i++)
	{
		data_send(outfd[i], has_changed, n);
		data_send(outfd[i], too_far, n);
	}
}

/*
 * Asserts if another iteration is needed.
 */
static int again(void)
{
	int i;               /* Loop index. */
	uint64_t start, end; /* Timer.      */
	
	start = timer_get();
	for (i = 0; i < nclusters*NUM_THREADS; i++)
	{
		if (has_changed[i] && too_far[i])
		{
			end = timer_get();
			master += timer_diff(start, end);
			return (1);
		}
	}
	end = timer_get();
	master += timer_diff(start, end);
	
	return (0);
}

/*
 * Internal kmeans().
 */
static void _kmeans(void)
{
	int i, j;            /* Loop indexes. */
	uint64_t start, end; /* Timer.        */
	
	/* Create auxiliary structures. */
	map = scalloc(npoints, sizeof(int));
	too_far = smalloc(nclusters*NUM_THREADS*sizeof(int));
	has_changed = smalloc(nclusters*NUM_THREADS*sizeof(int));
	centroids = smalloc(CENTROIDS_SIZE*sizeof(float));
	population = smalloc(POPULATION_SIZE*sizeof(int));
	pcentroids = smalloc(PCENTROIDS_SIZE*sizeof(float));
	ppopulation = smalloc(PPOPULATION_SIZE*sizeof(int));
	lnpoints = smalloc(nclusters*sizeof(int));
	lncentroids = smalloc(nclusters*sizeof(int));
	
	start = timer_get();
	/* Initialize mapping. */
	for (i = 0; i < npoints; i++)
		map[i] = -1;

	/* Initialize centroids. */
	for (i = 0; i < ncentroids; i++)
	{
		j = randnum()%npoints;
		memcpy(CENTROID(i), data[j]->elements, dimension*sizeof(float));
		map[j] = i;
	}
	
	/* Map unmapped data points. */
	for (i = 0; i < npoints; i++)
	{
		if (map[i] < 0)
			map[i] = randnum()%ncentroids;
	}
	end = timer_get();
	master += timer_diff(start, end);
	
	sendwork();
	
	do
	{
		sync_pcentroids();
		sync_ppopulation();
		sync_centroids();
		sync_status();

	} while (again());
	
	/* House keeping. */
	free(lncentroids);
	free(lnpoints);
	free(ppopulation);
	free(pcentroids);
	free(population);
	free(centroids);
	free(has_changed);
	free(too_far);
}

/*
 * Clusters data. 
 */
int *kmeans(vector_t *_data, int _npoints, int _ncentroids, float _mindistance)
{
	/* Setup parameters. */
	data = _data;
	npoints = _npoints;
	ncentroids = _ncentroids;
	mindistance = _mindistance;
	dimension = vector_size(data[0]);
	
	open_noc_connectors();
	spawn_slaves();

	_kmeans();

	join_slaves();
	close_noc_connectors();
	
	return (map);
}
