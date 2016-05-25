#include "../tsp_mppa.h"

static int nb_clusters;
static int cluster_id;

MUTEX_CREATE(min_slave_to_master_lock, static);

void callback_slave (mppa_sigval_t sigval);

static int *comm_buffer;
static int comm_buffer_size;
static broadcast_t *broad;

static rqueue_t *rqueue_partition_request, *rqueue_partition_response;

static tsp_t_pointer tsp_instance;

int main(int argc, char **argv) {	
	int nb_threads = atoi(argv[0]);
	int nb_towns = atoi(argv[1]);
	int seed = atoi(argv[2]);
	nb_clusters = atoi(argv[3]);
	cluster_id =  atoi(argv[4]);

	run_tsp(nb_threads, nb_towns, seed, nb_clusters);

	free(comm_buffer);

	mppa_exit(0);
	return 0;
}

void run_tsp (int nb_threads, int nb_towns, int seed, int nb_clusters) {
	int i;	
	MUTEX_INIT(min_slave_to_master_lock);
	//int cluster_id = __k1_get_cluster_id();

	comm_buffer_size = (nb_clusters + 1) * sizeof(int);
	comm_buffer = (int *) malloc(comm_buffer_size);
	for (i = 0; i <= nb_clusters; i++)
		comm_buffer[i] = INT_MAX;

	barrier_t *sync_barrier = mppa_create_slave_barrier (BARRIER_SYNC_MASTER, BARRIER_SYNC_SLAVE);
	barrier_par_t barrier;
	barrier.void_t = sync_barrier;

	broad = mppa_create_broadcast (nb_clusters, BROADCAST_MASK, comm_buffer, comm_buffer_size, TRUE, callback_slave);
	rqueue_partition_request = mppa_create_write_rqueue(2 * sizeof(int), 128, 70, "[0..15]", 71, 1);
	rqueue_partition_response = mppa_create_read_rqueue(sizeof(partition_interval_t), cluster_id, cluster_id + 72, "128", cluster_id + 72 + MAX_CLUSTERS, 1);

	tsp_instance = init_execution(cluster_id, nb_clusters, get_number_of_partitions(nb_clusters), nb_threads, nb_towns, seed);
	wait_barrier (barrier);
	start_execution(tsp_instance);
	wait_barrier (barrier);	
	end_execution(tsp_instance);

	mppa_close_broadcast(broad);
	mppa_close_barrier(sync_barrier);
	mppa_close_rqueue(rqueue_partition_response);
	mppa_close_rqueue(rqueue_partition_request);
}

void new_minimun_distance_found(tsp_t_pointer tsp_instance) {
	MUTEX_LOCK(min_slave_to_master_lock);
	int min_local = tsp_get_shortest_path(tsp_instance);
	mppa_broadcast (broad, &min_local, sizeof(int));	
	MUTEX_UNLOCK(min_slave_to_master_lock);
}

void callback_slave (mppa_sigval_t sigval) {	
	int i, min = INT_MAX;
	for (i = 0; i < nb_clusters; i++)
		if (comm_buffer[i] < min) 
			min = comm_buffer[i];
	tsp_update_minimum_distance(tsp_instance, min);	
}

partition_interval_t get_next_partition(tsp_t_pointer tsp) {
	int buffer[2];
	partition_interval_t ret;
	
	buffer[0] = tsp->cluster_id;
	buffer[1] = tsp->processed_partitions;

	mppa_write_rqueue (rqueue_partition_request, buffer, 2 * sizeof(int));
	mppa_read_rqueue (rqueue_partition_response, &ret, sizeof(partition_interval_t));

	return ret;
}
