#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <sched.h>
#include <errno.h>
#include <inttypes.h>

#include "common_main.h"

static tsp_t_pointer *tsps;

static int min_distance;
static int next_partition;
MUTEX_CREATE(main_lock, static);

//Initialization synchronization
COND_VAR_CREATE(sync_barrier, static);
static int running_count;

/*
 * Problem.
 */
struct problem {
	int nb_towns;		/* Number of towns */
};

/* Problem sizes. */
static struct problem tiny     =  { 14 };
static struct problem small    =  { 15 };
static struct problem standard =  { 17 };
static struct problem large    =  { 19 };
static struct problem huge     =  { 20 };

/* Be verbose? */
int verbose = 0;

int nthreads = 1;	/* Number of threads */
int seed = 12345;	/* Seed for random numbers generator */

/* Problem. */           
static struct problem *p = &tiny;

/*
 * Prints program usage and exits.
 */
static void usage(void)
{
	printf("Usage: tsp [options]\n");
	printf("Brief: Traveler Salesman-problem.\n");
	printf("Options:\n");
	printf("  --help             Display this information and exit\n");
	printf("  --nthreads <value> Set number of threads\n");
	printf("  --class <name>     Set problem class:\n");
	printf("                       - tiny\n");
	printf("                       - small\n");
	printf("                       - standard\n");
	printf("                       - large\n");
	printf("                       - huge\n");
	printf("  --verbose          Be verbose\n");
	exit(0);
}

/*
 * Reads command line arguments.
 */
static void readargs(int argc, char **argv)
{
	int i;     /* Loop index.       */
	char *arg; /* Working argument. */
	int state; /* Processing state. */
	
	/* State values. */
	#define READ_ARG     0 /* Read argument.         */
	#define SET_NTHREADS 1 /* Set number of threads. */
	#define SET_CLASS    2 /* Set problem class.     */
	
	state = READ_ARG;
	
	/* Read command line arguments. */
	for (i = 1; i < argc; i++)
	{
		arg = argv[i];
		
		/* Set value. */
		if (state != READ_ARG)
		{
			switch (state)
			{
				/* Set problem class. */
				case SET_CLASS :
					if (!strcmp(argv[i], "tiny"))
						p = &tiny;
					else if (!strcmp(argv[i], "small"))
						p = &small;
					else if (!strcmp(argv[i], "standard"))
						p = &standard;
					else if (!strcmp(argv[i], "large"))
						p = &large;
					else if (!strcmp(argv[i], "huge"))
						p = &huge;
					else 
						usage();
					state = READ_ARG;
					break;
				
				/* Set number of threads. */
				case SET_NTHREADS :
					nthreads = atoi(arg);
					state = READ_ARG;
					break;
				
				default:
					usage();			
			}
			
			continue;
		}
		
		/* Parse argument. */
		if (!strcmp(arg, "--verbose"))
			verbose = 1;
		else if (!strcmp(arg, "--nthreads"))
			state = SET_NTHREADS;
		else if (!strcmp(arg, "--class"))
			state = SET_CLASS;
		else
			usage();
	}
	
	/* Invalid argument(s). */
	if (nthreads < 1)
		usage();
}

/*
 * Runs benchmark.
 */
int main (int argc, char **argv) {

	readargs(argc, argv);
	
	COND_VAR_INIT(sync_barrier);
	MUTEX_INIT(main_lock);
	
	run_tsp(nthreads, p->nb_towns, seed, 1);

	return 0;
}

void new_minimun_distance_found(tsp_t_pointer tsp) {
	int i;
	int min = tsp_get_shortest_path(tsp);
	for (i = 0; i < tsp->nb_clusters; i++) {
		if (tsps[i] != tsp)
			tsp_update_minimum_distance (tsps[i], min);
	}
	if (min_distance > min) {
		MUTEX_LOCK(main_lock);
		if (min_distance > min)
			min_distance = min;
		MUTEX_UNLOCK(main_lock);
	}
}

void wait_barrier (barrier_par_t barrier_par) {
	int limit = barrier_par.int_t;
	COND_VAR_MUTEX_LOCK(sync_barrier);
	running_count++;
	if (running_count == limit) {
		running_count = 0;
		COND_VAR_BROADCAST(sync_barrier);
	} else  {
		COND_VAR_WAIT(sync_barrier);
	}
	COND_VAR_MUTEX_UNLOCK(sync_barrier);
} 


pthread_t *spawn (tsp_t_pointer *tsp, int cluster_id, int nb_clusters, int nb_partitions, int nb_threads, int nb_towns, int seed) {

	pthread_t *tid = (pthread_t *)malloc (sizeof(pthread_t));
	struct execution_parameters *params = (struct execution_parameters*) malloc (sizeof(struct execution_parameters));
	params->cluster = cluster_id;
	params->nb_clusters = nb_clusters;
	params->nb_partitions = nb_partitions;
	params->nb_threads = nb_threads;
	params->nb_towns = nb_towns;
	params->seed = seed;
	params->tsp = tsp;
	params->barrier.int_t = nb_clusters;

	int status = pthread_create (tid, NULL, spawn_worker, params);
	assert (status == 0);

	return tid;	
}

void run_tsp (int nb_threads, int nb_towns, int seed, int nb_clusters) {
	int i;

	uint64_t end;   /* End time.     */
	uint64_t start; /* Start time.   */
	
#ifdef _XEON_PHI_
	double power;
#endif

	timer_init();
	
#ifdef _XEON_PHI_
	power_init();
#endif
	
	int nb_partitions = get_number_of_partitions(nb_clusters);
	if (verbose)
		printf ("Number of clusters..: %3d\nNumber of partitions: %3d\nNumber of threads...: %3d\nNumber of Towns.....: %3d\nSeed................: %3d\n", 
			nb_clusters, nb_partitions, nb_threads, nb_towns, seed);

	min_distance = INT_MAX;
	next_partition = 0;
	running_count = 0;
	tsps = 	(tsp_t_pointer *)(malloc(sizeof(tsp_t_pointer) * nb_clusters));
	assert(tsps != NULL);
	pthread_t **tids = (pthread_t **) malloc (sizeof(pthread_t *) * nb_clusters);
	assert (tids != NULL);	
	
	start = timer_get();
	for (i = 0; i < nb_clusters; i++)
		tids[i] = spawn(&tsps[i], i, nb_clusters, nb_partitions, nb_threads, nb_towns, seed);
	for (i = 0; i < nb_clusters; i++) {
		pthread_join (*(tids[i]), NULL);
		free(tids[i]);
	}

	free (tids);
	free(tsps);

	end = timer_get();
	
#ifdef _XEON_PHI_
	power = power_end();
#endif

	printf("shortest path size = %5d towns\n", min_distance);
	
	printf("timing statistics:\n");
	printf("  total time:    %f\n", timer_diff(start, end)*MICROSEC);

#ifdef _XEON_PHI_
	printf("  average power: %f\n", power*0.000001);
#endif
}

partition_interval_t get_next_partition(tsp_t_pointer tsp) {
	partition_interval_t ret;
	MUTEX_LOCK(main_lock);
	ret = get_next_partition_default_impl(tsp->nb_partitions, tsp->nb_clusters, &next_partition, tsp->processed_partitions);
	MUTEX_UNLOCK(main_lock);
	return ret;
}
