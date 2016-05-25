/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 */


#include <arch.h>
#include <global.h>
#include <math.h>
#include <message.h>
#include <stdint.h>
#include <timer.h>
#include <util.h>
#include "master.h"

/*
 * Wrapper to data_send(). 
 */
#define data_send(a, b, c)                   \
	{                                        \
		data_sent += c;                      \
		nsend++;                             \
		communication += data_send(a, b, c); \
	}                                        \

/*
 * Wrapper to data_receive(). 
 */
#define data_receive(a, b, c)                   \
	{                                           \
		data_received += c;                     \
		nreceive++;                             \
		communication += data_receive(a, b, c); \
	}                                           \


/* Lists. */
static struct message *works = NULL;   /* Work list.   */
static struct message *results = NULL; /* Result list. */

/* Timing statistics. */
uint64_t end;
uint64_t start;

/*
 * Swaps two rows of a matrix.
 */
static void _swap_rows(struct matrix *m, int i1, int i2)
{
	int j;     /* Loop index.      */
	float tmp; /* Temporary value. */
	
	/* Swap columns. */
	for (j = 0; j < m->width; j++)
	{
		tmp = MATRIX(m, i1, j);
		MATRIX(m, i1, j) = MATRIX(m, i2, j);
		MATRIX(m, i2, j) = tmp;
	}
}

/*
 * Swaps two columns in a matrix.
 */
static void _swap_columns(struct matrix *m, int j1, int j2)
{
	int i;     /* Loop index.      */
	float tmp; /* Temporary value. */

	/* Swap columns. */
	for (i = 0; i < m->height; i++)
	{
		tmp = MATRIX(m, i, j1);
		MATRIX(m, i, j1) = MATRIX(m, i, j2);
		MATRIX(m, i, j2) = tmp;
	}
}

/*
 * Populates work list.
 */
static void works_populate(struct matrix *m, int i0, int j0)
{
	int i;               /* Loop index.     */
	int height;          /* Number of rows. */
	struct message *msg; /* Work.           */
	
	height = (CLUSTER_WORKLOAD/sizeof(float))/((m->width - j0)*sizeof(float));
	
	/* Populate works. */
	for (i = i0; i < m->height; i += height)
	{
		if (i + height > m->height)
			height = m->height - i;
		
		msg = message_create(FINDWORK, i, j0, height, m->width - j0);
		
		push(works, msg);
	}
}

/*
 * Finds the pivot element.
 */
float find_pivot(struct matrix *m, int i0, int j0)
{
	size_t n;            /* Number of bytes to send. */
	int i,j;             /* Loop index.              */
	int ipvt;            /* ith index of pivot.      */
	int jpvt;            /* jth index of pivot.      */
	struct message *msg; /* Message.                 */
	
	start = timer_get();
	works_populate(m, i0, j0);
	end = timer_get();
	master += timer_diff(start, end);
	
	i = 0;
	
	/* Send work. */
	while (!empty(works))
	{	
		pop(works, msg);
		
		/* Send message. */
		message_send(outfd[i], msg);
		
		/* Send data. */
		n = (msg->u.findwork.height)*(msg->u.findwork.width)*sizeof(float);
		
		data_send(outfd[i], &MATRIX(m,msg->u.findwork.i0,msg->u.findwork.j0), n);
		
		i++;
		message_destroy(msg);
		
		/* 
		 * Slave processes are busy.
		 * So let's wait for results.
		 */
		if (i == nclusters)
		{
			/* Receive results. */
			for (/* NOOP */ ; i > 0; i--)
			{
				msg = message_receive(infd[nclusters - i]);
				push(results, msg);
			}
		}
	}
	
	/* Receive results. */
	for (/* NOOP */ ; i > 0; i--)
	{	
		msg = message_receive(infd[i - 1]);
		push(results, msg);
	}
	
	start = timer_get();
	
	/* Find pivot. */
	ipvt = i0;
	jpvt = j0;
	while (!empty(results))
	{
		pop(results, msg);
		
		i = msg->u.findresult.ipvt + msg->u.findresult.i0;
		j = msg->u.findresult.jpvt;
		
		if (fabs(MATRIX(m, i, j)) > fabs(MATRIX(m, ipvt, jpvt)))
		{
			ipvt = i;
			jpvt = j;
		}
		
		message_destroy(msg);
	}

	_swap_rows(m, i0, ipvt);
	_swap_columns(m, j0, jpvt);
	
	
	end = timer_get();
	master += timer_diff(start, end);
	
	return (MATRIX(m, ipvt, jpvt));
}
