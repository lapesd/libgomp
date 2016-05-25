/*
 * Copyright (C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * mppa/slave/main.c - Slave main().
 */

#include <arch.h>
#include <assert.h>
#include <global.h>
#include <math.h>
#include <message.h>
#include <omp.h>
#include <stdlib.h>
#include <stdint.h>
#include <timer.h>
#include <util.h>
#include <ipc.h>
#include "slave.h"

/* 
 * Matrix block
 */
struct 
{
	int height;                                     /* Block height. */
	int width;                                      /* Block width.  */
	float elements[CLUSTER_WORKLOAD/sizeof(float)]; /* Elements.     */
} block;

/*
 * Pivot line.
 */
struct
{
	int width;                                          /* Pivot line width. */ 
	float elements[CLUSTER_WORKLOAD/(4*sizeof(float))]; /* Elements.         */
} pvtline;

/* Timing statistics. */
uint64_t start;
uint64_t end;
uint64_t communication = 0;
uint64_t total = 0;

/*
 * Returns the element [i][j] of the block.
 */
#define BLOCK(i, j) \
	(block.elements[block.width*(i) + (j)])

/*
 * Finds the pivot element.
 */
static void _find_pivot(int *ipvt, int *jpvt)
{
	int tid;                             /* Thread ID.        */
	int i, j;                            /* Loop indexes.     */
	int _ipvt[(NUM_CORES/NUM_CLUSTERS)]; /* Index i of pivot. */
	int _jpvt[(NUM_CORES/NUM_CLUSTERS)]; /* Index j of pivot. */
	
	#pragma omp parallel private(i, j, tid) default(shared)
	{
		tid = omp_get_thread_num();
	
		_ipvt[tid] = 0;
		_jpvt[tid] = 0;
	
		/* Find pivot element. */
		#pragma omp for
		for (i = 0; i < block.height; i++)
		{
			for (j = 0; j < block.width; j++)
			{
				/* Found. */
				if (fabs(BLOCK(i, j)) > fabs(BLOCK(_ipvt[tid],_jpvt[tid])))
				{
					_ipvt[tid] = i;
					_jpvt[tid] = j;
				}
			}
		}
	}
	
	/* Min reduction of pivot. */
	for (i = 1; i < (NUM_CORES/NUM_CLUSTERS); i++)
	{
		/* Smaller found. */
		if (fabs(BLOCK(_ipvt[i], _jpvt[i])) > fabs(BLOCK(_ipvt[0],_jpvt[0])))
		{
			_ipvt[0] = _ipvt[i];
			_jpvt[0] = _jpvt[i];
		}
	}
	
	*ipvt = _ipvt[0];
	*jpvt = _jpvt[0];
}

/*
 * Applies the row reduction algorithm in a matrix.
 */
void row_reduction(void)
{
	int i, j;    /* Loop indexes.   */
	float mult;  /* Row multiplier. */
	float pivot; /* Pivot element.  */
	
	pivot = pvtline.elements[0];
	
	/* Apply row redution in some lines. */
	#pragma omp parallel for private(i, j, mult) default(shared)
	for (i = 0; i < block.height; i++)
	{
		mult = BLOCK(i, 0)/pivot;
	
		/* Store multiplier. */
		BLOCK(i, 0) = mult;
	
		/* Iterate over columns. */
		for (j = 1; j < block.width; j++)
			BLOCK(i, j) = BLOCK(i, j) - mult*pvtline.elements[j];
	}
}

/*
 * Obeys master.
 */
int main(int argc, char **argv)
{
	int i0, j0;    /* Block start.             */
	int ipvt;      /* ith idex of pivot.       */
	int jpvt;      /* jth index of pivot.      */
	size_t n;      /* Number of bytes to send. */
	struct message *msg; /* Message.           */
	
	((void)argc);
	
	timer_init();
	
	rank = atoi(argv[0]);
	open_noc_connectors();
	
	/* Slave life. */
	while (1)
	{
		msg = message_receive(infd);
		
		switch (msg->type)
		{
			/* FINDWORK. */
			case FINDWORK:
				/* Receive matrix block. */
				n = (msg->u.findwork.height)*(msg->u.findwork.width)*sizeof(float);
				data_receive(infd, block.elements, n);
				
				/* Extract message information. */
				block.height = msg->u.findwork.height;
				block.width = msg->u.findwork.width;
				i0 = msg->u.findwork.i0;
				j0 = msg->u.findwork.j0;
				message_destroy(msg);
				
				start = timer_get();
				_find_pivot(&ipvt, &jpvt);
				end = timer_get();
				total += timer_diff(start, end);
				
				/* Send message back.*/
				msg = message_create(FINDRESULT, i0, j0, ipvt, jpvt);
				message_send(outfd, msg);
				message_destroy(msg);
				
				break;
			
			/* REDUCTRESULT. */
			case REDUCTWORK :
				/* Receive pivot line. */
				n = (msg->u.reductwork.width)*sizeof(float);
				assert(n <= sizeof(pvtline.elements));
				data_receive(infd, pvtline.elements, n);
				
				/* Receive matrix block. */
				n = (msg->u.reductwork.height)*(msg->u.reductwork.width)*sizeof(float);
				data_receive(infd, block.elements, n);
			
				/* Extract message information. */
				block.height = msg->u.reductwork.height;
				block.width = pvtline.width = msg->u.reductwork.width;
				i0 = msg->u.reductwork.i0;
				j0 = msg->u.reductwork.j0;
				message_destroy(msg);
			
				start = timer_get();
				row_reduction();
				end = timer_get();
				total += timer_diff(start, end);
				
				/* Send message back.*/
				msg = message_create(REDUCTRESULT, i0, j0, block.height, block.width);
				message_send(outfd, msg);
				message_destroy(msg);
				
				/* Send matrix block. */
				n = (block.height)*(block.width)*sizeof(float);
				data_send(outfd, block.elements, n);
				
				break;
				
			/* DIE. */
			default:
				message_destroy(msg);
				goto out;		
		}
	}

out:

	data_send(outfd, &total, sizeof(uint64_t));
	close_noc_connectors();
	mppa_exit(0);
	return (0);
}
