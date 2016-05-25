#ifndef __COMMON_MAIN_H
#define __COMMON_MAIN_H

#include <timer.h>
#include "exec.h"

typedef union { 
	void *void_t;
	int int_t;
} barrier_par_t;

struct execution_parameters {
	int cluster;
	int nb_clusters;
	int nb_partitions;
	int nb_threads;
	int nb_towns;
	int seed;
	barrier_par_t barrier;
	tsp_t_pointer *tsp;
};

void *spawn_worker (void* params);

extern pthread_t *spawn (tsp_t_pointer *tsp, int cluster_id, int nb_clusters, int nb_partitions, int nb_threads, int nb_towns, int seed);
extern void run_tsp (int nb_threads, int nb_towns, int seed, int nb_clusters);
extern void wait_barrier (barrier_par_t barrier_par);

#endif //__COMMON_MAIN_H
