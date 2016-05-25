/* 
 * File:   master.c - Mutually Friendly Numbers master process.
 * Authors: 
 *	Matheus A. Souza <matheusalcantarasouza@gmail.com>
 *      Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * Copyright(C) 2014
 *
 */

#include <assert.h>
#include <global.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <mppaipc.h>
#include <timer.h>
#include <util.h>
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


typedef struct {
    int number;
    int num;
    int den;
} Item;

#define MAX_TASK_SIZE 65536

static Item finishedTasks[MAX_TASK_SIZE*NUM_CLUSTERS];
static Item task[MAX_TASK_SIZE];

/* Parameters.*/
static int endnum;      /* Start number.      */
static int startnum;    /* End number.        */
static int *tasksize;   /* Task size.         */
static int avgtasksize; /* Average task size. */

/*
 * Sends works to slaves.
 */
static void sendWork(void)
{
    int i, j;               /* Loop indexes.            */
	uint64_t start, end;    /* Timers.                  */
    int lowernum, uppernum; /* Lower and upper numbers. */

    /* Distribute tasks to slaves. */
    lowernum = startnum; uppernum = endnum;
    for (i = 0; i < nclusters; i++)
    {
		start = timer_get();
		
		/* Build pool of tasks. */
		for (j = 0; j < tasksize[i]; j += 2)
		{
			task[j].number = lowernum++;
			task[j + 1].number = uppernum--;
		}
		
		end = timer_get();
		master += timer_diff(start, end);

		data_send(outfd[i], &tasksize[i], sizeof(int));
		data_send(outfd[i], task, tasksize[i]*sizeof(Item));
    }
    
}

static void syncNumbers(void)
{
	int i;

	for (i = 0; i < nclusters - 1; i++)
	{
        data_receive(infd[i], &finishedTasks[i*avgtasksize], 
													avgtasksize*sizeof(Item));
	}
	
	data_receive(infd[i], &finishedTasks[i*avgtasksize], 
													tasksize[i]*sizeof(Item));
}

/*
 * Thread's data.
 */
struct tdata
{
	/* Thread ID. */
	pthread_t tid; 
	 
	struct
	{
		int i0;
		int in;
	} args;
	
	struct
	{
		int nfriends;
	} result;
} tdata[NUM_IO_CORES];

/*
 * Thread's main.
 */
static void *thread_main(void *args)
{
	int i, j;        /* Loop indexes.      */
	int nfriends;    /* Number of friends. */
	struct tdata *t; /* Thread data.       */
	
	t = args;
	
	/* Count number of friends. */
	nfriends = 0;
	for (i = t->args.i0; i < t->args.in; i++)
	{
		for (j = 0; j < i; j++)
		{
			/* Friends. */
			if ((finishedTasks[i].num == finishedTasks[j].num) && 
			(finishedTasks[i].den == finishedTasks[j].den))
				nfriends++;
		}	
	}
	
	t->result.nfriends = nfriends;
	
	pthread_exit(NULL);
	return (NULL);
}

/*
 * Counts friendly numbers.
 */
static int count_friends(void)
{
	int i;               /* Loop index.       */
	int nfriends;        /* Number friends.   */
	uint64_t start, end; /* Timers.           */
	int chunksize;       /* Thread work size. */
	
	start = timer_get();
	
	/* Spwan slave threads. */
	chunksize = (endnum - startnum + 1)/NUM_IO_CORES;
	for (i = 0; i < NUM_IO_CORES; i++)
	{
		tdata[i].args.i0 = (i == 0) ? startnum + 1 : i*chunksize;
		tdata[i].args.in = (i + 1 < NUM_IO_CORES) ? (i + 1)*chunksize :
													(endnum - startnum + 1);
		pthread_create(&tdata[i].tid, NULL, thread_main, (void *)&tdata[i]);
	}
	
	/* Join threads. */
	for (i = 0; i < NUM_IO_CORES; i++)
		pthread_join(tdata[i].tid, NULL);	
	
	/* Reduce. */
	nfriends = 0;
    for (i = 0; i < NUM_IO_CORES; i++)
		nfriends += tdata[i].result.nfriends;
		
	end = timer_get();
	master += timer_diff(start, end);
    
    return (nfriends);
}


/*
 * Counts the number of friendly numbers in a range.
 */
int friendly_numbers(int _start, int _end) 
{
	int i;           /* Loop index.   */
	int problemsize; /* Problem size. */
	
	startnum = _start;
	endnum = _end;
	
	tasksize = smalloc(nclusters*sizeof(int));
	
	/* Distribute task sizes. */
	problemsize = (_end - _start + 1);
	avgtasksize = problemsize/nclusters;
	for (i = 0; i < nclusters; i++)
		tasksize[i] = (i + 1 < nclusters)?avgtasksize:problemsize-i*avgtasksize;
	
	/* Setup slaves. */
	open_noc_connectors();
	spawn_slaves();
    
	sendWork();
	syncNumbers();
	
	/* House keeping. */
	join_slaves();
	close_noc_connectors();
	free(tasksize);

	return (count_friends());
}
