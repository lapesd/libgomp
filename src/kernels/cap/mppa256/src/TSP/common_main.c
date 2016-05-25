#define _GNU_SOURCE             

#include "common_main.h"

void *spawn_worker(void* params) {
	struct execution_parameters *p = (struct execution_parameters *)params;
	(*p->tsp) = init_execution(p->cluster, p->nb_clusters, p->nb_partitions, p->nb_threads, p->nb_towns, p->seed);
	
	wait_barrier (p->barrier);
	start_execution(*p->tsp);
	wait_barrier (p->barrier);
	end_execution(*p->tsp);

	free(params);
	return NULL;
}
