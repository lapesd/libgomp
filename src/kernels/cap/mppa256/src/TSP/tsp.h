#ifndef __TSP_H
#define __TSP_H

#include "job.h"

typedef struct {
	int nb_towns;
	struct {
		int to_city;
		int dist;
	} info[MAX_TOWNS][MAX_TOWNS];
} distance_matrix_t;

typedef struct {
	int max_hops;
	int cluster_id;
	int nb_clusters;
	int nb_threads;
	int nb_partitions;
	char PADDING1[PADDING(5 * sizeof (int))];

	distance_matrix_t *distance;
	char PADDING2[PADDING(sizeof(distance_matrix_t))];

	int min_distance;
	MUTEX_CREATE(mutex);
	int processed_partitions;
	char PADDING4[PADDING(sizeof(int) * 2 + MUTEX_SIZE)];
	job_queue_t queue;
} tsp_t;

typedef tsp_t * tsp_t_pointer;

typedef struct {
	tsp_t *tsp;
	int thread_id;
} tsp_thread_par_t;

tsp_t_pointer init_tsp(int cluster_id, int nb_clusters, int nb_partitions, int nb_threads, int nb_towns, int seed);
void free_tsp(tsp_t_pointer tsp);
int queue_size (int nb_threads, int nb_clusters, int nb_towns, int *max_hops_ret);

void *worker (void *tsp_worker_par);
inline int tsp_get_shortest_path (tsp_t_pointer tsp);
inline int tsp_update_minimum_distance(tsp_t_pointer tsp, int length);

//callback
extern void new_minimun_distance_found(tsp_t_pointer tsp);

typedef struct {
	int start;
	int end;
} partition_interval_t;

extern partition_interval_t get_next_partition(tsp_t_pointer tsp);
inline partition_interval_t get_next_partition_default_impl(int nb_partitions, int nb_clusters, int *next_partition, int processed_partitions);

#endif
