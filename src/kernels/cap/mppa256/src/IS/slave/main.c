/*
 * Copyright (C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * mppa/slave/main.c - Slave main().
 */

#include <arch.h>
#include <assert.h>
#include <global.h>
#include <limits.h>
#include <message.h>
#include <omp.h>
#include <stdlib.h>
#include <stdint.h>
#include <timer.h>
#include <util.h>
#include <ipc.h>
#include "slave.h"

/* Timing statistics. */
uint64_t start;
uint64_t end;
uint64_t communication = 0;
uint64_t total = 0;

/* 
 * Array block.
 */
struct 
{
	int size;                                   /* Size of block. */
	int elements[CLUSTER_WORKLOAD/sizeof(int)]; /* Elements.      */
} block;

/*
 * Sorts an array of numbers.
 */
extern void sort2power(int *array, int size, int chunksize);

/*
 * Obeys master.
 */
int main(int argc, char **argv)
{
	int i;               /* Loop index. */
	int id;              /* Bucket ID.  */
	struct message *msg; /* Message.    */
	
	#define NUM_THREADS 4
	
	omp_set_num_threads(NUM_THREADS);
	
	timer_init();
	
	total = 0;
	
	((void)argc);
	
	rank = atoi(argv[0]);
	open_noc_connectors();
	
	/* Slave life. */
	while (1)
	{
		msg = message_receive(infd);
		
		switch (msg->type)
		{
			/* SORTWORK. */
			case SORTWORK:
				/* Receive matrix block. */
				block.size = msg->u.sortwork.size;
				data_receive(infd, block.elements, block.size*sizeof(int));
				
				/* Extract message information. */
				id = msg->u.sortwork.id;
				message_destroy(msg);
				
				start = timer_get();
				for (i = block.size; i < (int)(CLUSTER_WORKLOAD/sizeof(int)); i++)
					block.elements[i] = INT_MAX;
				sort2power(block.elements, CLUSTER_WORKLOAD/sizeof(int), CLUSTER_WORKLOAD/NUM_THREADS);
				end = timer_get();
				total += timer_diff(start, end);
				
				/* Send message back.*/
				msg = message_create(SORTRESULT, id, block.size);
				message_send(outfd, msg);
				data_send(outfd, block.elements, block.size*sizeof(int));
				message_destroy(msg);
				
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
