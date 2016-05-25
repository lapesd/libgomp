/*
 * Copyright(C) 2014 	Alyson D. Pereira <alyson.deives@outlook.com>, 
 * 			Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * FAST Corner Detection Benchmark Kernel.
 */

#include <global.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <timer.h>
#include <util.h>
#include "master.h"

/*
 * FAST corner detection.
 */
extern int fast
(char *img, char *output,int imgsize, int *mask, int masksize);

/* Timing statistics. */
uint64_t master = 0;          /* Time spent on master.        */
uint64_t slave[NUM_CLUSTERS]; /* Time spent on slaves.        */
uint64_t communication = 0;   /* Time spent on communication. */
uint64_t total = 0;           /* Total time.                  */

/* Data exchange statistics. */
size_t data_sent = 0;     /* Number of bytes received. */
unsigned nsend = 0;       /* Number of sends.          */
size_t data_received = 0; /* Number of bytes sent.     */
unsigned nreceive = 0;    /* Number of receives.       */

/*
 * Problem.
 */
struct problem
{
	int maskcolumns; /* Mask columns. */
	int maskrows;    /* Mask Rows.    */
	int masksize;	 /* Mask Size.    */
	int imgsize;  	 /* Image size.   */
};

/* Problem sizes. */
static struct problem tiny     = { 2,27,54,  2048};
static struct problem small    = { 2,27,54,  4096};
static struct problem standard = { 2,27,54,  8192};
static struct problem large    = { 2,27,54, 16384};
static struct problem huge     = { 2,27,54, 24576};

/* Benchmark parameters. */
int verbose = 0;                  /* Be verbose?        */
int nclusters = 1;                 /* Number of threads. */
static int seed = 0;              /* Seed number.       */
static struct problem *p = &tiny; /* Problem.           */

/*
 * Prints program usage and exits.
 */
static void usage(void)
{
	printf("Usage: gf [options]\n");
	printf("Brief: Fast Corner Detection Benchmark Kernel\n");
	printf("Options:\n");
	printf("  --help             Display this information and exit\n");
	printf("  --nclusters <value> Set number of threads\n");
	printf("  --class <name>     Set problem class:\n");
	printf("                       - small\n");
	printf("                       - standard\n");
	printf("                       - large\n");
	printf("                       - huge\n");
	printf("  --verbose          Be verbose\n");
	exit(0);
}

/*
 * Reads command line arguments.
 */
static void readargs(int argc, char **argv)
{
	int i;     /* Loop index.       */
	char *arg; /* Working argument. */
	int state; /* Processing state. */
	
	/* State values. */
	#define READ_ARG     0 /* Read argument.         */
	#define SET_nclusters 1 /* Set number of threads. */
	#define SET_CLASS    2 /* Set problem class.     */
	
	state = READ_ARG;
	
	/* Read command line arguments. */
	for (i = 1; i < argc; i++)
	{
		arg = argv[i];
		
		/* Set value. */
		if (state != READ_ARG)
		{
			switch (state)
			{
				/* Set problem class. */
				case SET_CLASS :
					if (!strcmp(argv[i], "tiny"))
						p = &tiny;
					else if (!strcmp(argv[i], "small"))
						p = &small;
					else if (!strcmp(argv[i], "standard"))
						p = &standard;
					else if (!strcmp(argv[i], "large"))
						p = &large;
					else if (!strcmp(argv[i], "huge"))
						p = &huge;
					else 
						usage();
					state = READ_ARG;
					break;
				
				/* Set number of threads. */
				case SET_nclusters :
					nclusters = atoi(arg);
					state = READ_ARG;
					break;
				
				default:
					usage();			
			}
			
			continue;
		}
		
		/* Parse argument. */
		if (!strcmp(arg, "--verbose"))
			verbose = 1;
		else if (!strcmp(arg, "--nclusters"))
			state = SET_nclusters;
		else if (!strcmp(arg, "--class"))
			state = SET_CLASS;
		else
			usage();
	}
	
	/* Invalid argument(s). */
	if (nclusters < 1)
		usage();
}

/*
 * Generates mask.
 */
static void generate_mask(int *mask)
{
	mask[0*p->maskcolumns + 0] = -1;
	mask[0*p->maskcolumns + 1] = -3;

	mask[1*p->maskcolumns + 0] = 0;
	mask[1*p->maskcolumns + 1] = -3;

	mask[2*p->maskcolumns + 0] = 1;
	mask[2*p->maskcolumns + 1] = -3;
	
	mask[3*p->maskcolumns + 0] = 2;
	mask[3*p->maskcolumns + 1] = -2;

	mask[4*p->maskcolumns + 0] = 3;
	mask[4*p->maskcolumns + 1] = -1;

	mask[5*p->maskcolumns + 0] = 3;
	mask[5*p->maskcolumns + 1] = 0;

	mask[6*p->maskcolumns + 0] = 3;
	mask[6*p->maskcolumns + 1] = 1;

	mask[7*p->maskcolumns + 0] = 2;
	mask[7*p->maskcolumns + 1] = 2;

	mask[8*p->maskcolumns + 0] = 1;
	mask[8*p->maskcolumns + 1] = 3;

	mask[9*p->maskcolumns + 0] = 0;
	mask[9*p->maskcolumns + 1] = 3;

	mask[10*p->maskcolumns + 0] = -1;
	mask[10*p->maskcolumns + 1] = 3;

	mask[11*p->maskcolumns + 0] = -2;
	mask[11*p->maskcolumns + 1] = 2;

	mask[12*p->maskcolumns + 0] = -3;
	mask[12*p->maskcolumns + 1] = 1;

	mask[13*p->maskcolumns + 0] = -3;
	mask[13*p->maskcolumns + 1] = 0;

	mask[14*p->maskcolumns + 0] = -3;
	mask[14*p->maskcolumns + 1] = -1;

	mask[15*p->maskcolumns + 0] = -2;
	mask[15*p->maskcolumns + 1] = -2;

	mask[16*p->maskcolumns + 0] = -1;
	mask[16*p->maskcolumns + 1] = -3;

	mask[17*p->maskcolumns + 0] = 0;
	mask[17*p->maskcolumns + 1] = -3;

	mask[18*p->maskcolumns + 0] = 1;
	mask[18*p->maskcolumns + 1] = -3;
	
	mask[19*p->maskcolumns + 0] = 2;
	mask[19*p->maskcolumns + 1] = -2;

	mask[20*p->maskcolumns + 0] = 3;
	mask[20*p->maskcolumns + 1] = -1;

	mask[21*p->maskcolumns + 0] = 3;
	mask[21*p->maskcolumns + 1] = 0;

	mask[22*p->maskcolumns + 0] = 3;
	mask[22*p->maskcolumns + 1] = 1;

	mask[23*p->maskcolumns + 0] = 2;
	mask[23*p->maskcolumns + 1] = 2;
	
	mask[24*p->maskcolumns + 0] = 1;
	mask[24*p->maskcolumns + 1] = 3;

	mask[25*p->maskcolumns + 0] = 0;
	mask[25*p->maskcolumns + 1] = 3;

	mask[26*p->maskcolumns + 0] = -1;
	mask[26*p->maskcolumns + 1] = 3;
}

/*
 * Runs benchmark.
 */
int main(int argc, char **argv)
{
	int i;              /* Loop index.         */
	int *mask;      	/* Mask.               */
	uint64_t end;       /* End time.           */
	uint64_t start;     /* Start time.         */
	char *img; 			/* Image input.        */
	char *output;		/* Image output.		*/
	int numcorners=0;	/* Total corners detected */
		
	readargs(argc, argv);
	
	timer_init();
	srandnum(seed);
	
	/* Benchmark initialization. */
	if (verbose)
		printf("initializing...\n");
	start = timer_get();
	img = smalloc(p->imgsize*p->imgsize*sizeof(char));
	output = scalloc(p->imgsize*p->imgsize,sizeof(char));
	for (i = 0; i < p->imgsize*p->imgsize; i++){
		char val = (char) (randnum() & 0xff);
		img[i] = (val>0) ? val : val*(-1);
	}
	mask = smalloc(p->maskrows*p->maskcolumns*sizeof(int));
	generate_mask(mask);
	end = timer_get();
	if (verbose)
		printf("  time spent: %f\n", timer_diff(start, end)*MICROSEC);
		
	/* Apply filter. */
	if (verbose)
		printf("Detecting corners...\n");
	start = timer_get();
	numcorners = fast(img, output, p->imgsize, mask, p->masksize);
	end = timer_get();
	
	total = timer_diff(start, end);

	/* Print timing statistics. */
	printf("timing statistics:\n");
	printf("  master:           %f\n", master*MICROSEC);
	for (i = 0; i < nclusters; i++){
		printf("  slave %d:         %f\n",i,slave[i]*MICROSEC);
	}
	printf("  communication:    %f\n", communication*MICROSEC);
	printf("  total time:       %f\n", total*MICROSEC);
	printf("data exchange statistics:\n");
	printf("  data sent:            %d\n", data_sent);
	printf("  number sends:         %u\n", nsend);
	printf("  data received:        %d\n", data_received);
	printf("  number receives:      %u\n", nreceive);
	
	/* House keeping. */
	free(mask);
	free(img);
	
	return (0);
}
