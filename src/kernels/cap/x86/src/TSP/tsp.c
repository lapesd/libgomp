#include <math.h>

#include <timer.h>
#include "tsp.h"

int repopulate_queue (void *tsp_par);

void init_distance (tsp_t *tsp, int seed) {
	int n = tsp->distance->nb_towns;	
	int x[n], y[n];
	int tempdist [n];	
	int i, j, k, city = 0;
	int dx, dy, tmp;

	RAND_STATE_T rand_state = RAND_INIT(seed);	

	for (i = 0; i < n; i++) {
		x[i] = RAND_NEXT(rand_state) % MAX_GRID_X;
		y[i] = RAND_NEXT(rand_state) % MAX_GRID_Y;
	}

	for (i = 0; i < n; i++) {
		for (j = 0; j < n; j++) {
			dx = x[i] - x[j];
			dy = y[i] - y[j];
			tempdist [j] = (int) sqrt ((double) ((dx * dx) + (dy * dy)));
		}

		for (j = 0; j < n; j++) {
			tmp = INT_MAX;
			for (k = 0; k < n; k++) {
				if (tempdist [k] < tmp) {
					tmp = tempdist [k];
					city = k;
				}
			}
			tempdist [city] = INT_MAX;
			tsp->distance->info[i][j].to_city = city;
			tsp->distance->info[i][j].dist = tmp;
		}
	}
}

void print_distance_matrix (distance_matrix_t *distance) {
	int i, j;

	LOG ("distance.n_towns = %d\n", distance->nb_towns);

	for (i = 0; i<distance->nb_towns; i++) {
		LOG ("distance.dst [%1d]",i);
		for (j = 0; j<distance->nb_towns; j++) {
			LOG (" [d:%2d, to:%2d] ", distance->info[i][j].dist, distance->info[i][j].to_city);
		}
		LOG (";\n\n");
	}
	LOG ("done ...\n");
}

static inline void intern_update_minimum_distance(tsp_t_pointer tsp, int new_distance) {
#ifdef NO_CACHE_COHERENCE		
	__builtin_k1_swu(&tsp->min_distance, new_distance);	
#else 
	tsp->min_distance = new_distance;
#endif		
}

inline int init_max_hops(tsp_t_pointer tsp) {
	int total = 1;
	tsp->max_hops = 0;
	while (total < MIN_JOBS_THREAD * tsp->nb_threads * tsp->nb_clusters && tsp->max_hops < tsp->distance->nb_towns - 1) {
		tsp->max_hops++;
		total *= tsp->distance->nb_towns - tsp->max_hops;
	}
	tsp->max_hops++;
	return total;
}

tsp_t_pointer init_tsp(int cluster_id, int nb_clusters, int nb_partitions, int nb_threads, int nb_towns, int seed) {
	int total;
	tsp_t_pointer tsp = (tsp_t_pointer) malloc (sizeof (tsp_t));
	assert(tsp != NULL);
	tsp->distance = (distance_matrix_t *) malloc (sizeof (distance_matrix_t));
	assert(tsp->distance != NULL);

	MUTEX_INIT(tsp->mutex);
	tsp->cluster_id = cluster_id;
	tsp->nb_clusters = nb_clusters;
	tsp->nb_threads = nb_threads;
	tsp->nb_partitions = nb_partitions;
	intern_update_minimum_distance(tsp, INT_MAX);
	tsp->processed_partitions = 0;
	tsp->distance->nb_towns = nb_towns;
	init_distance(tsp, seed);
	total = init_max_hops(tsp);

	if (cluster_id == 0) {
		LOG("MAX_HOPS %d\n", tsp->max_hops);
		LOG("NB_TASKS %d\n", total);
	}
	
	unsigned long queue_size = total / nb_clusters + total % nb_clusters + nb_clusters - 1;
	init_queue(&tsp->queue, queue_size, repopulate_queue, tsp);

	return tsp;
}

void free_tsp(tsp_t_pointer tsp) {
	free_queue(&tsp->queue);
	free(tsp->distance);
	free(tsp);
}

inline int present (int city, int hops, path_t *path) {
	int i;
	for (i = 0; i < hops; i++)
		if ((*path)[i] == city) return 1;
	return 0;
}

void tsp (tsp_t_pointer tsp_par, int hops, int len, path_t *path, unsigned long *cuts, unsigned long long *path_cuts, int thread_id) {
	int i;
	if (len >= tsp_get_shortest_path(tsp_par)) {
#ifdef DEBUG	
		(*cuts)++;
		unsigned long long height = tsp_par->distance->nb_towns - hops;
		assert (height < 21);
		(*path_cuts) += FACTORIAL_TABLE[height];
#endif	
		return;
	}
	if (hops == tsp_par->distance->nb_towns) {
		if (tsp_update_minimum_distance(tsp_par, len)) {
			new_minimun_distance_found(tsp_par);
			LOG ("Cluster %d - Worker[%d] finds path len = %3d :", tsp_par->cluster_id, thread_id, len);
			for (i = 0; i < tsp_par->distance->nb_towns; i++)
				LOG ("%2d ", (*path)[i]);
			LOG ("\n");
		}
	} else {
		int city, me, dist;
		me = (*path)[hops - 1];
		for (i = 0; i < tsp_par->distance->nb_towns; i++) {
			city = tsp_par->distance->info[me][i].to_city;
			if (!present (city, hops, path)) {
				(*path)[hops] = city;
				dist = tsp_par->distance->info[me][i].dist;
				tsp (tsp_par, hops + 1, len + dist, path, cuts, path_cuts, thread_id);
			}
		}
	}
}

void distributor (tsp_t_pointer tsp_par, partition_interval_t partition_interval, int hops, int len, path_t *path, int *job_index) {
	job_t j;
	int i;	
	if (hops == tsp_par->max_hops) {
		int job_partition_id = (*job_index) % tsp_par->nb_partitions;
		if (job_partition_id >= partition_interval.start && job_partition_id <= partition_interval.end) {
			j.len = len;
			for (i = 0; i < hops; i++)
				j.path[i] = (*path)[i];
			add_job (&tsp_par->queue, j);
		}
		(*job_index)++;
	} else {
		int me, city, dist;
		me = (*path)[hops - 1];
		for (i = 0; i < tsp_par->distance->nb_towns; i++) {
			city = tsp_par->distance->info[me][i].to_city;
			if (!present(city, hops, path)) {
				(*path)[hops] = city;
				dist = tsp_par->distance->info[me][i].dist;
				distributor (tsp_par, partition_interval, hops + 1, len + dist, path, job_index);       
			}
		}
	}
}

void generate_jobs (tsp_t_pointer tsp, partition_interval_t partition_interval) {
	int job_count = 0;
	path_t path;	
	path [0] = 0;
	LOG("Cluster %d - Task generation %d-%d starting (queue %p)...\n", tsp->cluster_id, partition_interval.start, partition_interval.end, &tsp->queue);
	distributor (tsp, partition_interval, 1, 0, &path, &job_count);
	LOG("Cluster %d - Task generation for partition %d-%d complete.\n", tsp->cluster_id, partition_interval.start, partition_interval.end);
}

int repopulate_queue (void *tsp_par) {
	tsp_t_pointer tsp = (tsp_t_pointer)tsp_par;

	partition_interval_t partition_interval = get_next_partition(tsp);	
	if (partition_interval.start < 0)
		return 0;
	tsp->processed_partitions += partition_interval.end - partition_interval.start + 1;
	generate_jobs(tsp, partition_interval);
	return 1;
}

void *worker (void *pars) {
	tsp_thread_par_t *p = (tsp_thread_par_t *)pars;

	int jobcount = 0;
	job_t job;
	unsigned long cuts = 0;
	unsigned long long path_cuts = 0;
	

	while (1) {
		int found = get_job (&p->tsp->queue, &job);
		if (!found) break;
		jobcount++;
		tsp (p->tsp, p->tsp->max_hops, job.len, &job.path, &cuts, &path_cuts, p->thread_id);
	}

	LOG ("Worker [%3d,%3d] terminates, %4d jobs done with %16lu cuts %20llu path cuts %10g cut efficiency.\n", 
		p->tsp->cluster_id, p->thread_id, jobcount, cuts, path_cuts, 1.0 * path_cuts / cuts);

	free(pars);
	return NULL;
}

inline int tsp_get_shortest_path (tsp_t *tsp) {
#ifdef NO_CACHE_COHERENCE
	return __builtin_k1_lwu(&tsp->min_distance);
#else
	return tsp->min_distance;
#endif
}

inline int tsp_update_minimum_distance (tsp_t_pointer tsp, int new_distance) {
	int min_updated = 0;
	MUTEX_LOCK(tsp->mutex);
	if (new_distance < tsp_get_shortest_path(tsp)) {
		intern_update_minimum_distance(tsp, new_distance);
		min_updated = 1;
	}
	MUTEX_UNLOCK(tsp->mutex);
	return min_updated;
}



static inline partition_interval_t get_next_partition_block_size(int nb_partitions, int *next_partition, int block_size, int processed_partitions) {
	partition_interval_t ret;
	ret.start = ret.end = -1;	
	if ((*next_partition) < nb_partitions)
		ret.start = (*next_partition);
		if ((*next_partition) + block_size - 1 < nb_partitions)
			ret.end = (*next_partition) + block_size - 1;
		else
			ret.end = ret.start + nb_partitions - ret.start;
		(*next_partition) += block_size;
	return ret;
}

inline partition_interval_t get_next_partition_block(int nb_partitions, int *next_partition, int processed_partitions) {	
	partition_interval_t ret = get_next_partition_block_size(nb_partitions, next_partition, 5, processed_partitions);	
	return ret;
}

inline partition_interval_t get_next_partition_fss(int nb_partitions, int nb_clusters, int *next_partition, int alfa, int processed_partitions) {	
	int block_size;
	block_size = (nb_partitions / (1.0 / (INITIAL_JOB_DISTRIBUTION_PERCENTAGE / 100.0)) / nb_clusters);
	if (processed_partitions != 0) {
		int block_size2 = (nb_partitions - (*next_partition)) / (nb_clusters * alfa);
		block_size = (block_size2 < block_size) ? block_size2 : block_size;
	}
	if (block_size < 1) 
		block_size = 1;
	partition_interval_t ret = get_next_partition_block_size(nb_partitions, next_partition, block_size, processed_partitions);	
	return ret;
}

inline partition_interval_t get_next_partition_gss(int nb_partitions, int nb_clusters, int *next_partition, int processed_partitions) {
	return get_next_partition_fss(nb_partitions, nb_clusters, next_partition, 1, processed_partitions);
}

inline partition_interval_t get_next_partition_default_impl(int nb_partitions, int nb_clusters, int *next_partition, int processed_partitions) {	
	return get_next_partition_gss(nb_partitions, nb_clusters, next_partition, processed_partitions);
}
