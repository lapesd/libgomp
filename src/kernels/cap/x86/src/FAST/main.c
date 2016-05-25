/*
 * Copyright(C) 2014 	Alyson D. Pereira <alyson.deies@outlook.com>,
 * 			Pedro H. Penna <pedrohenriquepenna@gmail.com>,
 * 
 * FAST corner detection Benchmark Kernel.
 */

#include <global.h>
#include <math.h>
#include <omp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <timer.h>
#include <util.h>
#include "fast.h"

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

/* Be verbose? */
int verbose = 0;

/* Number of threads. */                
int nthreads = 1;

/* Seed number. */
static int seed = 0;

/* Problem. */           
static struct problem *p = &tiny;

/*
 * Prints program usage and exits.
 */
static void usage(void)
{
	printf("Usage: gf [options]\n");
	printf("Brief: FAST Corner Detection Kernel\n");
	printf("Options:\n");
	printf("  --help             Display this information and exit\n");
	printf("  --nthreads <value> Set number of threads\n");
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
	#define SET_NTHREADS 1 /* Set number of threads. */
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
				case SET_NTHREADS :
					nthreads = atoi(arg);
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
		else if (!strcmp(arg, "--nthreads"))
			state = SET_NTHREADS;
		else if (!strcmp(arg, "--class"))
			state = SET_CLASS;
		else
			usage();
	}
	
	/* Invalid argument(s). */
	if (nthreads < 1)
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
	int i;              /* Loop index.            */
	int *mask;       	/* Mask.                  */
	uint64_t end;       /* End time.              */
	uint64_t start;     /* Start time.            */
	char *img; 			/* Image.                 */
	int numcorners=0;	/* Total corners detected */
	
#ifdef _XEON_PHI_
	double power;
#endif	
	
	readargs(argc, argv);
	
	timer_init();
	srandnum(seed);
	omp_set_num_threads(nthreads);
	
	/* Benchmark initialization. */
	if (verbose)
		printf("initializing...\n");
	start = timer_get();
	img = smalloc(p->imgsize*p->imgsize*sizeof(char));
	for (i = 0; i < p->imgsize*p->imgsize; i++){
		char val = randnum() & 0xff;
		img[i] = (val>0) ? val : val*(-1);
	}	
	mask = smalloc(p->maskrows*p->maskcolumns*sizeof(int));
	generate_mask(mask);
	end = timer_get();
	if (verbose)
		printf("  time spent: %f\n", timer_diff(start, end)*MICROSEC);
	
#ifdef _XEON_PHI_
	power_init();
#endif		
		
	/* Detect corners. */
	if (verbose)
		printf("detecting corners...\n");
	start = timer_get();
	numcorners = fast(img, p->imgsize, mask);
	end = timer_get();
	
#ifdef _XEON_PHI_
	power = power_end();
#endif

	printf("timing statistics:\n");
	printf("  total time:       %f\n", timer_diff(start, end)*MICROSEC);

#ifdef _XEON_PHI_
	printf("  average power: %f\n", power*0.000001);
#endif

	printf("  corners detected: %d\n", numcorners);
	
	/* House keeping. */
	free(mask);
	free(img);
	
	return (0);
}
