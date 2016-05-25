#include "exec.h"

 void do_work (tsp_t_pointer tsp) {
#ifdef MT
	int i;
	pthread_t *tids = NULL;

	if(tsp->nb_threads > 1) {
		tids = (pthread_t *) malloc (sizeof(pthread_t) * (tsp->nb_threads - 1));
		assert (tids != NULL);
	}

	for (i = 0; i < tsp->nb_threads - 1; i++) {
		int status;
		tsp_thread_par_t *par = (tsp_thread_par_t *) malloc(sizeof (tsp_thread_par_t));
		par->tsp = tsp;
		par->thread_id = i;
		status = pthread_create (&tids[i], NULL, worker, par);
		assert (status == 0);
	}

	tsp_thread_par_t *par = (tsp_thread_par_t *) malloc(sizeof (tsp_thread_par_t));
	par->tsp = tsp;
	par->thread_id = tsp->nb_threads - 1;
	worker (par);

	for (i = 0; i < tsp->nb_threads - 1; i++)
		pthread_join (tids[i], NULL);
	free (tids);
#else
	tsp_thread_par_t *par = (tsp_thread_par_t *) malloc(sizeof (tsp_thread_par_t));
	assert (par != NULL);
	par->tsp = tsp;
	par->thread_id = 1;
	worker (par);
#endif
}

tsp_t_pointer init_execution(int cluster_id, int nb_clusters, int nb_partitions, int nb_threads, int nb_towns, int seed) {
	return init_tsp(cluster_id, nb_clusters, nb_partitions, nb_threads, nb_towns, seed);
}

void start_execution(tsp_t_pointer tsp) {
	do_work(tsp);
}

void end_execution (tsp_t_pointer tsp) {
	free_tsp(tsp);
}
