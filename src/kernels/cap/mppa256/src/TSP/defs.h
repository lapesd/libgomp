#ifndef __DEFS_H
#define __DEFS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <limits.h>
#include <pthread.h>

#define MAX_TOWNS		21
#define MIN_JOBS_THREAD 50
#define MIN_PARTITIONS_PER_CLUSTER 20
#define MAX_MEM_PER_CLUSTER 1572864 //1.5MB
#define INITIAL_JOB_DISTRIBUTION_PERCENTAGE 50.0 //float!!
#define MAX_GRID_X		100
#define MAX_GRID_Y		100
extern const unsigned long long FACTORIAL_TABLE[];


unsigned int get_number_of_partitions (int clusters);

/*
 * We pad the structures to avoid false sharing 
 * during the execution. We combine this passing with the 
 * __attribute__ ((aligned (PAGE_SIZE))) directive on the 
 * variable declaration or posix_memalign for memory allocation
 */
#define PAGE_SIZE 4096
#define PADDING(struct_size) PAGE_SIZE - struct_size + (struct_size / PAGE_SIZE * PAGE_SIZE)

/*
 * LOCKING PRIMITIVES
 */

#ifdef MT 

struct cond_var_struct {
	pthread_mutex_t mutex;
	pthread_cond_t cond_var;
};

#define GET_3TH_ARG(arg1, arg2, arg3, ...) arg3

//COND_VAR_CREATE
#define COND_VAR_T struct cond_var_struct
#define COND_VAR_CREATE_1(cv) COND_VAR_T cv
#define COND_VAR_CREATE_2(cv, modifiers) modifiers COND_VAR_CREATE_1(cv)
#define COND_VAR_CREATE_MACRO_CHOOSER(...) \
    GET_3TH_ARG(__VA_ARGS__, COND_VAR_CREATE_2, COND_VAR_CREATE_1, )
#define COND_VAR_CREATE(...) COND_VAR_CREATE_MACRO_CHOOSER(__VA_ARGS__)(__VA_ARGS__)

#define COND_VAR_INIT(cv) pthread_mutex_init(&cv.mutex, NULL); pthread_cond_init (&cv.cond_var, NULL)
#define COND_VAR_SIGNAL(cv) pthread_cond_signal(&cv.cond_var)
#define COND_VAR_BROADCAST(cv) pthread_cond_broadcast(&cv.cond_var)
#define COND_VAR_WAIT(cv) pthread_cond_wait (&cv.cond_var, &cv.mutex)
#define COND_VAR_MUTEX_LOCK(cv) pthread_mutex_lock(&cv.mutex)
#define COND_VAR_MUTEX_UNLOCK(cv) pthread_mutex_unlock(&cv.mutex)

#ifdef CAS

#define MUTEX_T unsigned int
#define MUTEX_INIT(m) 		m = 0
#define MUTEX_LOCK(m) 		while(__sync_lock_test_and_set(&m, 1))
#define MUTEX_UNLOCK(m) 	__sync_lock_release(&m)	

#else

#define MUTEX_T pthread_mutex_t
#define MUTEX_INIT(m) 		pthread_mutex_init(&m, NULL)
#define MUTEX_LOCK(m) 		pthread_mutex_lock(&m)
#define MUTEX_UNLOCK(m) 	pthread_mutex_unlock(&m)

#endif //CAS

#define MUTEX_CREATE_1(m) MUTEX_T m
#define MUTEX_CREATE_2(m, modifiers) modifiers MUTEX_CREATE_1(m)
#define MUTEX_CREATE_MACRO_CHOOSER(...) \
    GET_3TH_ARG(__VA_ARGS__, MUTEX_CREATE_2, MUTEX_CREATE_1, )
#define MUTEX_CREATE(...) MUTEX_CREATE_MACRO_CHOOSER(__VA_ARGS__)(__VA_ARGS__)

#define MUTEX_SIZE sizeof (MUTEX_T)
#define COND_VAR_SIZE sizeof(COND_VAR_T)

#else //end MT

//Sequential version
#define MUTEX_SIZE 0
#define MUTEX_CREATE(...)	/* none */
#define MUTEX_INIT(m)		/* none */
#define MUTEX_LOCK(m)		/* none */
#define MUTEX_UNLOCK(m)		/* none */

#define COND_VAR_SIZE 0
#define COND_VAR_CREATE(...) 
#define COND_VAR_INIT(cv) 
#define COND_VAR_SIGNAL(cv) 
#define COND_VAR_BROADCAST(cv) 
#define COND_VAR_WAIT(cv) 
#define COND_VAR_MUTEX_LOCK(cv)
#define COND_VAR_MUTEX_UNLOCK(cv) 

#endif //MT



/*
 * DEBUGGING AND TRACING
 */

#ifdef DEBUG
#define LOG(...) printf(__VA_ARGS__); fflush(stdout)
#else
#define LOG(...) 
#endif //DEBUG


enum {
	STARTING_EXECUTION,
	WAITING_FIRST_BARRIER, RELEASED_FIRST_BARRIER,
	GENERATING_TASKS,
	WAITING_FOR_TASKS,
	PROCESSING_TASKS,
	ENDING_THREAD,
	WAITING_SECOND_BARRIER, RELEASED_SECOND_BARRIER,
	ENDING_EXECUTION
}; //Tracing states



#ifdef TRACE_ENABLED

#define TRACE_INIT(...)
#define TRACE(when, cluster_id, thread_id, state) printf("%lu\t%d\t%d\t%d\n", when, cluster_id, thread_id, state) 
#define TRACE_END(...) fflush(stdout)

#else

#define TRACE_INIT(...)
#define TRACE(...) 
#define TRACE_END(...)

#endif //TRACE_ENABLED


/*
 * RANDOM NUMBER GENERATOR
 */

#ifdef DEF_RAND

#define RAND_STATE_T int 
#define RAND_INIT(s) srand(s); 0
#define RAND_NEXT(state) rand()

#else //Our random thread-safe impl.

struct rand_state_struct {
	unsigned int w;
	unsigned int z;
};

#define RAND_STATE_T struct rand_state_struct

struct rand_state_struct simple_rng_initialize(int seed);
inline unsigned int simple_rng_next(struct rand_state_struct *current_state);

#define RAND_INIT(s) simple_rng_initialize(s)
#define RAND_NEXT(state) simple_rng_next(&state)

#endif //DEF_RAND

#endif //__DEFS_H
