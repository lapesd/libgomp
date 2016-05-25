/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * slave.c - k-means slave process.
 */

#include <arch.h>
#include <assert.h>
#include <omp.h>
#include <stdint.h>
#include <string.h>
#include <timer.h>
#include <util.h>
#include <ipc.h>
#include "slave.h"

#define NUM_THREADS (NUM_CORES/NUM_CLUSTERS)

#define DIMENSION          16
#define NUM_POINTS     262144
#define NUM_CENTROIDS    1024

/* Size of arrays. */
#define MAP_SIZE (NUM_POINTS/NUM_CLUSTERS)                /* map[]         */
#define POINTS_SIZE ((NUM_POINTS/NUM_CLUSTERS)*DIMENSION) /* points[]      */
#define CENTROIDS_SIZE (NUM_CENTROIDS*DIMENSION)          /* centroids[]   */
#define PPOPULATION_SIZE (NUM_CENTROIDS)                  /* ppopulation[] */
#define LCENTROIDS_SIZE ((NUM_CENTROIDS/1)*DIMENSION)     /* lcentroids[]  */

/* Delta of array sizes. */
#define DELTA (NUM_CLUSTERS - 1)

/*
 * Returns the ith data point.
 */
#define POINT(i) \
	(&points[(i)*dimension])

/*
 * Returns the ith centroid.
 */
#define CENTROID(i) \
	(&centroids[(i)*dimension])

/* K-means. */
int dimension;
static int nprocs;
static float mindistance;
static int ncentroids;
static float points[POINTS_SIZE + DELTA*DIMENSION];                    /* Data points.               */
static float centroids[CENTROIDS_SIZE + NUM_CLUSTERS*DELTA*DIMENSION]; /* Data centroids.            */
static int map[MAP_SIZE + DELTA];                                      /* Map of clusters.           */
static int too_far[NUM_CLUSTERS*NUM_THREADS];                          /* Are points too far?        */
static int has_changed[NUM_CLUSTERS*NUM_THREADS];                      /* Hash any centroid changed? */
static int lncentroids[NUM_CLUSTERS];                                  /* Local number of centroids. */
static int lnpoints;                                                   /* Local number of points.    */
static int ppopulation[PPOPULATION_SIZE + NUM_CLUSTERS*DELTA];         /* Partial population.        */
static float lcentroids[LCENTROIDS_SIZE + DELTA*DIMENSION];            /* Local centroids.           */

/* Thread communication. */
static omp_lock_t lock[NUM_THREADS];

/* Timing statistics. */
uint64_t start;
uint64_t end;
uint64_t communication = 0;
uint64_t total = 0;

/*============================================================================*
 *                                populate()                                 *
 *============================================================================*/

/*
 * Populates clusters.
 */
static void populate(void)
{
	int i, j;        /* Loop indexes.       */
	float tmp;      /* Auxiliary variable. */
	float distance; /* Smallest distance.  */

	start = timer_get();
	memset(&too_far[rank*NUM_THREADS], 0, NUM_THREADS*sizeof(int)); 
	
	/* Iterate over data points. */
	#pragma omp parallel for schedule(static) default(shared) private(i, j, tmp, distance)
	for (i = 0; i < lnpoints; i++)
	{
		distance = vector_distance(CENTROID(map[i]), POINT(i));
		
		/* Look for closest cluster. */
		for (j = 0; j < ncentroids; j++)
		{
			/* Point is in this cluster. */
			if (j == map[i])
				continue;
				
			tmp = vector_distance(CENTROID(j), POINT(i));
			
			/* Found. */
			if (tmp < distance)
			{
				map[i] = j;
				distance = tmp;
			}
		}
		
		/* Cluster is too far away. */
		if (distance > mindistance)
			too_far[rank*NUM_THREADS + omp_get_thread_num()] = 1;
	}
	end = timer_get();
	total += timer_diff(start, end);
	
}

/*============================================================================*
 *                          compute_centroids()                               *
 *============================================================================*/

/*
 * Returns the population for clusther ith, vector jth.
 */
#define POPULATION(i, j) \
	(&ppopulation[(i)*lncentroids[rank] + (j)])

/*
 * Returns the jth input centroid from the ith cluster.
 */
#define PCENTROID(i, j) \
	(&centroids[((i)*lncentroids[rank] + (j))*dimension])

/*
 * Returns the ith local centroid.
 */
#define LCENTROID(i) \
	(&lcentroids[(i)*dimension])

/*
 * Synchronizes partial centroids.
 */
static void sync_pcentroids(void)
{	
	ssize_t n;
	
	/* Send partial centroids. */
	n = ncentroids*dimension*sizeof(float);
	total += data_send(outfd, centroids, n);
	
	/* Receive partial centroids. */
	n = nprocs*lncentroids[rank]*dimension*sizeof(float);
	total += data_receive(infd, centroids, n);
}

/*
 * Synchronizes partial population.
 */
static void sync_ppopulation(void)
{
	ssize_t n;
	
	/* Send partial population. */
	n = ncentroids*sizeof(int);
	total += data_send(outfd, ppopulation, n);
	
	/* Receive partial population. */
	n = nprocs*lncentroids[rank]*sizeof(int);
	total += data_receive(infd, ppopulation, n);
}

/*
 * Synchronizes centroids.
 */
static void sync_centroids(void)
{
	ssize_t n;
	
	n = lncentroids[rank]*dimension*sizeof(float);
	total += data_send(outfd, lcentroids, n);
	
	n = ncentroids*dimension*sizeof(float);
	total += data_receive(infd, centroids, n);
}

/*
 * Synchronizes status.
 */
static void sync_status(void)
{
	ssize_t n;
	
	n = NUM_THREADS*sizeof(int);
	total += data_send(outfd, &has_changed[rank*NUM_THREADS], n);
	total += data_send(outfd, &too_far[rank*NUM_THREADS], n);
		
	n = nprocs*NUM_THREADS*sizeof(int);
	total += data_receive(infd, has_changed, n);
	total += data_receive(infd, too_far, n);
}

/*
 * Computes clusters' centroids.
 */
static void compute_centroids(void)
{
	int i, j;       /* Loop indexes.        */
	int population; /* Centroid population. */

	start = timer_get();
	
	memcpy(lcentroids, CENTROID(rank*(ncentroids/nprocs)), lncentroids[rank]*dimension*sizeof(float));
	memset(&has_changed[rank*NUM_THREADS], 0, NUM_THREADS*sizeof(int));
	memset(centroids, 0, (ncentroids + DELTA*nprocs)*dimension*sizeof(float));
	memset(ppopulation, 0, (ncentroids + nprocs*DELTA)*sizeof(int));

	/* Compute partial centroids. */
	#pragma omp parallel for schedule(static) default(shared) private(i, j)
	for (i = 0; i < lnpoints; i++)
	{
		j = map[i]%NUM_THREADS;
		
		omp_set_lock(&lock[j]);
		
		vector_add(CENTROID(map[i]), POINT(i));
			
		ppopulation[map[i]]++;
		
		omp_unset_lock(&lock[j]);
	}
	
	end = timer_get();
	total += timer_diff(start, end);
	
	sync_pcentroids();

	sync_ppopulation();
	
	start = timer_get();

	/* Compute centroids. */
	#pragma omp parallel for schedule(static) default(shared) private(i, j, population)
	for (j = 0; j < lncentroids[rank]; j++)
	{
		population = 0;
		
		for (i = 0; i < nprocs; i++)
		{
			if (*POPULATION(i, j) == 0)
				continue;
			
			population += *POPULATION(i, j);
			
			if (i == rank)
				continue;
			
			vector_add(PCENTROID(rank, j), PCENTROID(i, j));
		}
		
		if (population > 1)
			vector_mult(PCENTROID(rank, j), 1.0/population);
		
		/* Cluster mean has changed. */
		if (!vector_equal(PCENTROID(rank, j), LCENTROID(j)))
		{
			has_changed[rank*NUM_THREADS + omp_get_thread_num()] = 1;
			vector_assign(LCENTROID(j), PCENTROID(rank, j));
		}
	}
	
	end = timer_get();
	total += timer_diff(start, end);
		
	sync_centroids();
		
	sync_status();
}

/*============================================================================*
 *                                 again()                                    *
 *============================================================================*/

/*
 * Asserts if another iteration is needed.
 */
static int again(void)
{
	int i;
	
	start = timer_get();
	
	/* Checks if another iteration is needed. */	
	for (i = 0; i < nprocs*NUM_THREADS; i++)
	{
		if (has_changed[i] && too_far[i])
		{
			end = timer_get();
			total += timer_diff(start, end);
			return (1);
		}
	}
	
	end = timer_get();
	total += timer_diff(start, end);
			
	return (0);
}

/*============================================================================*
 *                                kmeans()                                    *
 *============================================================================*/

/*
 * Clusters data.
 */
static void kmeans(void)
{	
	int i;

	omp_set_num_threads(NUM_THREADS);
	for (i = 0; i < NUM_THREADS; i++)
		omp_init_lock(&lock[i]);
	
	/* Cluster data. */
	do
	{	
		populate();
		compute_centroids();
	} while (again());
}

/*============================================================================*
 *                                 getwork()                                  *
 *============================================================================*/

/*
 * Receives work from master process.
 */
static void getwork(void)
{	
	int i;
	
	ssize_t n;     /* Bytes to send/receive.        */
	ssize_t count; /* Bytes actually sent/received. */
	
	timer_init();
	
	n = sizeof(int);
	total += data_receive(infd, &lnpoints, n);
	
	data_receive(infd, &nprocs, sizeof(int));
	
	data_receive(infd, &ncentroids, sizeof(int));
	
	data_receive(infd, &mindistance, sizeof(int));
	
	data_receive(infd, &dimension, sizeof(int));
	
	n = nprocs*sizeof(int);
	total += data_receive(infd, lncentroids, n);
	
	n = dimension*sizeof(float);
	for (i = 0; i < lnpoints; i++)
		data_receive(infd, &points[i*dimension], n);
	
	n = ncentroids*dimension*sizeof(float);
	total += data_receive(infd, centroids, n);
	
	n = lnpoints*sizeof(int);
	total += data_receive(infd, map, n);
}

/*============================================================================*
 *                                  main()                                    *
 *============================================================================*/

/*
 * Clusters data.
 */
int main(int argc, char **argv)
{
	((void)argc);
	
	rank = atoi(argv[0]);
	
	/* Setup interprocess communication. */
	open_noc_connectors();
	
	getwork();
	
	kmeans();
	
	data_send(outfd, &total, sizeof(uint64_t));
	close_noc_connectors();
	mppa_exit(0);
	return (0);
}
