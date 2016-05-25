#include "job.h"

#include "tsp.h"

#ifdef NO_CACHE_COHERENCE //See close_queue() for details
static int waiting_threads = 0;
#endif

inline void reset_queue(job_queue_t *q) {
	q->begin = 0;
	q->end = 0;
}

void init_queue (job_queue_t *q, unsigned long max_size, int (*repopulate_queue)(void*), void *repopulate_queue_par) {
	q->max_size = max_size;
	q->status = QUEUE_OK;
	q->repopulate_queue = repopulate_queue;
	q->repopulate_queue_par = repopulate_queue_par;
	reset_queue(q);
	
	q->buffer = (job_queue_node_t *) malloc(sizeof(job_queue_node_t) * q->max_size);
	LOG("Trying to allocate %lu bytes for the queue (max_size = %lu)\n", sizeof(job_queue_node_t) * q->max_size, q->max_size);
	assert(q->buffer != NULL);
	COND_VAR_INIT(q->cond_var);
}

static void close_queue (job_queue_t *q) {	
	q->status = QUEUE_CLOSED;
#ifdef NO_CACHE_COHERENCE
	int i;
	for (i = 0; i < waiting_threads; i++) //Dirty trick to solve the problem of pthread_broadcast on MPPA
		COND_VAR_SIGNAL(q->cond_var);
#else
	COND_VAR_BROADCAST(q->cond_var);
#endif
}

void add_job (job_queue_t *q, job_t j) {
	COND_VAR_MUTEX_LOCK(q->cond_var);
	assert (q->end < (int)q->max_size);
	q->buffer[q->end].tsp_job.len = j.len;
	memcpy (&q->buffer[q->end].tsp_job.path, j.path, sizeof(path_t));
	q->end++;
	COND_VAR_SIGNAL(q->cond_var);
	COND_VAR_MUTEX_UNLOCK(q->cond_var);
}

int get_job (job_queue_t *q, job_t *j) {
	int index;
	
#ifdef NO_CACHE_COHERENCE
	__k1_rmb();
#endif

	if(q->begin == q->end && q->status == QUEUE_CLOSED)
		return 0;
	
	COND_VAR_MUTEX_LOCK(q->cond_var);
	while (q->begin == q->end) {
		switch (q->status) {
			case QUEUE_CLOSED:
				COND_VAR_MUTEX_UNLOCK(q->cond_var);
				return 0;
			case QUEUE_WAIT:
				#ifdef NO_CACHE_COHERENCE
				waiting_threads++; //see close_queue()
				COND_VAR_WAIT(q->cond_var);
				waiting_threads--;
				#else
				COND_VAR_WAIT(q->cond_var);
				#endif
				break;
			case QUEUE_OK:
				q->status = QUEUE_WAIT;
				reset_queue(q);
				COND_VAR_MUTEX_UNLOCK(q->cond_var);
				int jobs_added = q->repopulate_queue(q->repopulate_queue_par);
				COND_VAR_MUTEX_LOCK(q->cond_var);
				if (jobs_added)
					q->status = QUEUE_OK;
				else
					close_queue(q);
		}
	}

	index = q->begin++;
	COND_VAR_MUTEX_UNLOCK(q->cond_var);
	memcpy(j, &q->buffer[index].tsp_job, sizeof(job_t));
	return 1;
} 

void free_queue (job_queue_t *q) {
	free(q->buffer);
}
