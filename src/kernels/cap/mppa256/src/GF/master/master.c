/*
 * Copyright(C) 2014 Matheus M. Queiroz <matheus.miranda.queiroz@gmail.com>, 
 *                   Pedro H. Penna <pedrohenriquepenna@gmail.com>
 */

#include <arch.h>
#include <assert.h>
#include <global.h>
#include <inttypes.h>
#include <math.h>
#include <mppaipc.h>
#include <stdlib.h>
#include <string.h>
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


/*
 * Gaussian filter.
 */
void gauss_filter(unsigned char *img, int imgsize, double *mask, int masksize)
{	
	int i,j;             /* Loop indexes.     */ 
	size_t n;            /* Bytes to send.    */
	int msg;             /* Message.          */
	int nchunks;         /* Number of chunks. */
	
	open_noc_connectors();
	spawn_slaves();
	
	 /* Send mask. */
    n = sizeof(double)*masksize*masksize;	
	for (i = 0; i < nclusters; i++)
	{
		data_send(outfd[i], &masksize, sizeof(int));
		data_send(outfd[i], mask, n);
	}
    
    /* Process image in chunks. */
    j = 0; n = CHUNK_SIZE*CHUNK_SIZE; msg = MSG_CHUNK;
    nchunks = (imgsize*imgsize)/(CHUNK_SIZE*CHUNK_SIZE);
    for (i = 0; i < nchunks; i++)
    {		
		data_send(outfd[j], &msg, sizeof(int));
		data_send(outfd[j], &img[i*(CHUNK_SIZE*CHUNK_SIZE)],n);
		
		j++;
		
		/* 
		 * Slave processes are busy.
		 * So let's wait for results.
		 */
		if (j == nclusters)
		{
			for (/* NOOP */ ; j > 0; j--)
			{
				data_receive(infd[nclusters-j],
								   &img[(nclusters-j)*CHUNK_SIZE*CHUNK_SIZE], n);
			}
		}
	}
	
	/* Receive remaining results. */
	for (/* NOOP */ ; j > 0; j--)
	{
		data_receive(infd[j - 1], 
								&img[(nchunks - j)*CHUNK_SIZE*CHUNK_SIZE], n);
	}
	
	/* House keeping. */
	msg = MSG_DIE;
	for (i = 0; i < nclusters; i++)
		data_send(outfd[i], &msg, sizeof(int));
	join_slaves();
	close_noc_connectors();
}
