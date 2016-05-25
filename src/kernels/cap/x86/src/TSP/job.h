#ifndef __JOB_H
#define __JOB_H

#include "defs.h"

typedef int path_t[MAX_TOWNS];

typedef struct {
	int len;
	path_t path;
} job_t;

typedef struct job_queue_node {
	job_t tsp_job;
	struct job_queue_node *next;
} job_queue_node_t;

typedef enum {
	QUEUE_CLOSED = 0,
	QUEUE_WAIT = 1,
	QUEUE_OK = 2
} queue_status_t;

typedef struct {
	job_queue_node_t *buffer;
	unsigned long max_size;
	queue_status_t status;
	int (*repopulate_queue)(void*);
	void *repopulate_queue_par;
	char PADDING [PADDING(sizeof(job_queue_node_t *) + sizeof (unsigned long) + sizeof(int) + sizeof (int (*)(void*)) + sizeof (void *))];
	int begin;
	int end;
	COND_VAR_CREATE(cond_var);
	char PADDING2 [PADDING(COND_VAR_SIZE) + 2 * sizeof(int)];
} job_queue_t;

void init_queue (job_queue_t *q, unsigned long max_size, int (*repopulate_queue)(void*), void *repopulate_queue_par);
void add_job (job_queue_t *q, job_t j);
int get_job (job_queue_t *q, job_t *j);
void free_queue (job_queue_t *q);

#endif
