/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 */

#include <global.h>
#include <limits.h>
#include <message.h>
#include <pthread.h>
#include <timer.h>
#include <util.h>
#include <stdio.h>
#include <ipc.h>
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


/* Number of buckets. */
#define NUM_BUCKETS 256

/*
 * Thread's data.
 */
static struct tdata
{
	/* Thread's ID. */
	pthread_t tid;
	
	/* Arguments. */
	struct
	{
		int i0;               /* Start bucket.        */
		int in;               /* End bucket.          */
		int j0;               /* Start array's index. */
		struct bucket **done; /* Buckets.             */
		int *array;           /* Array.               */
	} args;
} tdata[NUM_IO_CORES];

/*
 * Thread's main.
 */
static void *thread_main(void *args)
{
	int i, j;        /* Loop indexes.  */
	struct tdata *t; /* Thread's data. */
		
	t = args;
	
	/* Rebuild array. */
	j = t->args.j0;
	for (i = t->args.i0; i < t->args.in; i++)
	{
		bucket_merge(t->args.done[i], &t->args.array[j]);
		j += bucket_size(t->args.done[i]);
	}
	
	pthread_exit(NULL);
	return (NULL);
}

/*
 * Rebuilds array.
 */
static void rebuild_array(struct bucket **done, int *array)
{
	int j;    /* array[] offset. */
	int i, k; /* Loop index.     */
	
	#define BUCKETS_PER_CORE (NUM_BUCKETS/NUM_IO_CORES)
	
	/* Spawn threads. */
	j = 0;
	for (i = 0; i < NUM_IO_CORES; i++)
	{
		tdata[i].args.i0 = i*BUCKETS_PER_CORE;
		tdata[i].args.in = (i + 1)*BUCKETS_PER_CORE;
		tdata[i].args.done = done;
		tdata[i].args.array = array;
		pthread_create(&tdata[i].tid, NULL, thread_main, (void *)&tdata[i]);
		
		for (k = i*BUCKETS_PER_CORE; k < (i + 1)*BUCKETS_PER_CORE; k++)
			j += bucket_size(done[k]);
	}
	
	/* Join threads. */
	for (i = 0; i < NUM_IO_CORES; i++)
		pthread_join(tdata[i].tid, NULL);
}

/*
 * Bucket-sort algorithm.
 */
extern void bucketsort(int *array, int n)
{
	int max;                  /* Maximum number.      */
	int i, j;                 /* Loop indexes.        */
	int range;                /* Bucket range.        */
	struct minibucket *minib; /* Working mini-bucket. */
	struct message *msg;      /* Working message.     */
	struct bucket **todo;     /* Todo buckets.        */
	struct bucket **done;     /* Done buckets.        */
	uint64_t start, end;      /* Timers.              */
	
	/* Setup slaves. */
	open_noc_connectors();
	spawn_slaves();
	
	todo = smalloc(NUM_BUCKETS*sizeof(struct bucket *));
	done = smalloc(NUM_BUCKETS*sizeof(struct bucket *));
	for (i = 0; i < NUM_BUCKETS; i++)
	{
		done[i] = bucket_create();
		todo[i] = bucket_create();
	}

	/* Find max number in the array. */
	start = timer_get();
	max = INT_MIN;
	for (i = 0; i < n; i++)
	{
		/* Found. */
		if (array[i] > max)
			max = array[i];
	}

	/* Distribute numbers. */
	range = max/NUM_BUCKETS;
	for (i = 0; i < n; i++)
	{
		j = array[i]/range;
		if (j >= NUM_BUCKETS)
			j = NUM_BUCKETS - 1;
		
		bucket_insert(&todo[j], array[i]);
	}
	end = timer_get();
	master += timer_diff(start, end);

	/* Sort buckets. */
	j = 0;
	for (i = 0; i < NUM_BUCKETS; i++)
	{	
		while (bucket_size(todo[i]) > 0)
		{
			minib = bucket_pop(todo[i]);
			
			/* Send message. */
			msg = message_create(SORTWORK, i, minib->size);
			message_send(outfd[j], msg);
			message_destroy(msg);
			
			/* Send data. */
			
				data_send(outfd[j], minib->elements, minib->size*sizeof(int));
			minibucket_destroy(minib);
			
			j++;
			
			/* 
			 * Slave processes are busy.
			 * So let's wait for results.
			 */
			if (j == nclusters)
			{	
				/* Receive results. */
				for (/* NOOP */ ; j > 0; j--)
				{					
					/* Receive message. */
					msg = message_receive(infd[nclusters - j]);
					
					/* Receive mini-bucket. */
					minib = minibucket_create();
					minib->size = msg->u.sortresult.size;
					data_receive(infd[nclusters -j], minib->elements, 
													minib->size*sizeof(int));
					
					bucket_push(done[msg->u.sortresult.id], minib);
					
					message_destroy(msg);
				}
			}
		}
	}

	/* Receive results. */
	for (/* NOOP */ ; j > 0; j--)
	{						
		/* Receive message. */
		msg = message_receive(infd[j - 1]);
					
		/* Receive bucket. */
		minib = minibucket_create();
		minib->size = msg->u.sortresult.size;
		
			data_receive(infd[j - 1], minib->elements, minib->size*sizeof(int));
					
		bucket_push(done[msg->u.sortresult.id], minib);
					
		message_destroy(msg);
	}

	start = timer_get();
	rebuild_array(done, array);
	end = timer_get();
	master += timer_diff(start, end);
	
	/* House keeping. */
	for (i = 0; i < NUM_BUCKETS; i++)
	{
		bucket_destroy(todo[i]);
		bucket_destroy(done[i]);
	}
	free(done);
	free(todo);
	join_slaves();
	close_noc_connectors();
}
