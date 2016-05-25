/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * mppa/master/row_reduction.c - row_reduction() implementation.
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


/* Work list. */
static struct message *works = NULL; 

/* Timing statistics. */
uint64_t end;
uint64_t start;

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
	for (i = i0 + 1; i < m->height; i += height)
	{
		if (i + height > m->height)
			height = m->height - i;
		
		msg = message_create(REDUCTWORK, i0, i, j0, height, m->width - j0);
		
		push(works, msg);
	}
}

/*
 * Applies the row reduction algorithm in a matrix.
 */
void row_reduction(struct matrix *m, int i0)
{
	int i;               /* Loop indexes.          */
	size_t n;            /* Bytes to send/receive. */
	struct message *msg; /* Message.               */
	
	start = timer_get();
	works_populate(m, i0, i0);
	end = timer_get();
	master += timer_diff(start, end);
	
	/* Send work. */
	i = 0;
	while (!empty(works))
	{	
		pop(works, msg);
		
		/* Send message. */
		message_send(outfd[i], msg);
		
		/* Send pivot line. */
		n = (msg->u.reductwork.width)*sizeof(float);
		
		data_send(outfd[i], &MATRIX(m, msg->u.reductwork.ipvt, msg->u.reductwork.j0), n);
		
		/* Send matrix block. */
		n = (msg->u.reductwork.height)*(msg->u.reductwork.width)*sizeof(float);
		
		data_send(outfd[i], &MATRIX(m,msg->u.reductwork.i0, msg->u.reductwork.j0), n);
		
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
				
				/* Receive matrix block. */
				n = (msg->u.reductresult.height)*(msg->u.reductresult.width)*sizeof(float);
				data_receive(infd[nclusters - i], &MATRIX(m,msg->u.reductresult.i0, msg->u.reductresult.j0), n);
				
				message_destroy(msg);
			}
		}
	}
	
	/* Receive results. */
	for (/* NOOP */ ; i > 0; i--)
	{			
		msg = message_receive(infd[i - 1]);
				
		/* Receive matrix block. */
		n = (msg->u.reductresult.height)*(msg->u.reductresult.width)*sizeof(float);
		data_receive(infd[i - 1], &MATRIX(m,msg->u.reductresult.i0, msg->u.reductresult.j0), n);
				
		message_destroy(msg);
	}
}
