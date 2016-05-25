/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * mppa/matrix_lu/master.c - matrix_lu() master.
 */

#include <arch.h>
#include <assert.h>
#include <global.h>
#include <pthread.h>
#include <stdint.h>
#include <timer.h>
#include <util.h>
#include <ipc.h>
#include "master.h"

/*
 * Arguments sanity check.
 */
#define SANITY_CHECK() \
	assert(m != NULL); \
	assert(l != NULL); \
	assert(u != NULL); \

/*
 * Finds the pivot element.
 */
extern float find_pivot(struct matrix *m, int i0, int j0);

/*
 * Applies the row reduction algorithm in a matrix.
 */
extern void row_reduction(struct matrix *m, int i0);

/*
 * Thread's data.
 */
struct tdata
{
	/* Thread ID. */
	pthread_t tid;
	
	/* Arguments. */
	struct 
	{
		int i0;           /* Start row.      */
		int in;           /* End row.        */
		struct matrix *m; /* Reduced matrix. */
		struct matrix *l; /* Lower matrix.   */
		struct matrix *u; /* Upper matrix.   */
	} args;
} tdata[NUM_IO_CORES];

/*
 * Threads's main.
 */
extern void *thread_main(void *args)
{
	int i, j;        /* Loop index.    */
	struct tdata *t; /* Thread's data. */
	
	t = args;
	
	/* Build upper and lower matrixes.  */
	for (i = t->args.i0; i < t->args.in; i++)
	{
		for (j = 0; j < t->args.m->width; j++)
		{
			if (i > j)
			{
				MATRIX(t->args.l, i, j) = 0.0;
				MATRIX(t->args.u, i, j) = MATRIX(t->args.m, i, j);
			}
			
			else if (i < j)
			{	
				MATRIX(t->args.l, i, j) = MATRIX(t->args.m, i, j);
				MATRIX(t->args.u, i, j) = 0.0;
			}
			
			else
			{
				MATRIX(t->args.l, i, j) = 1.0;
				MATRIX(t->args.u, i, j) = 1.0;
			}
		}
	}
	
	pthread_exit(NULL);
	return (NULL);
}

/*
 * Performs LU factorization in a matrix.
 */
int matrix_lu(struct matrix *m, struct matrix *l, struct matrix *u)
{
	int i;               /* Loop index. */
	float pivot;         /* Pivot.      */
	uint64_t start, end; /* Timer.      */
	
	/* Setup slaves. */
	open_noc_connectors();
	spawn_slaves();
	
	/* Apply elimination on all rows. */
	for (i = 0; i < m->height - 1; i++)
	{
		pivot = find_pivot(m, i, i);	

		/* Impossible to solve. */
		if (pivot == 0.0)
		{
			warning("cannot factorize matrix");
			return (-1);
		}

		row_reduction(m, i);
	}
	
	start = timer_get();

	/* Spawn threads. */
	for (i = 0; i < NUM_IO_CORES; i++)
	{
		tdata[i].args.i0 = i*(m->height >> 2);
		tdata[i].args.in = (i + 1 < NUM_IO_CORES) ? (i + 1)*(m->height >> 2) :
													 m->height;
		tdata[i].args.m = m;
		tdata[i].args.l = l;
		tdata[i].args.u = u;
		pthread_create(&tdata[i].tid, NULL, thread_main, (void *)&tdata[i]);
	}
	
	/* Join threads. */
	for (i = 0; i < NUM_IO_CORES; i++)
		pthread_join(tdata[i].tid, NULL);
		
	end = timer_get();
	master += timer_diff(start, end);
		
	/* House keeping. */
	join_slaves();
	close_noc_connectors();
	
	return (0);
}
