#ifndef __TSP_MPPA_H
#define __TSP_MPPA_H

#include <mppaipc.h>
#include <inttypes.h>

#include "common_main.h"
#include "exec.h"

//to
#define BROADCAST_MASK "/mppa/portal/%d:%d"

#define BARRIER_SYNC_MASTER "/mppa/sync/128:20"
#define BARRIER_SYNC_SLAVE "/mppa/sync/[0..15]:20"

#define TRUE 1
#define FALSE 0

#define IO_NODE_RANK 128
#define MAX_CLUSTERS 16
#define MAX_THREADS_PER_CLUSTER 16
#define MPPA_FREQUENCY 400

typedef enum {
	BARRIER_MASTER,
	BARRIER_SLAVE
} barrier_mode_t;

typedef struct {
	int file_descriptor;
	mppa_aiocb_t portal;
} portal_t;

typedef struct {
	int file_descriptor;
} rqueue_t;

typedef struct {
	int sync_fd_master;
	int sync_fd_slave;
	long long match;
	barrier_mode_t mode; 
} barrier_t;

typedef struct {
	int clusters;
	int includes_ionode;
	portal_t **portals;
} broadcast_t;

void set_path_name(char *path, char *template_path, int rx, int tag);

portal_t *mppa_create_read_portal (char *path, void* buffer, unsigned long buffer_size, int trigger, void (*function)(mppa_sigval_t));
portal_t *mppa_create_write_portal (char *path, int min_rank, int max_rank, int includes_ionode);
void mppa_write_portal (portal_t *portal, void *buffer, int buffer_size, int offset);
void mppa_close_portal (portal_t *portal);

rqueue_t *mppa_create_read_rqueue (int message_size, int rx_id, int rx_tag, char *tx_ids, int tx_tag, int credit);
rqueue_t *mppa_create_write_rqueue (int message_size, int rx_id, int rx_tag, char *tx_ids, int tx_tag, int credit);
void mppa_read_rqueue (rqueue_t *rqueue, void *buffer, int buffer_size);
void mppa_write_rqueue (rqueue_t *rqueue, void *buffer, int buffer_size);
void mppa_close_rqueue(rqueue_t *rqueue);

broadcast_t *mppa_create_broadcast (int clusters, char *path_name, void *buffer, int buffer_size, int includes_ionode, void (*callback_function)(mppa_sigval_t));
void mppa_broadcast (broadcast_t *broadcast, void * value, int size);
void mppa_close_broadcast (broadcast_t *broadcast);

barrier_t *mppa_create_master_barrier (char *path_master, char *path_slave, int clusters);
barrier_t *mppa_create_slave_barrier (char *path_master, char *path_slave);
void mppa_barrier_wait (barrier_t *barrier);
void mppa_close_barrier (barrier_t *barrier);

void mppa_init_time(void);
inline uint64_t mppa_get_time(void);
inline uint64_t mppa_diff_time(uint64_t t1, uint64_t t2);

#endif // __TSP_MPPA_H
