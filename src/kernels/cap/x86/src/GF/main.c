/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * Gaussian Filter Benchmark Kernel.
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
#include "gf.h"

/*
 * Problem.
 */
struct problem
{
	int masksize; /* mask size.  */
	int imgsize;  /* Image size. */
};

/* Problem sizes. */
static struct problem tiny     = {  7,  2048 };
static struct problem small    = {  7,  4096 };
static struct problem standard = { 11,  8192 };
static struct problem large    = { 11, 16384 };
static struct problem huge     = { 15, 32768 };

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
	printf("Brief: Gaussian Filter Benchmark Kernel\n");
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
static void generate_mask(double *mask)
{
	int half;
	int i, j;
	double sec;
	double first;
	double total;
	
	first = 1.0/(2.0*PI*SD*SD);
	half = p->masksize >> 1;
	total = 0;
	
	#define MASK(i, j) \
		mask[(i)*p->masksize + (j)]

	for (i = -half; i <= half; i++)
	{
		for (j = -half; j <= half; j++)
		{
			sec = -((i*i + j*j)/2.0*SD*SD);
			sec = pow(E, sec);

			MASK(i + half, j + half) = first*sec;
			total += MASK(i + half, j + half);
		}
	}
	
	for (i = 0 ; i < p->masksize; i++)
	{
		for (j = 0; j < p->masksize; j++)
			MASK(i, j) /= total;
	}
}

/*
 * Runs benchmark.
 */
int main(int argc, char **argv)
{
	int i;              /* Loop index.         */
	double *mask;       /* Mask.               */
	uint64_t end;       /* End time.           */
	uint64_t start;     /* Start time.         */
	unsigned char *img; /* Image.              */
	
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
	for (i = 0; i < p->imgsize*p->imgsize; i++)
		img[i] = randnum() & 0xff;
	mask = smalloc(p->masksize*p->masksize*sizeof(double));
	generate_mask(mask);
	end = timer_get();
	if (verbose)
		printf("  time spent: %f\n", timer_diff(start, end)*MICROSEC);
	
	/* Apply filter. */
	if (verbose)
		printf("applying filter...\n");
	
#ifdef _XEON_PHI_
	power_init();
#endif

	start = timer_get();
	gauss_filter(img, p->imgsize, mask, p->masksize);
	end = timer_get();
	
#ifdef _XEON_PHI_
	power = power_end();
#endif

	printf("timing statistics:\n");
	printf("  total time:    %f\n", timer_diff(start, end)*MICROSEC);

#ifdef _XEON_PHI_
	printf("  average power: %f\n", power*0.000001);
#endif
	
	/* House keeping. */
	free(mask);
	free(img);
	
	return (0);
}
