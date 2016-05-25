#include <mppa.h>
#include "tsp_mppa.h"

extern int k1_get_cluster_id(void);

void set_path_name(char *path, char *template_path, int rx, int tag) {
	sprintf(path, template_path, rx, tag);
}

/**************************************
 * PORTAL COMMUNICATION
 **************************************/

portal_t *mppa_create_read_portal (char *path, void* buffer, unsigned long buffer_size, int trigger, void (*function)(mppa_sigval_t)) {
	portal_t *ret = (portal_t*) malloc (sizeof(portal_t));
	int status;
	ret->file_descriptor = mppa_open(path, O_RDONLY);
	assert(ret->file_descriptor != -1);
	
	mppa_aiocb_t tmp = MPPA_AIOCB_INITIALIZER(ret->file_descriptor, buffer, buffer_size);
	memcpy(&ret->portal, &tmp, sizeof(mppa_aiocb_t));
	
	if (trigger > -1) 
		mppa_aiocb_set_trigger(&ret->portal, trigger);
	
	if (function)
		mppa_aiocb_set_callback(&ret->portal, function);
	
	status = mppa_aio_read(&ret->portal);
	assert(status == 0);

	return ret;
}

portal_t *mppa_create_write_portal (char *path, int min_rank, int max_rank, int includes_ionode) {	
	portal_t *ret = (portal_t*) malloc (sizeof(portal_t));
	ret->file_descriptor = mppa_open(path, O_WRONLY);
	assert(ret->file_descriptor != -1);
	
	if (min_rank != max_rank) {
		int status;
		int i;
		int size = max_rank - min_rank + 1;
		if (includes_ionode) size++;
		int ranks[size]; 		
		
		for (i = 0; i < size; i++)
			ranks[i] = min_rank + i;
		if (includes_ionode)
			ranks[size - 1] = IO_NODE_RANK; 

		status = mppa_ioctl(ret->file_descriptor, MPPA_TX_SET_RX_RANKS, size, ranks);
		assert(status == 0);
	}

   	return ret;
}

void mppa_close_portal (portal_t *portal) {
	assert(mppa_close (portal->file_descriptor) != -1);
	free (portal);
}

void mppa_write_portal (portal_t *portal, void *buffer, int buffer_size, int offset) {
	int status;
	status = mppa_pwrite(portal->file_descriptor, buffer, buffer_size, offset);
	assert(status == buffer_size);
}


/**************************************
 * RQUEUE COMMUNICATION
 **************************************/

  rqueue_t *mppa_create_read_rqueue (int message_size, int rx_id, int rx_tag, char *tx_ids, int tx_tag, int credit) {
	rqueue_t *ret = (rqueue_t *) malloc (sizeof(rqueue_t));
	char path_name[256];
	
	sprintf(path_name, "/mppa/rqueue/%d:%d/%s:%d/%d.%d", rx_id, rx_tag, tx_ids, tx_tag, credit, message_size);
	ret->file_descriptor = mppa_open(path_name, O_RDONLY);
	assert(ret->file_descriptor != -1);

	return ret;
}

rqueue_t *mppa_create_write_rqueue (int message_size, int rx_id, int rx_tag, char *tx_ids, int tx_tag, int credit) {
	rqueue_t *ret = (rqueue_t *) malloc (sizeof(rqueue_t));
	char path_name[256];
	
	sprintf(path_name, "/mppa/rqueue/%d:%d/%s:%d/%d.%d", rx_id, rx_tag, tx_ids, tx_tag, credit, message_size);
	ret->file_descriptor = mppa_open(path_name, O_WRONLY);
	assert(ret->file_descriptor != -1);

	return ret;
}

void mppa_read_rqueue (rqueue_t *rqueue, void *buffer, int buffer_size) {
	int status;
	status = mppa_read(rqueue->file_descriptor, buffer, buffer_size);
	assert(status == buffer_size);
}

void mppa_write_rqueue (rqueue_t *rqueue, void *buffer, int buffer_size) {
	int status;
	status = mppa_write(rqueue->file_descriptor, buffer, buffer_size);
	assert(status == buffer_size);
}

void mppa_close_rqueue(rqueue_t *rqueue) {
	assert(mppa_close (rqueue->file_descriptor) != -1);
	free (rqueue);
}

/**************************************
 * Broadcast
 **************************************/

broadcast_t *mppa_create_broadcast (int clusters, char *path_name, void *buffer, int buffer_size, int includes_ionode, void (*callback_function)(mppa_sigval_t)) {
	int i;
	char filled_path[256];
	broadcast_t *ret = (broadcast_t *) malloc(sizeof(broadcast_t));
	ret->portals = (portal_t **) malloc((clusters + 1) * sizeof(portal_t *));
	ret->clusters = clusters;
	ret->includes_ionode = includes_ionode;
	int rank = k1_get_cluster_id();
	
	for( i = 0; i < clusters; i++) {
		set_path_name(filled_path, path_name, i, i + 40);
		if (i == rank)
			ret->portals[i] = mppa_create_read_portal (filled_path, buffer, buffer_size, 0, callback_function);
		else		
			ret->portals[i] = mppa_create_write_portal(filled_path, i, i, FALSE);
	}

	if (includes_ionode) {
		set_path_name(filled_path, path_name, IO_NODE_RANK, clusters + 40);
		if (rank == IO_NODE_RANK)
			ret->portals[clusters] = mppa_create_read_portal (filled_path, buffer, buffer_size, 0, callback_function);
		else
			ret->portals[clusters] = mppa_create_write_portal(filled_path, IO_NODE_RANK, IO_NODE_RANK, FALSE);
	}

	return ret;
}

void mppa_broadcast (broadcast_t *broadcast, void *value, int size) {
	int i;
	int rank = k1_get_cluster_id();
	int offset = size * ((rank == IO_NODE_RANK) ? broadcast->clusters : rank);

	for (i = 0; i < broadcast->clusters; i++) {
		if (i == rank)
			continue;
		mppa_write_portal(broadcast->portals[i], value, size, offset);
	}

	if (broadcast->includes_ionode && rank != IO_NODE_RANK) {
		mppa_write_portal(broadcast->portals[broadcast->clusters], value, size, offset);	
	}
	
}

void mppa_close_broadcast (broadcast_t *broadcast) {
	int i;
	for (i = 0; i < broadcast->clusters; ++i)
		mppa_close_portal(broadcast->portals[i]);
	if (broadcast->includes_ionode)
		mppa_close_portal(broadcast->portals[broadcast->clusters]);
	free (broadcast->portals);
	free (broadcast);
}

/**************************************
 * BARRIER - INTERNAL
 **************************************/

barrier_t *mppa_create_master_barrier (char *path_master, char *path_slave, int clusters) {
	int status, i;
	int ranks[clusters];
	long long match;

	barrier_t *ret = (barrier_t*) malloc (sizeof (barrier_t));

	ret->sync_fd_master = mppa_open(path_master, O_RDONLY);
	assert(ret->sync_fd_master != -1);
	
	ret->sync_fd_slave = mppa_open(path_slave, O_WRONLY);
	assert(ret->sync_fd_slave != -1);

	match = -1 << clusters;
	status = mppa_ioctl(ret->sync_fd_master, MPPA_RX_SET_MATCH, match);
	assert(status == 0);
	
	for (i = 0; i < clusters; i++)
		ranks[i] = i;

	status = mppa_ioctl(ret->sync_fd_slave, MPPA_TX_SET_RX_RANKS, clusters, ranks);
	assert(status == 0);

	ret->mode = BARRIER_MASTER;

	return ret;
}

barrier_t *mppa_create_slave_barrier (char *path_master, char *path_slave) {
	int status;

	barrier_t *ret = (barrier_t*) malloc (sizeof (barrier_t));

	ret->sync_fd_master = mppa_open(path_master, O_WRONLY);
	assert(ret->sync_fd_master != -1);

	ret->sync_fd_slave = mppa_open(path_slave, O_RDONLY);
	assert(ret->sync_fd_slave != -1);

	status = mppa_ioctl(ret->sync_fd_slave, MPPA_RX_SET_MATCH, (long long) 0);
	assert(status == 0);

	ret->mode = BARRIER_SLAVE;

	return ret;
}

void mppa_barrier_wait(barrier_t *barrier) {
	int status;
	long long dummy;

	if(barrier->mode == BARRIER_MASTER) {
		dummy = -1;
		status = mppa_read(barrier->sync_fd_master, &barrier->match, sizeof(long long));
		assert(status == sizeof(long long));

		status = mppa_write(barrier->sync_fd_slave, &dummy, sizeof(long long));
		assert(status == sizeof(long long));
	}
	else {
		long long mask = (long long) 1 << k1_get_cluster_id();
		dummy = 0;
		
		status = mppa_write(barrier->sync_fd_master, &mask, sizeof(long long));
		assert(status == sizeof(long long));

		status = mppa_read(barrier->sync_fd_slave, &dummy, sizeof(long long));
		assert(status == sizeof(long long));
	}
}

void mppa_close_barrier (barrier_t *barrier) {
	assert(mppa_close(barrier->sync_fd_master) != -1);
	assert(mppa_close(barrier->sync_fd_slave) != -1);
	free(barrier);
}

/**************************************
 * BARRIER - EXTERNAL
 **************************************/

void wait_barrier (barrier_par_t barrier_par) {
	barrier_t *barrier = (barrier_t*)barrier_par.void_t;
	mppa_barrier_wait(barrier); 
}

/**************************************
 * TIME
 **************************************/

static uint64_t residual_error = 0;

void mppa_init_time(void) {
	uint64_t t1, t2;
	t1 = mppa_get_time();
	t2 = mppa_get_time();
	residual_error = t2 - t1;
}

inline uint64_t mppa_get_time(void) {
	return k1_io_read64(0x70084040);
}

inline uint64_t mppa_diff_time(uint64_t t1, uint64_t t2) {
	return t2 - t1 - residual_error;
}
