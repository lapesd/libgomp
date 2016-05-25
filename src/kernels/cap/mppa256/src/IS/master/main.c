/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * Integer-Sort Benchmark Kernel.
 */

#include <arch.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <timer.h>
#include <util.h>

/*
 * Bucket sort algorithm.
 */
extern void bucketsort(int *array, int n);

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
	int n; /* Number of elements. */
};

/* Problem sizes. */
static struct problem tiny     = {   8388608 };
static struct problem small    = {  16777216 };
static struct problem standard = {  33554432 };
static struct problem large    = {  67108864 };
static struct problem huge     = { 134217728 };

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
	printf("Usage: kmeans [options]\n");
	printf("Brief: Kmeans Benchmark Kernel\n");
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
 * Runs benchmark.
 */
int main(int argc, char **argv)
{
	int i;          /* Loop index.         */
	int *a;         /* Array to be sorted. */
	uint64_t end;   /* End time.           */
	uint64_t start; /* Start time.         */
	
	readargs(argc, argv);
	
	timer_init();
	srandnum(seed);
	
	/* Benchmark initialization. */
	if (verbose)
		printf("initializing...\n");
	start = timer_get();
	a = smalloc(p->n*sizeof(int));
	for (i = 0; i < p->n; i++)
		a[i] = randnum() & 0xfffff;
	end = timer_get();
	if (verbose)
		printf("  time spent: %f\n", timer_diff(start, end)*MICROSEC);
	
	/* Cluster data. */
	if (verbose)
		printf("sorting...\n");
	start = timer_get();
	bucketsort(a, p->n);
	end = timer_get();

	total = timer_diff(start, end);

	/* Print timing statistics. */
	printf("timing statistics:\n");
	printf("  master:        %f\n", master*MICROSEC);
	for (i = 0; i < nclusters; i++)
		printf("  slave %d:      %f\n", i, slave[i]*MICROSEC);
	printf("  communication: %f\n", communication*MICROSEC);
	printf("  total time:    %f\n", total*MICROSEC);
	printf("data exchange statistics:\n");
	printf("  data sent:            %d\n", data_sent);
	printf("  number sends:         %u\n", nsend);
	printf("  data received:        %d\n", data_received);
	printf("  number receives:      %u\n", nreceive);
	
	/* House keeping. */
	free(a);
	
	return (0);
}
