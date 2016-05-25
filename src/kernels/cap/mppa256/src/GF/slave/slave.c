/*
 * Copyright(C) 2014 Matheus M. Queiroz <matheus.miranda.queiroz@gmail.com>, 
 *                   Pedro H. Penna <pedrohenriquepenna@gmail.com>
 */

#include <global.h>
#include <mppaipc.h>
#include <omp.h>
#include <timer.h>
#include <util.h>
#include <ipc.h>
#include "slave.h"

/* Timing statistics. */
uint64_t start;
uint64_t end;
uint64_t communication = 0;
uint64_t total = 0;

/* Gaussian filter parameters. */
static int masksize;
static double mask[MASK_SIZE*MASK_SIZE];
static unsigned char chunk[CHUNK_SIZE*CHUNK_SIZE];

/*
 * Gaussian filter.
 */
void gauss_filter(void)
{
	int i, j;
	int half;
	double pixel;
	int imgI, imgJ, maskI, maskJ;
	
	#define MASK(i, j) \
		mask[(i)*masksize + (j)]
	
	#define CHUNK(i, j) \
		chunk[(i)*CHUNK_SIZE + (j)]
	
	i = 0; j = 0;
	half = CHUNK_SIZE >> 1;
	
	#pragma omp parallel default(shared) private(imgI,imgJ,maskI,maskJ,pixel,i,j)
	{
		#pragma omp for
		for (imgI = 0; imgI < CHUNK_SIZE; imgI++)
		{			
			for (imgJ = 0; imgJ < CHUNK_SIZE; imgJ++)
			{
				pixel = 0.0;
				for (maskI = 0; maskI < masksize; maskI++)
				{	
					for (maskJ = 0; maskJ < masksize; maskJ++)
					{
						i = (imgI - half < 0) ? CHUNK_SIZE-1 - maskI : imgI - half;
						j = (imgJ - half < 0) ? CHUNK_SIZE-1 - maskJ : imgJ - half;

						pixel += CHUNK(i, j)*MASK(maskI, maskJ);
					}
				}
				   
				CHUNK(imgI, imgJ) = (pixel > 255) ? 255 : (int)pixel;
			}
		}
	}
}


int main(int argc, char **argv)
{
	int msg;
	
	timer_init();

	((void)argc);
	
    total = 0;

	rank = atoi(argv[0]);	
	
	/* Setup interprocess communication. */
	open_noc_connectors();
	
	/* Receives filter mask.*/
	data_receive(infd, &masksize, sizeof(int));
	data_receive(infd, mask, sizeof(double)*masksize*masksize);
    
	/* Process chunks. */
    while (1)
	{
		data_receive(infd, &msg, sizeof(int));

		/* Parse message. */
		switch (msg)
		{
			case MSG_CHUNK:
				data_receive(infd, chunk, CHUNK_SIZE*CHUNK_SIZE);
				start = timer_get();
				gauss_filter();
				end = timer_get();
				total += timer_diff(start, end);
				data_send(outfd, chunk, CHUNK_SIZE*CHUNK_SIZE);
				break;
			
			default:
				goto out;
		}
	}

out:
	
	data_send(outfd, &total, sizeof(uint64_t));
	
	close_noc_connectors();
	mppa_exit(0);
	return (0);
}
	
